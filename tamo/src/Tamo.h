// #define SPRITESTARTX 16
#define SPRITESTARTX 20
#define SPRITESTARTY 0

//Speeds for Sprites
#define VVVFAST 50
#define VVFAST 100
#define VFAST 200
#define FAST 400
#define MEDIUM 600
#define SLOW 1200

enum Thought:uint8_t{
  LOVE,
  HEARTBREAK,
  DEATH,
  REVENGE,
  FOOD,
  FACE,
  MUSIC,
  MONEY,
  DOG,
  LOWBATTERY
};

enum Idea:uint8_t{
  GUN,
  BIRD,
  HEART
};

enum Mood:uint8_t{
  RANDOM,
  RANDOM_NO_TALK,
  NEUTRAL,
  HAPPY,
  ANGRY,
  SAD,
  SOBBING,
  TALKING,
  EATING,
  POOPING,
  DEAD,
  BIRTH,
  FREEPALESTINE
};

enum SPRITE_ID:uint8_t{
  IDLE_SPRITE,
  EATING_SPRITE,
  SAD_SPRITE,
  CRYING_SPRITE,
  MAD_SPRITE,
  HAPPY_SPRITE
};

const Thought happyThoughts[5] = {LOVE,FACE,MUSIC,MONEY,DOG};
const Thought neutralThoughts[6] = {FACE,MUSIC,MONEY,DOG,REVENGE,DEATH};
const Thought sadThoughts[5] = {HEARTBREAK,DEATH,REVENGE,MONEY,DOG};

const Mood goodMoods[3] = {HAPPY,TALKING};
const Mood neutralMoods[5] = {NEUTRAL,HAPPY,SAD,TALKING};
const Mood badMoods[5] = {SAD,SOBBING,ANGRY,TALKING};

#define GOOD_STATE 0
#define OKAY_STATE 1
#define BAD_STATE 2

//health starts at 65535 and goes down every 9s by 1 (drops to 0 in 6.8 days)
#define GOOD_HEALTH_THRESHOLD 55535 //drops to this in 25hrs
#define BAD_HEALTH_THRESHOLD 32768 //drops to this in 3 days

#define FOOD_HEALTH_RECOVERY 1800//equivalent to 3hr worth of health

#define NEEDS_TO_POOP 0
#define LOW_BATTERY 1
#define HEALTH_ADDRESS_ADDRESS 1
#define IDENTITY_ADDRESS 0

class Tamo{
  public:
    Tamo();
    Animation sprite;
    // Mood mood = BIRTH;
    Mood mood = DEAD;
    Thought thought = LOVE;
    int16_t moodTime = 0;
    uint8_t status = 0b00000000;
    uint8_t identity = BUG;//which sprites to chose from

    /*
      health decreases every few seconds. Food increases it
    */
    uint16_t health = 65535;
    uint16_t healthAddress = 1;
    uint8_t updatesSinceLastWrite = 0;

    // void update();
    void vibeCheck();
    bool isFeeling();
    void body();
    void feel();

    void basicEmotion(Mood moodAfter);
    void talk(Thought t);
    void poop();
    void eat();
    void dead();
    void birth();
    void freePalestine();

    void baby();

    bool getStatusBit(uint8_t which);
    void setStatusBit(uint8_t which, bool state);
    bool needsToPoop(){return getStatusBit(NEEDS_TO_POOP);}
    bool lowBattery(){return getStatusBit(LOW_BATTERY);}
    const unsigned char* const * getSprite(SPRITE_ID);
    void batteryCheck();

};

Tamo::Tamo(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,MEDIUM);
  //get identity
  // EEPROM.get(IDENTITY_ADDRESS,identity);
  //check and see if tamo is alive
  // EEPROM.get(HEALTH_ADDRESS_ADDRESS,healthAddress);
  // EEPROM.get(healthAddress,health);
  // if(!health)
  //   mood = DEAD;
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

const unsigned char* const * Tamo::getSprite(SPRITE_ID whichSprite){
  const unsigned char* const ** spritesheet = nullptr;
  switch(identity){
    case TAMO:
      spritesheet = tamo_spritesheet;
      break;
    case PORCINI:
      spritesheet = porcini_spritesheet;
      break;
    case BUG:
      spritesheet = bug_spritesheet;
      break;
  }
  return spritesheet[whichSprite];
}

bool Tamo::isFeeling(){
  return (moodTime>0 || !(sprite.isFrameReady() && sprite.hasPlayedAtLeastOnce()));
}
void Tamo::body(){
  if(health>80)
    health-=80;
  // //one write every 10 updates, then change the address every 200
  // //change address if it's been more than 200 updates
  // if(updatesSinceLastWrite >= 200){
  //   healthAddress++;
  //   //wrap around protection (0th byte is identity, and the 1st and 2nd are the health address. So the minimum is 3)
  //   if(healthAddress == 510){
  //     healthAddress = 3;
  //   }
  //   updatesSinceLastWrite = 0;
  //   EEPROM.put(0,healthAddress);
  //   EEPROM.put(healthAddress,health);
  // }
  // //normal update
  // else if(!(updatesSinceLastWrite%10) && updatesSinceLastWrite){
  //   EEPROM.put(healthAddress,health);
  // }
  // updatesSinceLastWrite++;
}

#define GROW_UP_NUMBER 3

void Tamo::baby(){
  //needs 
  uint8_t foodCount = 0;
  while(foodCount < GROW_UP_NUMBER){

  }
}

void Tamo::freePalestine(){
  oled.blink(0);
  oled.bitmap2x(8,0,37,2,free_palestine_bmp);
  lastTime = millis();
  while(!SINGLE_CLICK){
    if(itsbeen(1000))
      readButtons();
  }
  oled.disableFadeOutAndBlinking();
  clearEdges();
  mood = NEUTRAL;
}

void Tamo::feel(){
  Mood nextMood = NEUTRAL;
  switch(mood){
    case RANDOM_NO_TALK:
    case RANDOM:
      vibeCheck();
      return;
    case TALKING:
      talk(thought);
      return;
    case POOPING:
      poop();
      return;
    case NEUTRAL:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,MEDIUM);
      moodTime = 500;
      nextMood = RANDOM;
      break;
    case DEAD:
      dead();
      return;
    case BIRTH:
      birth();
      return;
    case SAD:
      moodTime = 1000;
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(SAD_SPRITE),2,MEDIUM);
      break;
    case SOBBING:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(CRYING_SPRITE),2,FAST);
      moodTime = 1000;
      break;
    case ANGRY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(MAD_SPRITE),2,VVFAST);
      moodTime = 300;
      break;
    case HAPPY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(HAPPY_SPRITE),2,FAST);
      moodTime = 300;
      break;
    case EATING:
      eat();
      return;
    case FREEPALESTINE:
      freePalestine();
      return;
    default:
      mood = RANDOM;
      return;
  }
  basicEmotion(nextMood);
}

void Tamo::basicEmotion(Mood moodAfter){
  while(isFeeling()){
    // updateBreathLED();
    //count down the mood timer
    moodTime--;
    //read inputs
    readButtons();
    //feed tamo!
    if(LONG_PRESS){
      lastTime = millis();
      mood = EATING;
      return;
    }
    //talk to tamo!
    if(SINGLE_CLICK && itsbeen(200)){
      mood = TALKING;
      lastTime = millis();
      return;
    }
    //update and show the current sprite
    sprite.update();
  }
  //check if you should sleep
  hardwareSleepCheck();
  mood = moodAfter;
}

void Tamo::eat(){
  bool justBit = false;
  uint8_t bite = 0;
  Animation food(24,0,16,16,food_animation,5,FAST);
  oled.clear();
  lastTime = millis();
  while(true){
    readButtons();

    //when user presses a button, tamo takes a bite
    //and the food sprite jumps
    if(SINGLE_CLICK){
      lastTime = millis();
      bite = 5;
      if(!justBit){
        clearEdges(SPRITESTARTX+5,0);
      }
      justBit = true;
      if(food.currentFrame == 3){
        food.nextFrame();
        food.showCurrentFrame();
        lastTime = millis();
        while(millis() - lastTime < 200){}
        break;
      }
      else{
        food.nextFrame();
      }
    }
    else{
      bite = 0;
      if(justBit){
        clearEdges(SPRITESTARTX,12);
      }
      justBit = false;
    }
    if(LONG_PRESS && itsbeen(1000)){
      lastTime = millis();
      mood = FREEPALESTINE;
      return;
    }
    food.xCoord = SPRITESTARTX+bite;
    food.showCurrentFrame();
    hardwareSleepCheck();
  }
  if(health < (65535-FOOD_HEALTH_RECOVERY)){
    health += FOOD_HEALTH_RECOVERY;
  }
  else{
    health = 65535;
  }
  //tamo needs to poop!
  setStatusBit(NEEDS_TO_POOP,true);
  //then, tamo eats
  moodTime = 200;
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(EATING_SPRITE),2,VFAST);
  basicEmotion(RANDOM);
}

void Tamo::birth(){
  uint8_t hit = 0;
  sprite = Animation(SPRITESTARTX+32,SPRITESTARTY,16,16,egg_sprite,4,SLOW);
  oled.fill(0);
  lastTime = millis();
  while(true){
    readButtons();
    if(BUTTON && itsbeen(100)){
      lastTime = millis();
      hit = 5;
      if(sprite.currentFrame == 2){
        sprite.nextFrame();
        sprite.showCurrentFrame();
        //wait for 200ms
        while(millis() - lastTime < 200){}
        break;
      }
      else{
        sprite.nextFrame();
      }
    }
    else
      hit = 0;
    if(sprite.xCoord > SPRITESTARTX){
      sprite.xCoord-=2;
    }
    else{
      sprite.xCoord = SPRITESTARTX+hit;
    }
    sprite.showCurrentFrame();
    hardwareSleepCheck();
  }
  lastTime = millis();//to prevent instant-talking
  mood = NEUTRAL;
}

void Tamo::dead(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,death_sprite,2,MEDIUM);
  while(true){
    readButtons();
    hardwareSleepCheck();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    sprite.update();
  }
  //get a new identity
  identity = pseudoRandom(0,3);
  //write it to the eeprom
  EEPROM.put(IDENTITY_ADDRESS,identity);
  mood = BIRTH;
}
void Tamo::talk(Thought t){
  sprite.xCoord = SPRITESTARTX-12;
  const unsigned char* const * animationBuffer = nullptr;
  uint8_t frameCount = 2;
  switch(t){
    case LOVE:
      animationBuffer = talking_love;
      break;
    case FOOD:
      animationBuffer = talking_hunger;
      break;
    case DEATH:
      animationBuffer = talking_death;
      break;
    case HEARTBREAK:
      animationBuffer = talking_heartbreak;
      frameCount = 3;
      break;
    case REVENGE:
      animationBuffer = talking_revenge;
      break;
    case FACE:
      animationBuffer = talking_me;
      frameCount = 3;
      break;
    case MUSIC:
      animationBuffer = talking_music;
      break;
    case MONEY:
      animationBuffer = talking_cash;
      break;
    case DOG:
      animationBuffer = talking_dog;
      break;
    case LOWBATTERY:
      animationBuffer = talking_low_battery;
      break;
    default:
      return;
  }
  TalkingAnimation talkingSprite(SPRITESTARTX+20,SPRITESTARTY,12,16,animationBuffer,frameCount,MEDIUM);
  while(talkingSprite.loopCount<2){
    sprite.update();
    talkingSprite.update();
    // talkingLED();
  }
  mood = RANDOM_NO_TALK;
  clearEdges();
}

void Tamo::poop(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,2,MEDIUM);
  while(true){
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    hardwareSleepCheck();
    sprite.update();
  }
  setStatusBit(NEEDS_TO_POOP,false);
  mood = RANDOM_NO_TALK;
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

void Tamo::batteryCheck(){
  uint16_t vcc = readVcc();
  setStatusBit(LOW_BATTERY,vcc > 640);
}

void Tamo::vibeCheck(){

  //if it's been Xs since the button was pressed, sleep the tamagotchi
  hardwareSleepCheck();
  //check batt voltage
  batteryCheck();

  uint8_t status = OKAY_STATE;
  if(health > GOOD_HEALTH_THRESHOLD){
    status = GOOD_STATE;
  }
  else if(health > BAD_HEALTH_THRESHOLD){
    status = OKAY_STATE;
  }
  else{
    status = BAD_STATE;
  }

  //always talk abt low batt if the batt is low
  if(lowBattery()){
    status = BAD_STATE;
  }
  Mood oldMood = mood;
  do{
    //get your thought/mood based on your state
    switch(status){
      case GOOD_STATE:
        thought = happyThoughts[pseudoRandom(0,3)];
        mood = goodMoods[pseudoRandom(0,4)];
        break;
      case OKAY_STATE:
        thought = neutralThoughts[pseudoRandom(0,6)];
        mood = neutralMoods[pseudoRandom(0,5)];
        break;
      case BAD_STATE:
        thought = sadThoughts[pseudoRandom(0,5)];
        mood = badMoods[pseudoRandom(0,5)];
        break;
    }
  }while(mood == TALKING && oldMood == RANDOM_NO_TALK);//if you can't be talking, try again

  //if you need to poop, there's a 1/10 chance you'll poop
  if(needsToPoop()){
    if((millis()%10) == 9)
      mood = POOPING;
  }
  //if batt is low, it's ALWAYS what you're thinking about
  if(lowBattery()){
    thought = LOWBATTERY;
  }
}