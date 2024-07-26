#define SPRITESTARTX 20
#define SPRITESTARTY 0

//Speeds for Sprites
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
  SLEEPING
};


class Tamo{
  public:
    Tamo();
    Tamo(Animation);
    Animation sprite;
    Animation actionSprite;
    Mood mood = NEUTRAL;
    Thought thought = LOVE;
    int16_t moodTime = 0;

    void update();
    void vibeCheck();
    void talk();
    void think();
    void poop();
    void sleep();
    void cry();
    void eat();
    void feelLikeCrying();
    void feelMad();
    void neutral();
    void happy();
    bool isFeeling();
};

Tamo::Tamo(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,debugFella,2,MEDIUM,1);
}
Tamo::Tamo(Animation a){
  sprite = a;
}


bool Tamo::isFeeling(){
  return (moodTime>0 || !(sprite.isFrameReady() && sprite.hasPlayedAtLeastOnce));
}

//0 is love, 1 is hearbreak, 2 is a face, 3 is the moon, 4 is a gun
void Tamo::talk(){
  sprite = Animation(SPRITESTARTX-16,SPRITESTARTY,16,16,debugFella,2,MEDIUM,1);
  switch(thought){
    case LOVE:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,unbrokenHeart,2,MEDIUM,1);
      break;
    case FOOD:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,moon,2,MEDIUM,1);
      break;
    case DEATH:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,skull,2,MEDIUM,1);
      break;
    case HEARTBREAK:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,brokenHeart,3,MEDIUM,1);
      break;
    case REVENGE:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,gun,2,MEDIUM,1);
      break;
    case SELF:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,eyeball,3,MEDIUM,1);
      break;
    case FACE:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,face,3,MEDIUM,1);
      break;
    case MUSIC:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,16,music,2,MEDIUM,1);
      break;
  }
  moodTime = random(100,500);
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
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,thinkingAnim,2,FAST,1);
  moodTime = random(100,500);
  bool headEmpty = true;
  actionSprite.hasPlayedAtLeastOnce = false;
  while(!actionSprite.hasPlayedAtLeastOnce){
    moodTime--;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    if(headEmpty){
      if(random(0,10) <= 8){//80% chance tamo will think of something
        headEmpty = false;
        sprite = Animation(SPRITESTARTX-16,SPRITESTARTY,16,16,thinkingAnim,2,FAST,1);
        actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,13,16,dreamAnim_gun,4,SLOW,1);
      }
    }
    sprite.update();
    if(!headEmpty){
      actionSprite.update();
    }
  }
  if(!headEmpty)
    clearEdges();
  mood = NEUTRAL;
}

void Tamo::eat(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,eatingAnim,2,MEDIUM,1);
  moodTime = random(200,700);
  while(isFeeling()){
    moodTime--;
    readButtons();
    sprite.update();
  }
  mood = HAPPY;
}
void Tamo::poop(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,7,VFAST,1);
  bool pooped = true;
  while(pooped){
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      pooped = false;
    }
    sprite.update();
  }
  mood = NEUTRAL;
}
void Tamo::happy(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,happyAnim,2,FAST,1);
  moodTime = random(100,300);
  while(isFeeling()){
    moodTime--;
    readButtons();
    sprite.update();
  }
  mood = NEUTRAL;
}

void Tamo::neutral(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,debugFella,2,MEDIUM,1);
  moodTime = random(200,1000);
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
  if(mood == NEUTRAL)
    mood = RANDOM;
}

//sleep mode
void Tamo::sleep(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,sleepingAnim,2,SLOW,1);
  moodTime = 1000;
  while(moodTime>0){
    moodTime--;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      mood = ANGRY;
      break;
    }
    sprite.update();
  }
  if(mood != ANGRY){
    hardwareSleep();
    mood = NEUTRAL;
  }
}

void Tamo::cry(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,cryingAnim,2,FAST,1);
  moodTime = random(100,500);
  while(isFeeling()){
    readButtons();
    moodTime--;
    sprite.update();
  }
  mood = NEUTRAL;
}

void Tamo::feelLikeCrying(){
  moodTime = random(100,500);
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,vergeOfTearsAnim,2,MEDIUM,1);
  while(isFeeling()){
    moodTime--;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      mood = EATING;
      break;
    }
    sprite.update();
  }
  if(mood != EATING)
    mood = SOBBING;
}

void Tamo::feelMad(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,tamoMadAnim,2,VFAST,1);
  moodTime = random(100,200);
  while(isFeeling()){
    readButtons();
    sprite.update();
    moodTime--;
  }
  mood = NEUTRAL;
}

void Tamo::vibeCheck(){
  //if it's been 30s since the button was pressed, sleep the tamagotchi
  if(itsbeen(60000)){
  // if(itsbeen(6000)){
    mood = SLEEPING;
    lastTime = millis();
    return;
  }
  //otherwise, choose a random mood
  const Mood possibleMoods[7] = {NEUTRAL,HAPPY,ANGRY,SAD,THINKING,EATING,POOPING};
  mood = possibleMoods[random(0,7)];
  const Thought possibleThoughts[9] = {LOVE,HEARTBREAK,DEATH,REVENGE,FOOD,SELF,FACE,MUSIC};
  thought = possibleThoughts[random(0,9)];
}

void Tamo::update(){
  switch(mood){
    case RANDOM:
      vibeCheck();
      break;
    case NEUTRAL:
      neutral();
      break;
    case HAPPY:
      happy();
      break;
    case ANGRY:
      feelMad();
      break;
    case SAD:
      feelLikeCrying();
      break;
    case SOBBING:
      cry();
      break;
    case THINKING:
      think();
      break;
    case TALKING:
      talk();
      break;
    case EATING:
      eat();
      break;
    case POOPING:
      poop();
      break;
    case SLEEPING:
      sleep();
      break;
  }
}