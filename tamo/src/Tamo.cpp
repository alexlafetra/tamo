#include "Arduino.h"
#include "Sprite.h"
#include "Tamo.h"
#include "utils.h"
#include "Display.h"
#include "spriteFrames.h"
#include <EEPROM.h>

Tamo::Tamo(){
  sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,MEDIUM);
}

void Tamo::init(){
  mode = TAMO_MODE(EEPROM.read(EEPROM_MODE_ADDR));
  identity = SPRITE_IDENTITY(EEPROM.read(EEPROM_IDENTITY_ADDR));
  health = uint16_t(EEPROM.read(EEPROM_HEALTH_ADDR))<<8 | uint16_t(EEPROM.read(EEPROM_HEALTH_ADDR+1));
  // status = EEPROM.read(EEPROM_STATUS_ADDR); //this is causing weird problems with sleep, smoking on reboot
  setStatusBit(NEEDS_TO_SMOKE_BIT,false);
}

bool Tamo::getStatusBit(uint8_t which){
  return 1&(status>>which);
}

void Tamo::setStatusBit(uint8_t which, bool state){
  if(state){
    status |= (1<<which);
  } 
  else{
    status &= ~(1<<which);
  }
}

void Tamo::sleepCheck(){
  setStatusBit(IS_ASLEEP_BIT,itsbeen(TIME_BEFORE_SLEEP));
}

void Tamo::sleep(){

  //set tamo into sleep mode
  setStatusBit(IS_ASLEEP_BIT,true);

  //when the RTC interrupt finishes it goes to this line and tamo can go back to sleep
  while(isAsleep()){
    // digitalWrite(LED_A,CHANGE);
    //put the attiny to sleep
    sleepHardware();
  }

  wakeHardware();
}

void Tamo::setIdentity(SPRITE_IDENTITY i){
  EEPROM.update(EEPROM_IDENTITY_ADDR,i);
  identity = i;
}

const uint16_t * Tamo::getSprite(uint8_t whichSprite){
  switch(identity){
    case TAMO:
      return tamo_spritesheet[whichSprite];
    case PORCINI:
      return porcini_spritesheet[whichSprite];
    case BUG:
      return bug_spritesheet[whichSprite];
    case BOTO:
      return boto_spritesheet[whichSprite];
    case CUSTOM_SPRITE:
      return custom_spritesheet[whichSprite];
    default:
      return tamo_spritesheet[whichSprite];
  }
}

void Tamo::slideshow(){

  //preset speeds/sleep times (0 sleep time is infinite)
  const uint16_t slideshow_speeds[] = {0,100,500,1000,2000,5000,10000,60000};
  const uint32_t slideshow_sleep_times[] = {0,10000,60000,300000};

  uint8_t slideshow_framecount = EEPROM.read(EEPROM_SLIDESHOW_FRAME_COUNT_ADDR);
  uint16_t slideshow_speed = slideshow_speeds[EEPROM.read(EEPROM_SLIDESHOW_SPEED_ADDR)];
  uint32_t slideshow_sleeptime = slideshow_sleep_times[EEPROM.read(EEPROM_SLIDESHOW_SLEEP_TIME_ADDR)];
  uint16_t slideshow_blink_interval = EEPROM.read(EEPROM_SLIDESHOW_BLINK_TIME_ADDR);
  
  if(slideshow_blink_interval == 0)
    oled.disableFadeOutAndBlinking();
  else
    oled.blink(slideshow_blink_interval-1);

  uint32_t timeOfLastFrame = 0;
  uint8_t counter = 0;

  //offset LEDs so they're opposite
  digitalWrite(LED_A,LOW);
  digitalWrite(LED_B,HIGH);

  LONG_PRESS = false;
  lastTime = millis();

  while(true){
    timeOfLastFrame = millis();
    oled.bitmap_from_spritesheet(4,0,64,32,slideshow_frame_0+counter*256);

    counter = (counter+1)%slideshow_framecount;

    //if the speed is slow, change LEDs on each frame
    if(slideshow_speed > 100){
      digitalWrite(LED_A,CHANGE);
      digitalWrite(LED_B,CHANGE);
    }
    //if it's fast, change LEDs when frames wrap around
    else if(counter == 0){
      digitalWrite(LED_A,CHANGE);
      digitalWrite(LED_B,CHANGE);
    }

    do{
      checkInput(false);
      //the itsbeen(500) call is for debouncing
      if((SINGLE_CLICK && itsbeen(500)) || ((slideshow_sleeptime == 0)?false:itsbeen(slideshow_sleeptime))){
        sleep();
        lastTime = millis();
        timeOfLastFrame = millis();
        digitalWrite(LED_A,LOW);
        digitalWrite(LED_B,HIGH);
      }
      if(LONG_PRESS && itsbeen(1000)){
        EEPROM.update(EEPROM_MODE_ADDR,NORMAL_TAMO);
        mode = NORMAL_TAMO;
        return;
      }
    }
    while(((millis() - timeOfLastFrame) < slideshow_speed));
  }
}

void Tamo::qrCode(){
  lastTime = millis();
  digitalWrite(LED_A,HIGH);
  digitalWrite(LED_B,LOW);
  uint32_t ledTime = millis();
  oled.clear();
  while(true){
    oled.bitmap_from_spritesheet(22,0,25,25,qr_code);
    if(itsbeen(1000)){
      checkInput(false);
      if(SINGLE_CLICK)
        break;
    }
    if((millis() - ledTime) > 1000){
      digitalWrite(LED_A,CHANGE);
      digitalWrite(LED_B,CHANGE);
      ledTime = millis();
    }
  }
  oled.clear();
  digitalWrite(LED_A,LOW);
  digitalWrite(LED_B,LOW);
  mood = MOOD_RANDOM;
}

bool Tamo::isFeeling(){
  //if there's still time on the moodTime counter, or if there's not but the next frame isn't yet, keep animating
  //over ride this if tamo dies!
  return ((moodTime>0 || !(sprite.isNextFrameReady() && sprite.hasPlayedAtLeastOnce())) && !getStatusBit(IS_DEAD_BIT));
}
void Tamo::body(){

  if(mode != NORMAL_TAMO)
    return;

  if(mood ==  MOOD_BIRTH || mood == MOOD_DEAD)
    return;

  timeSinceLastCig++;
  //tamo only smokes if it's asleep, not dead, and not being born, since smoking can change tamo's mood
  if(timeSinceLastCig > 600 && isAsleep() &&  !getStatusBit(IS_DEAD_BIT)){
    mood = MOOD_SMOKING;
    setStatusBit(IS_ASLEEP_BIT,false);
    setStatusBit(NEEDS_TO_SMOKE_BIT,true);
    timeSinceLastCig = 0;
    return;
  }
  // if(isAsleep() &&  !getStatusBit(IS_DEAD_BIT) && randomInt(240)){
  //   mood = MOOD_PECKISH;
  //   setStatusBit(IS_ASLEEP_BIT,false);
  //   return;
  // }
  

  uint8_t healthLoss = 0;
  if(hunger < 65535)
    hunger++;
  
  //hunger hits 28800 every 8 hrs
  if(hunger > 28800){
    //you loose health half as quickly for the first 8 hrs of being hungry
    if(hunger < 57,600){
      if(hunger%2)
        healthLoss = HEALTH_LOSS;
    }
    //then u lose 1h/s
    else{
      healthLoss = HEALTH_LOSS;
    }
  }
  //if pooping, you also lose health (this compounds)
  if(mood == MOOD_POOPING){
    healthLoss += HEALTH_LOSS;
  }

  //update health
  health = (health < healthLoss) ? 0:(health - healthLoss);
  //if tamo dies, set the death bit which is detected in vibecheck()
  setStatusBit(IS_DEAD_BIT,health == 0);

  //update EEPROM
  updatesSinceLastEEPROMSave++;
  if(updatesSinceLastEEPROMSave >= 7200){
    updatesSinceLastEEPROMSave = 0;
    EEPROM.update(EEPROM_HEALTH_ADDR,health>>8);
    EEPROM.update(EEPROM_HEALTH_ADDR+1,health&255);
    EEPROM.update(EEPROM_STATUS_ADDR,status);
  }
}

// cycle thru each identity and each emotion
void Tamo::debugCheckMoodSprites(){
  const uint8_t sprites[] = {IDLE_SPRITE,HAPPY_SPRITE,MAD_SPRITE,SAD_SPRITE,EATING_SPRITE};
  identity = CUSTOM_SPRITE;
  for(uint8_t currentSprite = 0; currentSprite<sizeof(sprites); currentSprite++){
    sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(sprites[currentSprite]),2,VFAST);
    while(sprite.loopCount < 3){
      checkInput();
      sprite.update();
    }
  }
}

void Tamo::baby(){
  // sprite = Sprite(SPRITESTARTX+3,8,10,8,baby_idle_sprite,2,MEDIUM);
  // while(true){
  //   checkInput();
  //   if(LONG_PRESS && itsbeen(200)){

  //     break;
  //   }
  //   hardwareSleepCheck();
  // }
}

void Tamo::idle(){
  setMoodSprite(MOOD_NEUTRAL);//set to a neutral sprite, even though you're not feeling neutral
  moodTime = 500;
  basicEmotion();
}

void Tamo::smokingDamage(){
  health = (health < 1000) ? 0 : (health - 1000);
}

//tamo sneaks a cigarette
void Tamo::smokeBreak(){
  sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,cig_animation,5,1000);
  while(true){
    //update and show the current sprite
    //tricky logic so there's not a lil visual glitch
    sprite.update(false,false);
    if(sprite.loopCount>0)
      break;
    sprite.showCurrentFrame();

    lastTime = millis();
    checkInput();
    //interrupting the smoke break
    if(SINGLE_CLICK){
      timeSinceLastCig = 0;
      setStatusBit(NEEDS_TO_SMOKE_BIT,false);
      lastTime = millis();
      setMoodSprite(MOOD_ANGRY);
      while(sprite.loopCount < 4){
        sprite.update();
      }
      return;
    }
  }
  //tamo was successful
  setMoodSprite(MOOD_HAPPY);
  while(sprite.loopCount < 4){
    sprite.update();
  }
  smokingDamage();
  timeSinceLastCig = 0;
  //go back to sleep
  setStatusBit(IS_ASLEEP_BIT,true);
  setStatusBit(NEEDS_TO_SMOKE_BIT,false);
}

//don't call this! you need to set sprites to use it, so only call it from feel()
void Tamo::basicEmotion(){
  while(isFeeling() && !isAsleep()){
    //count down the mood timer
    moodTime--;
    //read inputs
    checkInput();
    //feed tamo!
    if(LONG_PRESS){
      lastTime = millis();
      mood = MOOD_EATING;
      return;
    }
    //talk to tamo!
    if(SINGLE_CLICK && itsbeen(200)){
      //run vibecheck just to get the current thought
      //in case the battery has run down a lot since the last vibecheck, or health has decreased a lot
      vibeCheck(false);
      lastTime = millis();
      talk();
    }
    //update and show the current sprite
    sprite.update();
    sleepCheck();
  }
}

void drawReticle(bool bounce){
  //left
  fbo.drawLine(4-bounce,1-bounce,6-bounce,3-bounce,1);
  fbo.drawLine(4-bounce,14+bounce,6-bounce,12+bounce,1);
  fbo.drawLine(2-bounce,5,4-bounce,7,1);
  fbo.drawLine(2-bounce,10,4-bounce,8,1);
  //right
  fbo.drawLine(27+bounce,1-bounce,25+bounce,3-bounce,1);
  fbo.drawLine(27+bounce,14+bounce,25+bounce,12+bounce,1);
  fbo.drawLine(27+bounce,7,29+bounce,5,1);
  fbo.drawLine(27+bounce,8,29+bounce,10,1);
}

void Tamo::eat(const uint16_t * food){
//recenter sprite! play thru animation of tamo eating next to the food
  sprite = Sprite(32,0,16,16,getSprite(EATING_SPRITE),2,VVFAST);
  Sprite foodSprite = Sprite(16,1,16,16,food,5,VVFAST);
  
  //slide in
  int8_t i = 16;
  while(i>=0){
    sprite.xCoord = i+16;
    sprite.update(true,false);
    foodSprite.xCoord = i;
    //janky frame updating, just to keep it in sync with the main sprite
    if(foodSprite.isNextFrameReady())
      foodSprite.nextFrame();
    foodSprite.currentFrame = 0;
    foodSprite.showCurrentFrame(false,true);
    i-=2;
  }

  lastTime = millis();
  uint8_t offset = 0;
  bool playedThru = false;
  uint8_t currentFrame = 0;
  while(true){
    if(currentFrame != foodSprite.currentFrame){
      lastTime = millis();
      offset = 2; 
      currentFrame = foodSprite.currentFrame;
      digitalWrite(LED_A,CHANGE);
    }
    else{
      offset = 0;
    }
    sprite.update(true,false);
    foodSprite.xCoord = offset;
    foodSprite.update(false,false);
    
    if(foodSprite.currentFrame && !playedThru)
      playedThru = true;
    else if(!foodSprite.currentFrame && playedThru)
      break;
    oled.renderFBO2x(4,0,32,4,fbo.buffer);
  }
  sprite.loopCount = 0;
  foodSprite.currentFrame = 4;
  foodSprite.xCoord = 0;
  while(sprite.loopCount < 2){
    foodSprite.showCurrentFrame(true,false);
    sprite.update(false,true);
  }

  digitalWrite(LED_A,LOW);

  //determine if tamo liked it or not
  const uint16_t * foodPreference;
  //money
  if(thought == THOUGHT_OF_MONEY)
    foodPreference = penny_animation;
  //apple
  else if(thought == THOUGHT_OF_LOVE || thought == THOUGHT_OF_MUSIC || thought == THOUGHT_OF_HAPPYTHOUGHTS)
    foodPreference = apple_animation;
  //cigarette
  else if(thought == THOUGHT_OF_DEATH || thought == THOUGHT_OF_REVENGE || thought == THOUGHT_OF_HEARTBREAK || thought == THOUGHT_OF_SADTHOUGHTS || thought == THOUGHT_OF_SMOKING){
    foodPreference = cig_animation;
  }
  //cheese
  else foodPreference = cheese_animation;

  //cigarette damage
  if(food == cig_animation){
    smokingDamage();
  }

  //if it's not the right kind of food, tamo gets mad
  if(food != foodPreference){
    mood = MOOD_ANGRY;
  }
  //if it is, he's happy
  else{
    //reset hunger counter
    hunger = 0;
    //reset health counter
    health = (health < (65535 - HEALTH_GAIN))?(health+HEALTH_GAIN):65535;
    //tamo needs to poop!
    setStatusBit(NEEDS_TO_POOP_BIT,true);
    mood = MOOD_HAPPY;
  }
  lastTime = millis();
}

void Tamo::feed(){
  const uint16_t * foodAnimations[] = {cheese_animation,apple_animation,penny_animation,cig_animation};
  uint8_t currentFood = randomInt(4);
  sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,foodAnimations[currentFood],5,SLOW);
  //counter goes from 0-64, when it's 0-16 the sprite moves in, when it's 48-64 the sprite moves out
  int16_t counter = 0;
  LONG_PRESS = false;
  lastTime = millis();
  do{
    if(counter < 16){
      sprite.yCoord = counter-16+1;
    }
    else if(counter > 80){
      sprite.yCoord = counter-80+1;
    }
    else{
      sprite.yCoord = 1;
    }
    sprite.showCurrentFrame(true,false);
    drawReticle(counter>48);
    oled.renderFBO2x(4,0,32,4,fbo.buffer);
    checkInput();
    if(ULTRA_LONG_PRESS){
      qrCode();
      // EEPROM.update(EEPROM_MODE_ADDR,SLIDESHOW);
      // mode = SLIDESHOW;
      // return;
    }
    if(SINGLE_CLICK && itsbeen(500)){
      break;
    }
    sleepCheck();
    digitalWrite(LED_B,(counter/24)%2);
    counter = (counter+2);
    if(counter == 96){
      counter = 0;
      currentFood = (currentFood + 1)%4;
      sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,foodAnimations[currentFood],5,SLOW);
    }
  }
  while(!isAsleep());

  lastTime = millis();
  digitalWrite(LED_B,false);

  if(isAsleep()){
    return;
  }

  eat(foodAnimations[currentFood]);
}

void Tamo::feedSelf(){
  const uint16_t * foodAnimations[] = {cheese_animation,apple_animation,penny_animation,cig_animation};
  uint8_t currentFood = randomInt(4);
  eat(foodAnimations[currentFood]);
}

void Tamo::waitAndBlink(uint16_t speed){
  //wait for user input
  bool on = false;
  uint32_t lastBlink = millis();
  while(!SINGLE_CLICK && !isAsleep()){
    if((millis() - lastBlink)>speed){
      lastBlink = millis();
      on = !on;
      digitalWrite(LED_B,on);
    }
    sleepCheck();
    checkInput();
  }
  //turn off LED
  digitalWrite(LED_B,0);
}

void Tamo::waitAndPlayThruSprite(uint16_t speed,bool bounce){
  lastTime = millis();
  uint8_t offset = 0;
  while(true){
    if(itsbeen(speed)){
      lastTime = millis();
      //if it's not a cigarette
      if(bounce)
        offset = 2; 
      if(sprite.currentFrame == sprite.numberOfFrames - 2){
        sprite.xCoord = SPRITESTARTX;
        sprite.nextFrame();
        sprite.showCurrentFrame();
        lastTime = millis();
        while(!itsbeen(speed)){}
        break;
      }
      else{
        sprite.nextFrame();
      }
    }
    else{
      offset = 0;
    }
    sprite.xCoord = SPRITESTARTX+offset;
    sprite.showCurrentFrame();
  }
}

void Tamo::birth(){
  //check to see if the identity has already been set
  identity = EEPROM.read(EEPROM_IDENTITY_ADDR);
  lastTime = millis();
  //if it's not 255, identity was already set! so don't get reborn
  if(identity == NO_IDENTITY){
    sprite = Sprite(SPRITESTARTX+16,SPRITESTARTY,16,16,egg_sprite,4,VFAST);
    while(sprite.xCoord > SPRITESTARTX){
      sprite.xCoord--;
      sprite.showCurrentFrame();
    }
    sprite.xCoord = SPRITESTARTX;
    sprite.showCurrentFrame();
    //wait for button press
    // waitAndBlink(VFAST);
    uint8_t counter;
    while(!SINGLE_CLICK && !isAsleep()){
      if(itsbeen(500))
        checkInput();
      sleepCheck();
      sprite.showCurrentFrame(true,false);
      drawReticle(counter>48);
      oled.renderFBO2x(4,0,32,4,fbo.buffer);
      counter = (counter+2)%96;
    }
    if(isAsleep())
      return;
    //wait for button press
    waitAndPlayThruSprite(VVFAST,true);
    if(isAsleep())
      return;
    
    identity = randomInt(4);
    //write the new identity into eeprom
    EEPROM.update(EEPROM_IDENTITY_ADDR,identity);
    lastTime = millis();//to prevent instant-talking
    status = 0b00000000;//clear status bits
    //reset health
    health = 65535;
    //tamo starts full
    hunger = 0;
    mood = MOOD_NEUTRAL;
    // baby();
  }
  //if tamo is waking from a dead batt, or otherwise being depowered
  else{
    lastTime = millis();//to prevent instant-talking
    status = 0b00000000;//clear status bits (replace this with EEPROM stored value?)
    //reset health
    health = 65535;
    //tamo starts full
    hunger = 0;
    mood = MOOD_NEUTRAL;
  }
}

void Tamo::dead(){
  //erase identity from EEPROM
  EEPROM.update(EEPROM_IDENTITY_ADDR,NO_IDENTITY);

  sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,death_sprite,2,MEDIUM);
  //wake tamo up when it dies!
  setStatusBit(IS_ASLEEP_BIT,false);
  lastTime = millis();
  while(!isAsleep()){
    checkInput();
    sleepCheck();
    sprite.update();
    if(SINGLE_CLICK && itsbeen(200)){
      lastTime = millis();
      mood = MOOD_BIRTH;
      break;
    }
  }
  SINGLE_CLICK = false;
}

void Tamo::walkOn(){
  sprite.yCoord = SPRITESTARTY-16;
  lastTime = millis();
  //slide in
  int8_t i = 0;
  while(i < 16){
    sprite.yCoord = SPRITESTARTY-16+i;
    i++;
    sprite.update();
  }
}
void Tamo::walkOff(){
  sprite.yCoord = SPRITESTARTY;
  lastTime = millis();
  //slide in
  int8_t i = 0;
  while(i < 16){
    sprite.yCoord = SPRITESTARTY+i;
    i++;
    sprite.update();
  }
}

void Tamo::talk(){
  uint8_t frameCount = 2;

  const uint16_t * thoughts[] = {
    talking_love,
    talking_heartbreak,
    talking_death,
    talking_revenge,
    talking_hunger,
    talking_music,
    talking_cash,
    talking_low_battery,
    talking_charging,
    talking_sad,
    talking_neutral,
    talking_happy,
    talking_smoking,
    talking_flowers
  };

  const uint16_t * animationBuffer = thoughts[thought];

  if(thought == THOUGHT_OF_FOOD || thought == THOUGHT_OF_HEARTBREAK || thought == THOUGHT_OF_SADTHOUGHTS || thought == THOUGHT_OF_FLOWERS){
    frameCount = 3;
  }

  TalkingSprite talkingSprite(SPRITESTARTX+10,SPRITESTARTY,12,16,animationBuffer,frameCount,sprite.msPerFrame);
  setMoodSprite(mood); //get the actual mood sprite
  sprite.xCoord = SPRITESTARTX-6;//move sprite to the left
  lastTime = millis();
  SINGLE_CLICK = false;

  //slide in
  int8_t i = 6;
  while(i >= 0){
    sprite.xCoord = SPRITESTARTX-6+i;
    i-=3;
    sprite.update();
  }

  //talk for 2 cycle counts, and 
  while((talkingSprite.loopCount<2 && !SINGLE_CLICK)){
    talkingSprite.update();
    sprite.update(false,true);
    if(itsbeen(200)){
      checkInput();
    }
  }

  //slide out
  i = 0;
  while(i <= 6){
    sprite.xCoord = SPRITESTARTX-6+i;
    i+=3;
    sprite.update();
  }
  sprite.xCoord = SPRITESTARTX;//move sprite back
}

void Tamo::setMoodSprite(uint8_t m){
  switch(m){
    case MOOD_NEUTRAL:
      sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,FAST);
      return;
    case MOOD_SAD:
      sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(SAD_SPRITE),2,FAST);
      return;
    case MOOD_ANGRY:
      sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(MAD_SPRITE),2,VFAST);
      return;
    case MOOD_HAPPY:
      sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(HAPPY_SPRITE),2,VFAST);
      return;
    default:
      sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,FAST);
      return;
  }
}

void Tamo::poop(){
  sprite = Sprite(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,2,VFAST);
  lastTime = millis();
  while(!isAsleep()){
    checkInput();
    if(SINGLE_CLICK && itsbeen(800)){
      lastTime = millis();
      fbo.clear();
      fbo.bitmap_from_spritesheet(SPRITESTARTX,SPRITESTARTY,16,16,egg_sprite[3]);
      oled.renderFBO2x(4,0,32,4,fbo.buffer);
      while(!itsbeen(800)){}
      setStatusBit(NEEDS_TO_POOP_BIT,false);
      break;
    }
    fbo.clear();
    // drawReticle(sprite.currentFrame%2);
    sprite.update(false,true);
    sleepCheck();
  }
}

void Tamo::live(){
  //experience current emotion (talking happens inside basicEmotion())
  feel();
  //grab new emotion depending on health & batt state
  vibeCheck();
}

/*
Calibration notes:
@5V it's ~212
@3.9V it's ~260
@3.73 it's ~295
@3.3V it's ~358
@1.57 it's ~703

voltage = -150.70169*(measurement)+893.05592

attiny85 can operate from 1.8v - 5.5v

*/

//VCC reading gets LOWER with higher voltage bc you're comparing it to the internal 1.1v
//249-240 when plugged into usb-c
//240 - 242 when plugged into 5v
//shuts off at ~454 (guessing it's around 2.7v)
void Tamo::batteryCheck(){
  uint16_t vcc = readVcc();
  setStatusBit(LOW_BATTERY_BIT,vcc > 400);
  setStatusBit(IS_CHARGING_BIT,vcc < 260);//assume tamo is plugged in, if vcc is this high
}

//function selecting which emotion loop to run based on tamo's mood
void Tamo::feel(){

  // if the sleep bit is set, fall asleep
  if(isAsleep()){
    sleep();
  }

  //if the smoke bit is set, smokebreak!
  if(getStatusBit(NEEDS_TO_SMOKE_BIT)){
    smokeBreak();
    return;
  }
  if(mood == MOOD_RANDOM){
    vibeCheck();
  }
  if(mood == MOOD_POOPING){
    poop();
    return;
  }
  if(mood == MOOD_DEAD){
    dead();
    return;
  }
  if(mood == MOOD_BIRTH){
    birth();
    return;
  }
  if(mood == MOOD_EATING){
    feed();
    return;
  }
  if(mood == MOOD_TALKING){
    talk();
    return;
  }
  if(mood == MOOD_PECKISH){
    feedSelf();
    return;
  }
  moodTime = 500;
  setMoodSprite(mood);
  basicEmotion();
  return;
}

void Tamo::vibeCheck(){
  vibeCheck(true);
}
// function selecting which mood tamo is in based on battery & health & needing to poop
void Tamo::vibeCheck(bool updateThought){

  //check batt voltage
  batteryCheck();

  //if dead or being born or sleeping or eating, stay dead or being born or sleeping or eating
  //(these are all reset by their routines, and wait for user input)
  if(mood == MOOD_DEAD || mood == MOOD_BIRTH || isAsleep() || mood == MOOD_EATING || mood == MOOD_QR_CODE)
    return;

  if(getStatusBit(IS_DEAD_BIT)){
    if(health == 0){
      mood = MOOD_DEAD;
      return;
    }
    //if, for some reason, health increased from 0 (like a last-minute feeding) then tamo revives
    else{
      setStatusBit(IS_DEAD_BIT,false);
    }
  }

  uint8_t currentState = OKAY_STATE;

  if(health > GOOD_HEALTH_THRESHOLD){
    currentState = GOOD_STATE;
  }
  else if(health > BAD_HEALTH_THRESHOLD){
    currentState = OKAY_STATE;
  }
  else{
    currentState = BAD_STATE;
  }

  // //1/40 chance u get sad for no reason
  if(!randomInt(40)){
    currentState = BAD_STATE;
  }

  //always talk abt low batt if the batt is low
  if(lowBattery()){
    currentState = BAD_STATE;
  }

  if(updateThought){
    //get your thought/mood based on your state
    if(currentState == GOOD_STATE){
      thought = happyThoughts[randomInt(sizeof(happyThoughts))];
      mood = MOOD_HAPPY;//always happy
    }
    else if(currentState == OKAY_STATE){
      thought = neutralThoughts[randomInt(sizeof(neutralThoughts))];
      mood = MOOD_NEUTRAL;
    }
    else{
      thought = sadThoughts[randomInt(sizeof(sadThoughts))];
      mood = (randomInt(2))?MOOD_SAD:MOOD_ANGRY; // sad, angry
    }
  }

  //1/10 chance tamo randomly talks
  if(!randomInt(10))
    mood = MOOD_TALKING;

  //if you need to poop, there's a 1/3 chance you'll poop
  if(needsToPoop() && randomInt(240) == 0){
    mood = MOOD_POOPING;
  }
  
  //special states that tamo thinks about
  if(!randomInt(2)){
    if(lowBattery())
      thought = THOUGHT_OF_LOWBATTERY;
    else if((hunger == 255))
      thought = THOUGHT_OF_FOOD;
    else if(isCharging())
      thought = THOUGHT_OF_CHARGING;
  }
  
}

//Small shooter game
void Tamo::game(){
  int8_t location = 0;
  int8_t speed = 1;
  oled.clear();
  while(true){

    // //erase last target
    // oled.setCursor(location,0);
		// ssd1306_send_data_start();
    // ssd1306_send_data_byte(0);
    // ssd1306_send_stop();

    // //update location
    // location+=speed;
    // if(location>64 || location < 0)
    //   speed = -speed;

    // //draw target
    // oled.setCursor(location,0);
		// ssd1306_send_data_start();
    // ssd1306_send_data_byte(255);
    // ssd1306_send_stop();

    // //draw gun
    // oled.setCursor(32,1);
    // ssd1306_send_data_start();
    // ssd1306_send_data_byte(255);
    // ssd1306_send_stop();

    // //read inputs
    // checkInput();

    // //if it's a hit, break!
    // if(SINGLE_CLICK){
    //   if((abs(location - 32) < speed)){
    //     mood = MOOD_HAPPY;
    //     break;
    //   }
    //   else{
    //     mood = MOOD_ANGRY;
    //     break;
    //   }
    // }
  }
  oled.clear();
}