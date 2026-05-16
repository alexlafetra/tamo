#include "FrameBuffer.h"
#include "utils.h"
#include "Sprite.h"
#include "Display.h"

extern FrameBuffer fbo;
extern SSD1306Device oled;

bool Sprite::hasPlayedAtLeastOnce(){
  return loopCount;
}
//returns TRUE if it's been enough time for the next frame to be shown
bool Sprite::isNextFrameReady(){
  if(millis()-timeLastFramePlayed>msPerFrame){
    return true;
  }
  else{
    return false;
  }
}


void printScreenToSerial(){
  Serial.println("*--------------*");
  for(uint8_t y = 0; y<16; y++){
    for(uint8_t x = 0; x<16; x++){
      uint8_t val = fbo.getPixel(x+8,y);
      Serial.print(val?"#":".");
    }
    Serial.println(" ");
  }
  Serial.println("*--------------*");
}

void Sprite::showCurrentFrame(bool clearScreen,bool updateDisplay){
  if(clearScreen)
    fbo.clear();
  fbo.bitmap_from_spritesheet(xCoord,yCoord,width,height,frames[currentFrame]);
  if(updateDisplay){
    // printScreenToSerial();
    oled.renderFBO2x(4,0,36,3,fbo.buffer);
  }
}
void Sprite::showCurrentFrame(){
  showCurrentFrame(true,true);
}

void Sprite::nextFrame(){
  currentFrame++;
  if(currentFrame>=numberOfFrames){
    currentFrame = 0;
    loopCount++;
  }
}
void Sprite::update(bool clearScreen, bool updateDisplay){
  if(isNextFrameReady()){
    nextFrame();
    timeLastFramePlayed = millis();
    showCurrentFrame(clearScreen,updateDisplay);
  }
  else
    showCurrentFrame(clearScreen,updateDisplay);
}
void Sprite::update(){
  update(true,true);
}

  //important! to update both the talking anim and the main sprite this overload needs to clear the screen, but not draw to the oled yet
void TalkingSprite::showCurrentFrame(){
    if(currentFrame){
      fbo.clear();
      fbo.bitmap_from_spritesheet(xCoord,yCoord,width,height,frames[0]);
      fbo.bitmap_from_spritesheet(xCoord+3,yCoord+1,8,7,frames[currentFrame]);
    }
    //if it's the first frame, just send it normally (it'll always be the empty talking sprite)
    else
      Sprite::showCurrentFrame(true,false);
}

void TalkingSprite::update(){
    if(Sprite::isNextFrameReady()){
        Sprite::nextFrame();
        timeLastFramePlayed = millis();
    }
    showCurrentFrame();
}