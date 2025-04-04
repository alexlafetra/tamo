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
  SELF,
  FACE,
  MUSIC,
  MONEY,
  DOG
};

enum Idea:uint8_t{
  GUN,
  BIRD,
  HEART
};

enum Mood:uint8_t{
  RANDOM,
  NEUTRAL,
  HAPPY,
  ANGRY,
  SAD,
  SOBBING,
  THINKING,
  TALKING,
  EATING,
  POOPING,
  SLEEPING,
  DEAD,
  BIRTH
};

#define NEEDS_TO_POOP 0
#define LOW_BATTERY 1

class Tamo{
  public:
    Tamo();
    Tamo(Animation a);
    Animation sprite;
    Mood mood = DEAD;
    Thought thought = LOVE;
    int16_t moodTime = 0;
    uint8_t status = 0b00000000;

    /*
      mentalState and hunger start out high, hunger slowly decreases.
      When hunger is 0, then mentalState starts to go down, when hunger is high, mentalState starts to come up
      if mentalState drops below 0, tamo dies
    */
    uint16_t hunger = MAX_HUNGER;
    uint16_t mentalState = MAX_MENTAL;

    // void update();
    void vibeCheck();
    bool isFeeling();
    void feel();
    void body();

    void sleep();
    void talk();
    void talk(Thought t);
    void think();
    void poop();
    void eat();
    void neutral();
    void dead();
    void birth();
    void basicEmotion();
    void basicEmotion(Mood moodAfter);
    void quickEmotion();
    void drawHealth();

    bool getStatusBit(uint8_t which);
    void setStatusBit(uint8_t which, bool state);
    bool needsToPoop();
};

Tamo::Tamo(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,idle_sprite,2,MEDIUM,1);
}
Tamo::Tamo(Animation a){
  sprite = a;
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

bool Tamo::needsToPoop(){
  return getStatusBit(NEEDS_TO_POOP);
}
bool Tamo::isFeeling(){
  return (moodTime>0 || !(sprite.isFrameReady() && sprite.hasPlayedAtLeastOnce()));
}
//hunger on left, mental health on right
void Tamo::drawHealth(){
  uint8_t bar [2] = {0,0};
  uint16_t status = 0xFFFF>>((hunger)/(MAX_HUNGER/16));
  bar[1] = ~uint8_t(status>>8);
  bar[0] = ~uint8_t(status);
  oled.renderFBO2x(4,0,1,2,bar);

  status = 0xFFFF>>((mentalState)/(MAX_MENTAL/16));
  bar[1] = ~uint8_t(status>>8);
  bar[0] = ~uint8_t(status);
  oled.renderFBO2x(62,0,1,2,bar);
}

void Tamo::body(){
  if(hunger){
    hunger-=864;
  }
  else{
    if(mentalState){
      mentalState-=864;
    }
    else{
      mood = DEAD;
    }
  }
}

void Tamo::feel(){
  switch(mood){
    case RANDOM:
      vibeCheck();
      return;
    case TALKING:
      talk();
      return;
    case THINKING:
      think();
      return;
    case POOPING:
      poop();
      return;
    case SLEEPING:
      sleep();
      return;
    case NEUTRAL:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,idle_sprite,2,MEDIUM,1);
      moodTime = 500;
      basicEmotion(RANDOM);
      return;
    case DEAD:
      dead();
      return;
    case BIRTH:
      birth();
      return;
    case SAD:
      moodTime = 1000;
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,sad_sprite,2,MEDIUM,1);
      break;
    case SOBBING:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,crying_sprite,2,FAST,1);
      moodTime = 1000;
      break;
    case ANGRY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,mad_sprite,2,VVFAST,1);
      moodTime = 300;
      break;
    case HAPPY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,happy_sprite,2,FAST,1);
      moodTime = 300;
      break;
    case EATING:
      eat();
      return;
    default:
      mood = RANDOM;
      return;
  }
  basicEmotion();
}

void Tamo::quickEmotion(){
  while(isFeeling()){
    moodTime--;
    sprite.update();
  }
}

void Tamo::basicEmotion(Mood moodAfter){
  while(isFeeling()){
    //count down the mood timer
    moodTime--;
    //read inputs
    readButtons();
    // //feed tamo!
    if(LONG_PRESS){
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
void Tamo::basicEmotion(){
  basicEmotion(NEUTRAL);
}
void Tamo::eat(){
  bool justBit = false;
  uint8_t bite = 0;
  Animation food(SPRITESTARTX+8,SPRITESTARTY,16,16,food_animation,4,FAST,1);
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
        clearEdges(SPRITESTARTX+5,16);
      }
      justBit = true;
      if(food.currentFrame == 2){
        break;
      }
      else{
        food.nextFrame();
      }
    }
    else{
      bite = 0;
      if(justBit){
        clearEdges();
      }
      justBit = false;
    }
    food.xCoord = SPRITESTARTX+bite;
    food.showCurrentFrame();
    hardwareSleepCheck();
  }
  //then, tamo eats
  moodTime = 200;
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,eating_sprite,2,VFAST,1);
  quickEmotion();
  //then, tamo feels happy
  moodTime = 100;
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,happy_sprite,2,FAST,1);
  quickEmotion();
  //then, tamo needs to poop!
  setStatusBit(NEEDS_TO_POOP,true);
  mood = NEUTRAL;
  lastTime = millis();
}

void Tamo::neutral(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,idle_sprite,2,MEDIUM,1);
  moodTime = 500;
  mood = RANDOM;
  lastTime = millis();
  while(isFeeling()){
    moodTime--;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      mood = TALKING;
      break;
    }
    sprite.update();
  }
}

void Tamo::birth(){
  uint8_t hit = 0;
  mentalState = MAX_MENTAL;
  hunger = MAX_HUNGER;
  sprite = Animation(SPRITESTARTX+32,SPRITESTARTY,16,16,egg_sprite,3,SLOW,1);
  clearScreen();
  lastTime = millis();
  while(true){
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      hit = 5;
      if(sprite.currentFrame == 2){
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
  mood = NEUTRAL;
}

void Tamo::dead(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,death_sprite,2,MEDIUM,1);
  while(true){
    readButtons();
    hardwareSleepCheck();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    sprite.update();
  }
  mood = BIRTH;
}
void Tamo::talk(Thought t){
  sprite.xCoord=SPRITESTARTX-12;
  Animation actionSprite;
  switch(t){
    case LOVE:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_love,2,MEDIUM,1);
      break;
    case FOOD:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_hunger,2,MEDIUM,1);
      break;
    case DEATH:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_death,2,MEDIUM,1);
      break;
    case HEARTBREAK:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_heartbreak,3,MEDIUM,1);
      break;
    case REVENGE:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_revenge,2,MEDIUM,1);
      break;
    case SELF: 
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_eye,3,MEDIUM,1);
      break;
    case FACE:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_me,3,MEDIUM,1);
      break;
    case MUSIC:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_music,2,MEDIUM,1);
      break;
    case MONEY:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_cash,2,MEDIUM,1);
      break;
    case DOG:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,talking_dog,2,MEDIUM,1);
      break;
    default:
      return;
  }
  while(actionSprite.loopCount<2){
    sprite.update();
    actionSprite.update();
  }
  mood = RANDOM;
  clearEdges();
}
void Tamo::talk(){
  talk(thought);
}

void Tamo::think(){
  sprite = Animation(SPRITESTARTX-15,SPRITESTARTY,16,16,thinking_sprite,2,FAST,1);
  Animation actionSprite;
  Idea idea = static_cast<Idea>(pseudoRandom(0,3));
  switch(idea){
    case GUN:
      actionSprite = Animation(SPRITESTARTX+17,SPRITESTARTY,13,16,dreamAnim_gun,3,MEDIUM,1);
      break;
    case BIRD:
      actionSprite = Animation(SPRITESTARTX+17,SPRITESTARTY,13,16,dreamAnim_bird,3,MEDIUM,1);
      break;
    case HEART:
      actionSprite = Animation(SPRITESTARTX+17,SPRITESTARTY,13,16,dreamAnim_love,3,MEDIUM,1);
      break;
  }
  moodTime = 500;
  actionSprite.loopCount = 0;
  while(!actionSprite.hasPlayedAtLeastOnce() && moodTime){
    moodTime--;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    sprite.update();
    actionSprite.update();
  }
  clearEdges();
  mood = NEUTRAL;
}

void Tamo::poop(){
  // sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,7,VVFAST,1);
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,2,MEDIUM,1);
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
  mood = NEUTRAL;
}
//sleep mode
void Tamo::sleep(){
  sprite = Animation(32,SPRITESTARTY,16,16,sleeping_sprite,2,MEDIUM,1);
  moodTime = 0;
  lastTime = millis();
  while(!itsbeen(TIME_BEFORE_SLEEP)){
    moodTime++;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      clearEdges();
      mood = RANDOM;
      return;
    }
    sprite.update();
    sprite.xCoord = 0;
    sprite.showCurrentFrame();
    sprite.xCoord = 32;
  }
  hardwareSleep();
  mood = SLEEPING;
}

void Tamo::vibeCheck(){
  //if it's been Xs since the button was pressed, sleep the tamagotchi
  if(itsbeen(TIME_BEFORE_SLEEP)){
    mood = SLEEPING;
    lastTime = millis();
    return;
  }
  if(needsToPoop()){
    const Mood possibleMoods[8] = {NEUTRAL,HAPPY,ANGRY,SAD,THINKING,TALKING,SOBBING,POOPING};
    mood = possibleMoods[pseudoRandom(0,9)];
  }
  else{
    const Mood possibleMoods[8] = {NEUTRAL,HAPPY,ANGRY,SAD,THINKING,TALKING,SOBBING};
    mood = possibleMoods[pseudoRandom(0,8)];
  }

  // const Thought possibleThoughts[8] = {LOVE,HEARTBREAK,DEATH,REVENGE,FOOD,SELF,FACE,MUSIC};
  // thought = possibleThoughts[pseudoRandom(0,8)];
  thought = static_cast<Thought>(pseudoRandom(0,10));
}