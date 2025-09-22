//time (ms) before tamo sleeps
// #define TIME_BEFORE_SLEEP 60000
#define TIME_BEFORE_SLEEP 6000

#define MAX_HUNGER 4320 // takes 12hr = 720min = 4320s to run out
#define MAX_MENTAL 8640 //takes 24hr to fully deplete

#define TAMO 0
#define PORCINI 1
#define BUG 2
#define VISHAY 3

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
  EATING,
  POOPING,
  DEAD,
  BIRTH,
  FREEPALESTINE,
  LOWBATT
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

const Mood goodMoods[1] = {HAPPY};
const Mood neutralMoods[1] = {NEUTRAL};
const Mood badMoods[3] = {SAD,SOBBING,ANGRY};

#define GOOD_STATE 0
#define OKAY_STATE 1
#define BAD_STATE 2

#define FOOD_HEALTH_RECOVERY 1800//equivalent to 3hr worth of health

#define NEEDS_TO_POOP 0
#define LOW_BATTERY 1
#define IS_ASLEEP 2 

#define HEALTH_ADDRESS_ADDRESS 1
#define IDENTITY_ADDRESS 0

class Tamo{
  public:
    Tamo();
    Animation sprite;
    Mood mood = BIRTH;
    // Mood mood = NEUTRAL;
    // Mood mood = DEAD;
    Thought thought = LOVE;
    int16_t moodTime = 0;
    //status register (volatile so that sleep can be turned off from interrupts)
    volatile uint8_t status = 0b00000000;
    uint8_t identity = 0;//which sprites to chose from

    /*
      health decreases every 8 seconds. Food resets it
    */
    volatile uint16_t health = 65535;
    uint16_t healthAddress = 1;
    uint8_t updatesSinceLastWrite = 0;

    bool isFeeling();
    void body();
    //runs an update cycle
    void live();
    //gets new emotion/current state
    void vibeCheck();
    //runs the current emotion
    void feel();
    void cycleThruMoods();

    void setMoodSprite(Mood);

    void basicEmotion();
    void wakeUp();
    void idle();
    void talk(Thought t);
    void poop();
    void eat();
    void dead();
    void birth();
    void lowBattEmotion();
    bool getStatusBit(uint8_t which);
    void setStatusBit(uint8_t which, bool state);
    bool needsToPoop(){return getStatusBit(NEEDS_TO_POOP);}
    bool isAsleep(){return getStatusBit(IS_ASLEEP);}
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
    case VISHAY:
      spritesheet = vishay_spritesheet;
  }
  return spritesheet[whichSprite];
}

bool Tamo::isFeeling(){
  return (moodTime>0 || !(sprite.isFrameReady() && sprite.hasPlayedAtLeastOnce()));
}
void Tamo::body(){
  if(health>80)
    health-=1000;
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

void Tamo::feel(){
  switch(mood){
    case RANDOM_NO_TALK:
    case RANDOM:
      vibeCheck();
      return;
    case POOPING:
      poop();
      return;
    case DEAD:
      dead();
      return;
    case BIRTH:
      birth();
      return;
    case EATING:
      eat();
      return;
    case SAD:
      setMoodSprite(mood);
      moodTime = 1000;
      break;
    case SOBBING:
      setMoodSprite(mood);
      moodTime = 1000;
      break;
    case ANGRY:
      setMoodSprite(mood);
      moodTime = 300;
      break;
    case HAPPY:
      setMoodSprite(mood);
      moodTime = 300;
      break;
    case LOWBATT:
      lowBattEmotion();
      break; 
    case NEUTRAL:
    default:
      setMoodSprite(mood);
      moodTime = 500;
      break;   
  }
  moodTime = 1;
  basicEmotion();
}

void Tamo::cycleThruMoods(){
  uint8_t current = 0;
  const Mood moods[] = {NEUTRAL,SAD,SOBBING,ANGRY,HAPPY};
  while(true){
    // for(uint16_t i = 0; i<16535; i++){
    //   setMoodSprite(moods[i%5]);
    //   readButtons();
    // }
    hardwareSleepCheck();
    mood = moods[current];
    // setMoodSprite(mood);
    feel();
    current=(current+1)%5;
    // idle();
    // mood = NEUTRAL;
    // feel();
  }
}

void Tamo::lowBattEmotion(){
}

void Tamo::idle(){
  setMoodSprite(NEUTRAL);//set to a neutral sprite, even though you're not feeling neutral
  moodTime = 500;
  basicEmotion();
}

void Tamo::wakeUp(){

}

//don't call this! you need to set sprites to use it, so only call it from feel()
void Tamo::basicEmotion(){
  while(isFeeling()){
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
      lastTime = millis();
      talk(thought);
    }
    //update and show the current sprite
    sprite.update();
  }
}

void Tamo::eat(){
  bool justBit = false;
  uint8_t bite = 0;
  sprite = Animation(SPRITESTARTX,0,16,16,food_animation,5,FAST);
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
      if(sprite.currentFrame == 3){
        sprite.nextFrame();
        sprite.showCurrentFrame();
        lastTime = millis();
        while(millis() - lastTime < 200){}
        break;
      }
      else{
        sprite.nextFrame();
      }
    }
    else{
      bite = 0;
      if(justBit){
        clearEdges(SPRITESTARTX,12);
      }
      justBit = false;
    }
    sprite.xCoord = SPRITESTARTX+bite;
    sprite.showCurrentFrame();
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
  while(!itsbeen(1000)){
    sprite.update();
  }
  mood = RANDOM_NO_TALK;
  lastTime = millis();
}

void Tamo::birth(){
  //check to see if the identity has already been set
  EEPROM.get(IDENTITY_ADDRESS,identity);
  //if it's not -1, identity was already set! so don't get reborn;
  // if(identity != 255){
  //   return;
  // }

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
        while(millis() - lastTime < 400){}
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
  // identity = millis()%4;
  identity = VISHAY;
  //write the new identity into eeprom
  EEPROM.put(identity,IDENTITY_ADDRESS);
  lastTime = millis();//to prevent instant-talking
  // vibeCheck();//get a new mood ASAP
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
  // identity = pseudoRandom(0,2);
  // identity = millis()%3;
  // identity = 2;
  // identity = PORCINI;
  //write it to the eeprom
  // EEPROM.put(IDENTITY_ADDRESS,identity);
  mood = BIRTH;
}
void Tamo::talk(Thought t){
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
  setMoodSprite(mood); //get the actual mood sprite
  sprite.xCoord = SPRITESTARTX-12;//move sprite to the left

  //talk for 2 cycle counts
  while(talkingSprite.loopCount<2){
    sprite.update();
    talkingSprite.update();
  }
  clearEdges();//clear screen edges
  sprite.xCoord = SPRITESTARTX;//move sprite back
}

void Tamo::setMoodSprite(Mood m){
  switch(m){
    case NEUTRAL:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(IDLE_SPRITE),2,MEDIUM);
      return;
    case SAD:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(SAD_SPRITE),2,MEDIUM);
      return;
    case SOBBING:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(CRYING_SPRITE),2,FAST);
      return;
    case ANGRY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,getSprite(MAD_SPRITE),2,VVFAST);
      return;
    case HAPPY:
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
  while(true){
    readButtons();
    if(BUTTON && itsbeen(400)){
      lastTime = millis();
      break;
    }
    hardwareSleepCheck();
    sprite.update();
  }
  setStatusBit(NEEDS_TO_POOP,false);
  mood = RANDOM;
}

void Tamo::live(){
  //check if you should sleep
  hardwareSleepCheck();
  //experience current emotion (talking happens inside basicEmotion())
  feel();
  //after every other emotion, go idle to let the emotions feel more important
  idle();
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

void Tamo::batteryCheck(){
  uint16_t vcc = readVcc();
  setStatusBit(LOW_BATTERY,vcc > 640);
}

//health starts at 65535 and goes down every 9s by 1 (drops to 0 in 6.8 days)
#define GOOD_HEALTH_THRESHOLD 55535 //drops to this in 25hrs
#define BAD_HEALTH_THRESHOLD 32768 //drops to this in 3 days

void Tamo::vibeCheck(){
  //check batt voltage
  batteryCheck();

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

  //always talk abt low batt if the batt is low
  if(lowBattery()){
    currentState = BAD_STATE;
    thought = LOWBATTERY;
    mood = LOWBATT;
    return;
  }

  //get your thought/mood based on your state
  switch(currentState){
    case GOOD_STATE:
      thought = happyThoughts[millis()%5];
      mood = HAPPY;//always happy
      break;
    case OKAY_STATE:
      thought = neutralThoughts[millis()%6];
      mood = NEUTRAL;
      break;
    case BAD_STATE:
      thought = sadThoughts[millis()%5];
      mood = badMoods[millis()%3]; // sad, crying, angry, talking
      break;
  }
  
  //if you need to poop, there's a 1/10 chance you'll poop
  if(needsToPoop()){
    if((millis()%10) == 9)
      mood = POOPING;
  }
  mood = SOBBING;
}