#pragma once

#include "FrameBuffer.h"
#include "utils.h"

extern FrameBuffer fbo;
extern uint32_t lastTime;
extern volatile bool BUTTON;
extern volatile bool LONG_PRESS;
extern volatile bool DOUBLE_CLICK;
extern volatile bool SINGLE_CLICK;

class Sprite{
  public:
    //number of times the anim has looped
    uint8_t loopCount = 0;

    //placement of the bitmap
    int8_t xCoord = 0;
    int8_t yCoord = 0;
    uint8_t width = 0;
    uint8_t height = 0;

    //stores the current frame
    uint8_t currentFrame = 0;

    //stores total number of frames
    uint8_t numberOfFrames = 0;

    //stores time the last frame played
    uint32_t timeLastFramePlayed = millis();

    //stores milliseconds per frame (framerate)
    uint32_t msPerFrame = 0;

    //checks to see if it's been enough time to jump to the next frame
    bool isNextFrameReady();

    //draws current frame to the screen
    void showCurrentFrame();
    void showCurrentFrame(bool clearScreen,bool updateDisplay);

    void nextFrame();

    //combines isNextFrameReady() and showCurrentFrame() and nextFrame()
    void update();
    void update(bool clearScreen, bool updateDisplay);

    bool hasPlayedAtLeastOnce();

    //ptr to the frame data
    const uint16_t * frames;

    Sprite(){};
    Sprite(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const uint16_t* buffer, uint16_t frameCount, uint32_t frameRate){
      numberOfFrames = frameCount;
      msPerFrame = frameRate;
      //store pointer to the sprite offsets (this pointer is an array of offsets)
      frames = buffer;
      xCoord = x1;
      yCoord = y1;
      width = w;
      height = h;
    } __attribute__((noinline));
};

/*
full speech bubble is 12x11
small icons are 7x7
so they need to be offset by (5,4)
*/

class TalkingSprite:public Sprite{
  public:
  TalkingSprite(){}
  TalkingSprite(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const uint16_t* buffer, uint16_t frameCount, uint32_t frameRate){
    currentFrame = 0;
    numberOfFrames = frameCount;
    msPerFrame = frameRate;
    timeLastFramePlayed = millis();

    //allocate mem for pointers to bitmaps
    frames = buffer;
    xCoord = x1;
    yCoord = y1;
    width = w;
    height = h;
    loopCount = 0;
  } __attribute__((noinline));
  //important! to update both the talking anim and the main sprite this overload needs to clear the screen, but not draw to the oled yet
  void showCurrentFrame();
  void update();
};