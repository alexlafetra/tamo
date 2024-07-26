#define SPRITESTARTX 20
#define SPRITESTARTY 0


class Tamo{
  public:
    Tamo();
    Tamo(Animation);
    Animation sprite;
    Animation actionSprite;

    void update();
    void vibeCheck();
    void talk();
    void talk(uint8_t what);
    void think();
    void poop();
    void sleep();
    void cry();
    void eat();
    void feelLikeCrying();
    void feelMad();
    void neutral();
    void happy();
};

Tamo::Tamo(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,debugFella,2,800,1);
}
Tamo::Tamo(Animation a){
  sprite = a;
}

//0 is love, 1 is hearbreak, 2 is a face, 3 is the moon, 4 is a gun
void Tamo::talk(uint8_t what){
  sprite = Animation(SPRITESTARTX-16,SPRITESTARTY,16,16,debugFella,2,800,1);
  switch(what){
    case 0:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,11,unbrokenHeart,2,1600,1);
      break;
    case 1:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,11,moon,2,1600,1);
      break;
    case 2:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,11,face,2,1600,1);
      break;
    case 3:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,11,brokenHeart,3,1600,1);
      break;
    case 4:
      actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,12,11,gun,2,1600,1);
      break;
  }
  uint16_t talkTime = random(100,500);
  while(talkTime>0){
    readButtons();
    if(itsbeen(500) && BUTTON){
      lastTime = millis();
      break;
    }
    talkTime--;
    sprite.update();
    actionSprite.update();
  }
  oled.clear();
}
void Tamo::talk(){
  srand(millis());
  talk(random(0,5));
}
void Tamo::think(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,thinkingAnim,2,300,1);
  uint16_t thinkTime = random(100,500);
  bool headEmpty = true;
  while(thinkTime>0){
    thinkTime--;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    if(headEmpty){
      if(random(0,10) <= 8){//80% chance tamo will think of something
        headEmpty = false;
        sprite = Animation(SPRITESTARTX-16,SPRITESTARTY,16,16,thinkingAnim,2,300,1);
        actionSprite = Animation(SPRITESTARTX+16,SPRITESTARTY,13,13,badDreamAnim,4,500,1);
      }
    }
    sprite.update();
    if(!headEmpty){
      actionSprite.update();
    }
  }
  if(!headEmpty)
    oled.clear();
}
void Tamo::eat(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,eatingAnim,2,1600,1);
  uint16_t eatTime = random(200,700);
  while(eatTime>0){
    eatTime--;
    readButtons();
    sprite.update();
  }
  happy();
}
void Tamo::poop(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,poopAnim,7,200,1);
  bool pooped = true;
  while(pooped){
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      pooped = false;
    }
    sprite.update();
  }
}
void Tamo::happy(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,happyAnim,2,300,1);
  uint16_t happyTime = random(200,500);
  while(happyTime>0){
    happyTime--;
    readButtons();
    sprite.update();
  }
}

void Tamo::neutral(){
  // timeOfLastVibe = millis();
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,debugFella,2,800,1);
  uint16_t neutralTime = random(100,300);
  while(neutralTime>0){
    neutralTime--;
    readButtons();
    if(BUTTON && itsbeen(200)){
      lastTime = millis();
      break;
    }
    sprite.update();
  }
}

//sleep mode
void Tamo::sleep(){
  bool getMad = false;
  {
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,sleepingAnim,2,1600,1);
    uint16_t sleepTime = 1000;
    while(sleepTime>0){
      sleepTime--;
      readButtons();
      if(BUTTON && itsbeen(200)){
        lastTime = millis();
        getMad = true;
        break;
      }
      sprite.update();
    }
  }
  if(getMad)
    feelMad();
  else
    hardwareSleep();
}

void Tamo::cry(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,cryingAnim,2,800,1);
  uint32_t cryTime = random(100,500);
  while(cryTime>0){
    readButtons();
    cryTime--;
    sprite.update();
  }
}

void Tamo::feelLikeCrying(){
  bool eating = false;
  {
    int16_t cryTime = random(100,500);
    sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,vergeOfTearsAnim,2,600,1);
    while(cryTime>0){
      cryTime--;
      readButtons();
      if(BUTTON && itsbeen(200)){
        lastTime = millis();
        eating = true;
        break;
      }
      sprite.update();
    }
  }
  if(eating)
    eat();
  else
    cry();
}

void Tamo::feelMad(){
  sprite = Animation(SPRITESTARTX,SPRITESTARTY,16,16,tamoMadAnim,2,200,1);
  uint16_t angeryTime = random(100,200);
  while(angeryTime>0){
    readButtons();
    sprite.update();
    angeryTime--;
  }
}

void Tamo::vibeCheck(){
  //if it's been 30s since the button was pressed, sleep the tamagotchi
  if(itsbeen(60000)){
    sleep();
    lastTime = millis();
  }
  uint8_t voibe = random(0,5);
  switch(voibe){
    case 0:
      feelLikeCrying();
      break;
    case 1:
      poop();
      break;
    case 2:
      feelMad();
      break;
    case 3:
      think();
      break;
    case 4:
      talk();
      break;
    case 5:
      happy();
      break;
  }
}

void Tamo::update(){
  vibeCheck();
}