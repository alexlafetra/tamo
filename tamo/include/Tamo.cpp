#include "Arduino.h"
#include "Animation.h"
// #include "debug/numbers.cpp"

#define SPRITESTARTX 20
#define SPRITESTARTY 0

//Speeds for Sprites
#define VVVFAST 50
#define VVFAST 100
#define VFAST 200
#define FAST 400
#define MEDIUM 600
#define SLOW 1200

// Thoughts
#define THOUGHT_OF_LOVE 0 //heart
#define THOUGHT_OF_HEARTBREAK 1 //heart breaking
#define THOUGHT_OF_DEATH 2 //skull
#define THOUGHT_OF_REVENGE 3 //gun
#define THOUGHT_OF_FOOD 4 //pacman? don't like this one
#define THOUGHT_OF_MUSIC 5 //music note
#define THOUGHT_OF_MONEY 6 //$$$
#define THOUGHT_OF_LOWBATTERY 7 //empty battery
#define THOUGHT_OF_CHARGING 8 // charging battery
#define THOUGHT_OF_SADTHOUGHTS 9 //crying face
#define THOUGHT_OF_NEUTRALTHOUGHTS 10 //neutral face
#define THOUGHT_OF_HAPPYTHOUGHTS 11//happy face

// Moods
#define MOOD_NEUTRAL 0
#define MOOD_SAD 1
#define MOOD_ANGRY 2
#define MOOD_HAPPY 3
#define MOOD_EATING 4
#define MOOD_DEAD 5
#define MOOD_BIRTH 6
#define MOOD_RANDOM 7
#define MOOD_SMOKING 8
#define MOOD_POOPING 9

// Sprite ID's
#define IDLE_SPRITE MOOD_NEUTRAL
#define SAD_SPRITE MOOD_SAD
#define MAD_SPRITE MOOD_ANGRY
#define HAPPY_SPRITE MOOD_HAPPY
#define EATING_SPRITE MOOD_EATING

const uint8_t happyThoughts[4] = {THOUGHT_OF_HAPPYTHOUGHTS,THOUGHT_OF_LOVE,THOUGHT_OF_MUSIC,THOUGHT_OF_MONEY};
const uint8_t neutralThoughts[5] = {THOUGHT_OF_NEUTRALTHOUGHTS,THOUGHT_OF_MUSIC,THOUGHT_OF_MONEY,THOUGHT_OF_REVENGE,THOUGHT_OF_DEATH};
const uint8_t sadThoughts[5] = {THOUGHT_OF_SADTHOUGHTS,THOUGHT_OF_HEARTBREAK,THOUGHT_OF_DEATH,THOUGHT_OF_REVENGE,THOUGHT_OF_MONEY};

//health starts at 65535 and goes down every 9s by HEALTH_LOSS or HEALTH_LOSS * 4 if tamo has pooped
//meaning, with no intervention tamo should die in 65535/HEALTH_LOSS * 9 / 3600
// ==> 54.6125 hr @ 3 health loss
// set HL to 2000 to make it last 5min (debugging)

#define HEALTH_LOSS 3
//feed tamo every 12 hrs
#define FOOD_HEALTH_RECOVERY (HEALTH_LOSS/9 * 3600 * 12)

#define GOOD_HEALTH_THRESHOLD 55535 //drops to this in 25hrs
#define BAD_HEALTH_THRESHOLD 32768 //drops to this in 3 days

#define GOOD_STATE 0
#define OKAY_STATE 1
#define BAD_STATE 2

//Tamo bit flags
#define NEEDS_TO_POOP_BIT 0
#define IS_ASLEEP_BIT 1
#define LOW_BATTERY_BIT 2
#define IS_CHARGING_BIT 3
#define IS_DEAD_BIT 4
#define SMOKING_BIT 5
#define NEEDS_TO_SMOKE_BIT 6

#define IDENTITY_ADDRESS 0
#define FOOD_PREFERENCE_ADDRESS 1

// Creature identities
#define NO_IDENTITY 255u
#define TAMO 0
#define PORCINI 1
#define BUG 2
#define BOTO 3

#define NO_FOOD_PREFERENCE 255
#define PREFERS_CHEESE 0
#define PREFERS_COOKIE 1
#define PREFERS_APPLE 2
#define PREFERS_CIG 3


/*
when tamo sleeps, it needs to jump out of whatever emotion it's in before actually putting itself to sleep. Then 
mood/state can be changed during sleep, and when he wakes up he'll live() or vibecheck() again to get back to it
*/
class Tamo{
  public:
    Tamo();
    Animation sprite;
    uint8_t mood = MOOD_BIRTH;
    // uint8_t mood = MOOD_NEUTRAL;
    uint8_t thought = THOUGHT_OF_LOVE;
    int16_t moodTime = 0;
    uint8_t timeSinceLastCig = 0;
    uint8_t timeSinceLastTalk = 0;
    //status register (volatile so that sleep can be turned off from interrupts)
    volatile uint8_t status = 0b00000000;
    uint8_t identity = 0;//which sprites to chose from

    /*
      health decreases every 8 seconds. Food resets it
    */
    volatile uint16_t health = 65535;//decreases by 6, so it lasts ~24 hrs. When tamo has pooped, decreases by 12
    volatile uint16_t hunger = 0;//increases up to 255, at which point tamo can eat. Every 34min
    uint16_t healthAddress = 1;

    bool isFeeling();
    void sleepCheck();
    void body();
    //runs an update cycle
    void live();
    //gets new emotion/current state
    void vibeCheck();
    //runs the current emotion
    void feel();
    void debugCheckMoodSprites();

    void game();

    void setMoodSprite(uint8_t mood);

    void basicEmotion();
    void waitAndPlayThruSprite(uint8_t,bool);
    void waitAndBlink(uint8_t);
    void idle();
    void smokeBreak();
    void talk(uint8_t t);
    void poop();
    void eat();
    void dead();
    void birth();
    void baby();
    bool getStatusBit(uint8_t which);
    void setStatusBit(uint8_t which, bool state);
    bool needsToPoop(){return getStatusBit(NEEDS_TO_POOP_BIT);}
    bool isAsleep(){return getStatusBit(IS_ASLEEP_BIT);}
    bool lowBattery(){return getStatusBit(LOW_BATTERY_BIT);}
    bool isCharging(){return getStatusBit(IS_CHARGING_BIT);}
    const uint16_t *  getSprite(uint8_t);
    void batteryCheck();
};

Tamo::Tamo(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,MEDIUM);
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
    default:
      return tamo_spritesheet[whichSprite];
  }
}

bool Tamo::isFeeling(){
  //if there's still time on the moodTime counter, or if there's not but the next frame isn't yet, keep animating
  //over ride this if tamo dies!
  return ((moodTime>0 || !(sprite.isNextFrameReady() && sprite.hasPlayedAtLeastOnce())) && !getStatusBit(IS_DEAD_BIT));
}
void Tamo::body(){

  // talk to other tamo!
  // timeSinceLastTalk+=100;
  // if(timeSinceLastTalk > 10*(millis()%4)){
  //   PORTB |= (1<<BOTTOM_LED_PIN);
  //   timeSinceLastTalk = 0;
  //   //1/10 chance you long press
  //   if((millis()%10)){
  //     delay(200);
  //     PORTB &= ~(1<<BOTTOM_LED_PIN);
  //   }
  // }
  // else{
  //   PORTB &= ~(1<<BOTTOM_LED_PIN);
  // }

  if(mood ==  MOOD_BIRTH || mood == MOOD_DEAD)
    return;

  timeSinceLastCig+=100;
  //tamo only smokes if it's asleep, not dead, and not being born, since smoking can change tamo's mood
  if(timeSinceLastCig > 200 && isAsleep() &&  !getStatusBit(IS_DEAD_BIT)){
    mood = MOOD_SMOKING;
    setStatusBit(IS_ASLEEP_BIT,false);
    setStatusBit(NEEDS_TO_SMOKE_BIT,true);
    timeSinceLastCig = 0;
    return;
  }

  if(hunger < 255)
    hunger++;
  
  uint8_t healthLoss = HEALTH_LOSS;
  //if pooping or very hungry, health loss is doubled (u gotta clean him!)
  if(mood == MOOD_POOPING || hunger == 255){
    healthLoss = HEALTH_LOSS*4;
  }
  int32_t tempHealth = health;
  tempHealth -= healthLoss;
  if(tempHealth < 0)
    tempHealth = 0;
  health = tempHealth;
  //if tamo dies, set the death bit
  setStatusBit(IS_DEAD_BIT,health == 0);
  if(health == 0)
    mood = MOOD_DEAD;
}

// cycle thru each identity and each emotion
void Tamo::debugCheckMoodSprites(){
  const uint8_t sprites[] = {IDLE_SPRITE,EATING_SPRITE,SAD_SPRITE,MAD_SPRITE,HAPPY_SPRITE};
  const uint8_t identities[] = {TAMO,PORCINI,BOTO};

  for(uint8_t currentIdentity = 0; currentIdentity<3; currentIdentity++){
    identity = identities[currentIdentity];
    for(uint8_t currentSprite = 0; currentSprite<5; currentSprite++){
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(sprites[currentSprite]),2,VFAST);
      while(true){
        readButtons();
        if(SINGLE_CLICK && itsbeen(200)){
          break;
        }
        sprite.update();
      }
    }
  }
}

void Tamo::baby(){
  //matters!
  uint8_t whatYouFedTheBaby[3] = {0,0,0};
  sprite = Animation(SPRITESTARTX+3,8,10,8,baby_idle_sprite,2,MEDIUM);
  while(true){
    readButtons();
    if(LONG_PRESS && itsbeen(200)){

      break;
    }
    hardwareSleepCheck();
  }
}

void Tamo::idle(){
  setMoodSprite(MOOD_NEUTRAL);//set to a neutral sprite, even though you're not feeling neutral
  moodTime = 500;
  basicEmotion();
}

//tamo sneaks a cigarette
void Tamo::smokeBreak(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,cig_animation,5,1000);
  while(sprite.loopCount == 0){
    //update and show the current sprite
    sprite.update();
    lastTime = millis();
    readButtons();
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
    readButtons();
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
      uint8_t oldMood = mood;
      vibeCheck();
      lastTime = millis();
      talk(thought);
      mood = oldMood;
    }
    //update and show the current sprite
    sprite.update();
    sleepCheck();
  }
}

void Tamo::eat(){
  uint8_t currentFood = millis()%2;
  const uint16_t * foodAnimations[] = {cheese_animation,apple_animation};
  sprite = Animation(SPRITESTARTX,0,16,16,foodAnimations[currentFood],5,FAST);
  sprite.showCurrentFrame();
  while(!SINGLE_CLICK && !isAsleep()){
    readButtons();
    sleepCheck();
    if(itsbeen(250)){
      // on
      PORTB |= (1 << TOP_LED_PIN);
    }
    else{
      // off
      PORTB &= ~(1<<TOP_LED_PIN);
    }
    if(itsbeen(500)){
      currentFood = (currentFood + 1)%2;
      sprite = Animation(SPRITESTARTX,0,16,16,foodAnimations[currentFood],5,FAST);
      sprite.showCurrentFrame();
      lastTime = millis();
    }
  }
  //autoplay eating animation
  waitAndPlayThruSprite(150,currentFood != 2);

  uint8_t foodPreference = EEPROM.read(FOOD_PREFERENCE_ADDRESS);
  if(foodPreference == NO_FOOD_PREFERENCE){
    foodPreference = currentFood;
    EEPROM.write(FOOD_PREFERENCE_ADDRESS,foodPreference);
  }

  //if it's not the right kind of food, tamo gets mad
  if(currentFood != foodPreference){
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(MAD_SPRITE),2,VFAST);
  }
  //if it is, he eats it
  else{
    //tamo eats
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(EATING_SPRITE),2,VFAST);
    while(sprite.loopCount < 4){
      sprite.update();
    }
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(HAPPY_SPRITE),2,VFAST);
    //recover health
    health = (health<(65535-FOOD_HEALTH_RECOVERY))?(health+FOOD_HEALTH_RECOVERY):65535;
    //reset hunger counter
    hunger = 0;
    //tamo needs to poop!
    setStatusBit(NEEDS_TO_POOP_BIT,true);
  }
  while(sprite.loopCount < 4){
    sprite.update();
  }
  lastTime = millis();
  mood = MOOD_NEUTRAL;
}

/*
void Tamo::eat(){
  uint8_t currentFood = millis()%3;
  const uint16_t * foodAnimations[] = {cheese_animation,apple_animation,cig_animation};
  sprite = Animation(SPRITESTARTX,0,16,16,foodAnimations[currentFood],5,FAST);

  lastTime = millis();
  sprite.showCurrentFrame();
  while(!SINGLE_CLICK){
    hardwareSleepCheck();
    readButtons();
    if(itsbeen(250)){
      // on
      PORTB |= (1 << TOP_LED_PIN);
    }
    else{
      // off
      PORTB &= ~(1<<TOP_LED_PIN);
    }
    if(itsbeen(500)){
      currentFood = (currentFood + 1)%4;
      sprite = Animation(SPRITESTARTX,0,16,16,foodAnimations[currentFood],5,FAST);
      sprite.showCurrentFrame();
      lastTime = millis();
    }
  }
  // off
  PORTB &= ~(1<<TOP_LED_PIN);
  uint8_t bite = 0;
  while(true){
    readButtons();

    if(LONG_PRESS){
      oled.disableZoomIn();
      oled.blink(32);
      oled.bitmap_from_spritesheet(4,0,68,4,mexican_flag_sprite);
      while(!SINGLE_CLICK){ 
        readButtons();
      }
      oled.disableFadeOutAndBlinking();
      oled.clear();
      oled.enableZoomIn();
      return;
    }

    //when user presses a button, tamo takes a bite
    //and the food sprite jumps
    if(SINGLE_CLICK){
      lastTime = millis();
      //if it's not a cigarette
      if(currentFood != 3){
        if(!bite){
          oled.clearEdges(SPRITESTARTX+5,0);
        }
        bite = 5;
      }
      if(sprite.currentFrame == 3){
        sprite.nextFrame();
        sprite.showCurrentFrame();
        lastTime = millis();
        while(millis() - lastTime < 300){}
        break;
      }
      else{
        sprite.nextFrame();
      }
    }
    else{
      if(bite){
        oled.clearEdges(SPRITESTARTX,12);
      }
      bite = 0;
    }
    sprite.xCoord = SPRITESTARTX+bite;
    sprite.showCurrentFrame();
    hardwareSleepCheck();
  }

  uint8_t foodPreference = EEPROM.read(FOOD_PREFERENCE_ADDRESS);
  if(foodPreference == NO_FOOD_PREFERENCE){
    foodPreference = currentFood;
    EEPROM.write(FOOD_PREFERENCE_ADDRESS,foodPreference);
  }

  //if it's not the right kind of food, tamo gets mad
  if(currentFood != foodPreference){
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(MAD_SPRITE),2,VFAST);
  }
  //if it is, he eats it
  else{
    //tamo eats
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(EATING_SPRITE),2,VFAST);
    while(sprite.loopCount < 4){
      sprite.update();
    }
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(HAPPY_SPRITE),2,VFAST);
    //recover health
    health = (health<(65535-FOOD_HEALTH_RECOVERY))?(health+FOOD_HEALTH_RECOVERY):65535;
    //reset hunger counter
    hunger = 0;
    //tamo needs to poop!
    setStatusBit(NEEDS_TO_POOP_BIT,true);
  }
  while(sprite.loopCount < 4){
    sprite.update();
  }
  lastTime = millis();
}

*/
void Tamo::waitAndBlink(uint8_t speed){
  //wait for user input
  bool on = false;
  uint32_t lastBlink = millis();
  while(!SINGLE_CLICK && !isAsleep()){
    if((millis() - lastBlink)>speed){
      lastBlink = millis();
      on = !on;
      // on
      if(on)
        PORTB |= (1 << TOP_LED_PIN);
      //off
      else
        PORTB &= ~(1<<TOP_LED_PIN);
    }
    sleepCheck();
    readButtons();
  }
  //turn off LED
  PORTB &= ~(1<<TOP_LED_PIN);
}

void Tamo::waitAndPlayThruSprite(uint8_t speed,bool bounce){
  lastTime = millis();
  uint8_t offset = 0;
  while(true){
    if(itsbeen(speed)){
      lastTime = millis();
      //if it's not a cigarette
      if(!offset){
        oled.clearEdges(SPRITESTARTX+5,0);
      }
      if(bounce)
        offset = 5; 
      if(sprite.currentFrame == sprite.numberOfFrames - 2){
        sprite.xCoord = SPRITESTARTX+offset;
        sprite.nextFrame();
        sprite.showCurrentFrame();
        lastTime = millis();
        while(millis() - lastTime < 300){}
        oled.clearEdges(SPRITESTARTX,12);
        break;
      }
      else{
        sprite.nextFrame();
      }
    }
    else{
      if(offset){
        oled.clearEdges(SPRITESTARTX,12);
      }
      offset = 0;
    }
    sprite.xCoord = SPRITESTARTX+offset;
    sprite.showCurrentFrame();
  }
}
void Tamo::birth(){
  //check to see if the identity has already been set
  identity = EEPROM.read(IDENTITY_ADDRESS);

  //if it's not 255, identity was already set! so don't get reborn
  if(identity == NO_IDENTITY){
    sprite = Animation(SPRITESTARTX+32,SPRITESTARTY,16,16,egg_sprite,4,SLOW);
    oled.fill(0);
    while(sprite.xCoord > SPRITESTARTX){
      sprite.xCoord-=2;
      sprite.showCurrentFrame();
    }
    sprite.xCoord = SPRITESTARTX;
    sprite.showCurrentFrame();
    waitAndBlink(200);
    if(isAsleep())
      return;
    waitAndPlayThruSprite(200,true);
    if(isAsleep())
      return;
    
    identity = millis()%4;
    //write the new identity into eeprom
    EEPROM.write(IDENTITY_ADDRESS,identity);
  }
  lastTime = millis();//to prevent instant-talking
  //reset health
  health = 65535;
  //tamo starts full
  hunger = 0;
  mood = MOOD_NEUTRAL;
  // mood = MOOD_DEAD;
  // baby();
}

void Tamo::dead(){
  //erase identity from EEPROM
  if(EEPROM.read(IDENTITY_ADDRESS) != NO_IDENTITY)
    EEPROM.write(IDENTITY_ADDRESS,NO_IDENTITY);
  if(EEPROM.read(FOOD_PREFERENCE_ADDRESS) != NO_FOOD_PREFERENCE)
    EEPROM.write(FOOD_PREFERENCE_ADDRESS,NO_FOOD_PREFERENCE);

  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,death_sprite,2,MEDIUM);
  //wake tamo up when it dies!
  setStatusBit(IS_ASLEEP_BIT,false);
  lastTime = millis();
  while(!isAsleep()){
    readButtons();
    sleepCheck();
    if(SINGLE_CLICK && itsbeen(200)){
      lastTime = millis();
      break;
    }
    sprite.update();
  }
  SINGLE_CLICK = false;
  if(!isAsleep())
    mood = MOOD_BIRTH;
}

void Tamo::talk(uint8_t t){
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
    talking_happy
  };

  const uint16_t * animationBuffer = thoughts[t];

  if(t == THOUGHT_OF_FOOD || t == THOUGHT_OF_HEARTBREAK || t == THOUGHT_OF_SADTHOUGHTS){
    frameCount = 3;
  }

  TalkingAnimation talkingSprite(SPRITESTARTX+20,SPRITESTARTY,12,16,animationBuffer,frameCount,400);
  setMoodSprite(mood); //get the actual mood sprite
  sprite.xCoord = SPRITESTARTX-12;//move sprite to the left

  //talk for 2 cycle counts, and 
  while((talkingSprite.loopCount<2)){
    sprite.update();
    talkingSprite.update();
  }
  oled.clearEdges();//clear screen edges
  sprite.xCoord = SPRITESTARTX;//move sprite back
}

void Tamo::setMoodSprite(uint8_t m){
  switch(m){
    case MOOD_NEUTRAL:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,MEDIUM);
      return;
    case MOOD_SAD:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(SAD_SPRITE),2,MEDIUM);
      return;
    case MOOD_ANGRY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(MAD_SPRITE),2,VFAST);
      return;
    case MOOD_HAPPY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(HAPPY_SPRITE),2,FAST);
      return;
    default:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,MEDIUM);
      return;
  }
}

void Tamo::poop(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,2,MEDIUM);
  lastTime = millis();
  while(!isAsleep()){
    readButtons();
    if(SINGLE_CLICK && itsbeen(400)){
      lastTime = millis();
      oled.bitmap_from_spritesheet2x(SPRITESTARTX,SPRITESTARTY,SPRITESTARTX+16,SPRITESTARTY+1,egg_sprite[3]);
      while(millis() - lastTime < 400){}
      setStatusBit(NEEDS_TO_POOP_BIT,false);
      break;
    }
    sprite.update();
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
  // setStatusBit(LOW_BATTERY_BIT,vcc > 400);
  setStatusBit(IS_CHARGING_BIT,vcc < 260);//assume tamo is plugged in, if vcc is this high

  // printNumberString(String(vcc));
}

//function selecting which emotion loop to run based on tamo's mood
void Tamo::feel(){
  // if the sleep bit is set, fall asleep
  if(isAsleep()){
    hardwareSleep();
  }

  //if the smoke bit is set, smokebreak!
  if(getStatusBit(NEEDS_TO_SMOKE_BIT)){
    smokeBreak();
    return;
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
    eat();
    return;
  }
  moodTime = 500;
  setMoodSprite(mood);
  basicEmotion();
  return;
}

// function selecting which mood tamo is in based on battery & health & needing to poop
void Tamo::vibeCheck(){
  //check batt voltage
  batteryCheck();

  //if dead or being born or sleeping or eating, stay dead or being born or sleeping or eating
  //(these are all reset by their routines, and wait for user input)
  if(mood == MOOD_DEAD || mood == MOOD_BIRTH || isAsleep() || getStatusBit(IS_DEAD_BIT) || mood == MOOD_EATING)
    return;

  if(health == 0 || getStatusBit(IS_DEAD_BIT)){
    mood = MOOD_DEAD;
    return;
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

  //1/10 & 1/9 chance u get sad for no reason
  if(!(millis()%10) || !(millis()%9)){
    currentState = BAD_STATE;
  }

  //always talk abt low batt if the batt is low
  if(lowBattery()){
    thought = THOUGHT_OF_LOWBATTERY;
    mood = MOOD_SAD;
    return;
  }

  //get your thought/mood based on your state
  if(currentState == GOOD_STATE){
    thought = happyThoughts[millis()%4];
    mood = MOOD_HAPPY;//always happy
  }
  else if(currentState == OKAY_STATE){
    thought = neutralThoughts[millis()%5];
    mood = MOOD_NEUTRAL;
  }
  else{
    thought = sadThoughts[millis()%5];
    mood = (millis()%2)?MOOD_SAD:MOOD_ANGRY; // sad, angry
  }
  
  //if hungry, tamo thinks about food
  if((hunger == 255))
    thought = THOUGHT_OF_FOOD;
  else if(isCharging())
    thought = THOUGHT_OF_CHARGING;
  
  //if you need to poop, there's a 1/3 chance you'll poop
  if(needsToPoop()){
    if(!(millis()%3))
      mood = MOOD_POOPING;
  }
}

//Small shooter game
void Tamo::game(){
  int8_t location = 0;
  int8_t speed = 1;
  oled.clear();
  while(true){

    //erase last target
    oled.setCursor(location,0);
		ssd1306_send_data_start();
    ssd1306_send_data_byte(0);
    ssd1306_send_stop();

    //update location
    location+=speed;
    if(location>64 || location < 0)
      speed = -speed;

    //draw target
    oled.setCursor(location,0);
		ssd1306_send_data_start();
    ssd1306_send_data_byte(255);
    ssd1306_send_stop();

    //draw gun
    oled.setCursor(32,1);
    ssd1306_send_data_start();
    ssd1306_send_data_byte(255);
    ssd1306_send_stop();

    //read inputs
    readButtons();

    //if it's a hit, break!
    if(SINGLE_CLICK){
      if((abs(location - 32) < speed)){
        mood = MOOD_HAPPY;
        break;
      }
      else{
        mood = MOOD_ANGRY;
        break;
      }
    }
  }
  oled.clear();
}