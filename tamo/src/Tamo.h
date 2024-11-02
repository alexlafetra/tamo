#define SPRITESTARTX 16
#define SPRITESTARTY 0

//Speeds for Sprites
#define VVVFAST 50
#define VVFAST 100
#define VFAST 200
#define FAST 400
#define MEDIUM 800
#define SLOW 1200

enum Thought:uint8_t{
  LOVE,
  HEARTBREAK,
  DEATH,
  REVENGE,
  FOOD,
  SELF,
  FACE,
  MUSIC
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


class Tamo{
  public:
    Tamo();
    Tamo(Animation a);
    Animation sprite;
    Mood mood = NEUTRAL;
    // Mood mood = BIRTH;
    Thought thought = LOVE;
    int16_t moodTime = 0;

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
    void action();

    void sleep();
    void talk();
    void think();
    void poop();
    void neutral();
    void dead();
    void birth();
    void drawHealth();
};

Tamo::Tamo(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,idle_sprite,2,MEDIUM,1);
}
Tamo::Tamo(Animation a){
  sprite = a;
}

bool Tamo::isFeeling(){
  return (moodTime>0 || !(sprite.isFrameReady() && sprite.hasPlayedAtLeastOnce));
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
      neutral();
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
      mood = NEUTRAL;
      break;
    case SOBBING:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,crying_sprite,2,FAST,1);
      moodTime = 1000;
      mood = NEUTRAL;
      break;
    case ANGRY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,mad_sprite,2,VVFAST,1);
      moodTime = 300;
      mood = NEUTRAL;
      break;
    case HAPPY:
      sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,happy_sprite,2,FAST,1);
      moodTime = 300;
      mood = NEUTRAL;
      break;
    default:
      mood = RANDOM;
      return;
  }
  while(isFeeling()){
    moodTime--;
    readButtons();
    sprite.update();
  }
}

void Tamo::action(){
  mood = TALKING;
}

void Tamo::neutral(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,idle_sprite,2,MEDIUM,1);
  // moodTime = 6000;
  moodTime = 500;
  mood = RANDOM;
  bool showingHealth = false;
  while(isFeeling()){
    moodTime--;
    readButtons();
    if(BUTTON){
      //double click
      if(DOUBLE_CLICK){
        lastTime = millis();
        mood = DEAD;
        break;
      }
      else if(LONG_PRESS){
        lastTime = millis();
        action();
        break;
      }
      else{
        lastTime = millis();
        showingHealth = true;
      }
    }
    if(itsbeen(1000) && showingHealth){
      showingHealth = false;
      clearEdges();
    }
    sprite.update();
    if(showingHealth)
      drawHealth();
  }
  clearEdges();
}

void Tamo::birth(){
  uint8_t eggState = 0;
  uint8_t hit = 0;
  mentalState = MAX_MENTAL;
  hunger = MAX_HUNGER;
  sprite = Animation(SPRITESTARTX+32,SPRITESTARTY,16,16,egg_sprite,3,SLOW,1);
  clearScreen();
  while(true){
    readButtons();
    hardwareSleepCheck();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      hit = 5;
      eggState++;
      if(eggState == 3){
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
  }
  mood = NEUTRAL;
}

void Tamo::dead(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,death_sprite,2,SLOW,1);
  while(true){
    readButtons();
    hardwareSleepCheck();
    if(LONG_PRESS){
      lastTime = millis();
      break;
    }
    sprite.update();
  }
  mood = BIRTH;
}

//0 is love, 1 is hearbreak, 2 is a face, 3 is the moon, 4 is a gun
void Tamo::talk(){
  sprite = Animation(SPRITESTARTX-12,SPRITESTARTY,16,16,idle_sprite,2,MEDIUM,1);
  Animation actionSprite;
  switch(thought){
    case LOVE:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,unbrokenHeart,2,MEDIUM,1);
      break;
    case FOOD:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,moon,2,MEDIUM,1);
      break;
    case DEATH:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,skull,2,MEDIUM,1);
      break;
    case HEARTBREAK:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,brokenHeart,3,MEDIUM,1);
      break;
    case REVENGE:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,gun,2,MEDIUM,1);
      break;
    case SELF:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,eyeball,3,MEDIUM,1);
      break;
    case FACE:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,face,3,MEDIUM,1);
      break;
    case MUSIC:
      actionSprite = Animation(SPRITESTARTX+20,SPRITESTARTY,12,16,music,2,MEDIUM,1);
      break;
    default:
      return;
  }
  moodTime = 500;
  while(isFeeling() || (!actionSprite.hasPlayedAtLeastOnce && !actionSprite.isFrameReady())){
    readButtons();
    moodTime--;
    sprite.update();
    actionSprite.update();
  }
  clearEdges();
  mood = NEUTRAL;
}

void Tamo::think(){
  sprite = Animation(SPRITESTARTX-16,SPRITESTARTY,16,16,thinking_sprite,2,FAST,1);
  Animation actionSprite(SPRITESTARTX+16,SPRITESTARTY,13,16,dreamAnim_gun,4,SLOW,1);
  moodTime = 500;
  actionSprite.hasPlayedAtLeastOnce = false;
  while(!actionSprite.hasPlayedAtLeastOnce && moodTime){
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
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,7,VVFAST,1);
  while(true){
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    hardwareSleepCheck();
    sprite.update();
  }
  mood = NEUTRAL;
}
//sleep mode
void Tamo::sleep(){
  sprite = Animation(32,SPRITESTARTY,16,16,sleeping_sprite,4,FAST,1);
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
  //otherwise, choose a random mood
  // Mood possibleMoods[7] = {NEUTRAL,HAPPY,ANGRY,SAD,THINKING,POOPING,EATING};
  if(mentalState > 230){
    Mood possibleMoods[9] = {NEUTRAL,HAPPY,HAPPY,HAPPY,TALKING,TALKING,THINKING,POOPING,POOPING};
    mood = possibleMoods[pseudoRandom(0,9)];
  }
  else if(mentalState > 180){
    Mood possibleMoods[9] = {NEUTRAL,NEUTRAL,NEUTRAL,TALKING,TALKING,THINKING,THINKING,POOPING,POOPING};
    mood = possibleMoods[pseudoRandom(0,9)];
  }
  else{
    mood = SAD;
  }
  const Thought possibleThoughts[8] = {LOVE,HEARTBREAK,DEATH,REVENGE,FOOD,SELF,FACE,MUSIC};
  thought = possibleThoughts[pseudoRandom(0,8)];
}