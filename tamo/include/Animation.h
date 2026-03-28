

class Animation{
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

    void nextFrame();

    //combines isNextFrameReady() and showCurrentFrame() and nextFrame()
    void update();

    bool hasPlayedAtLeastOnce();

    //ptr to the frame data
    const uint16_t * frames;

    Animation(){};
    Animation(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const uint16_t* buffer, uint16_t frameCount, uint32_t frameRate){
      numberOfFrames = frameCount;
      msPerFrame = frameRate;
      //store pointer to the sprite offsets (this pointer is an array of offsets)
      frames = buffer;
      xCoord = x1;
      yCoord = y1/8;
      width = w;
      height = h;
    } __attribute__((noinline));
};

bool Animation::hasPlayedAtLeastOnce(){
  return loopCount;
}
//returns TRUE if it's been enough time for the next frame to be shown
bool Animation::isNextFrameReady(){
  if(millis()-timeLastFramePlayed>=msPerFrame){
    return true;
  }
  else{
    return false;
  }
}
void Animation::showCurrentFrame(){
  oled.bitmap_from_spritesheet2x(xCoord,yCoord,xCoord+width,yCoord+height/16,pgm_read_word(&frames[currentFrame]));
}

void Animation::nextFrame(){
  currentFrame++;
  if(currentFrame>=numberOfFrames){
    currentFrame = 0;
    loopCount++;
  }
}
void Animation::update(){
  if(isNextFrameReady()){
    nextFrame();
    timeLastFramePlayed = millis();
  }
  showCurrentFrame();
}

/*
full speech bubble is 12x11
small icons are 7x7
so they need to be offset by (5,4)
*/

class TalkingAnimation:public Animation{
  public:
  TalkingAnimation(){}
  TalkingAnimation(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const uint16_t* buffer, uint16_t frameCount, uint32_t frameRate){
    currentFrame = 0;
    numberOfFrames = frameCount;
    msPerFrame = frameRate;
    timeLastFramePlayed = millis();

    //allocate mem for pointers to bitmaps
    frames = buffer;
    xCoord = x1;
    yCoord = y1/8;
    width = w;
    height = h;
    loopCount = 0;
  } __attribute__((noinline));
  void showCurrentFrame(){
    if(currentFrame)
      oled.overlay_bitmap_from_spritesheet2x(xCoord,yCoord,xCoord+width,yCoord+height/16,pgm_read_word(&frames[0]),3,1,7,7,pgm_read_word(&frames[currentFrame]));
    //if it's the first frame, just send it normally (it'll always be the empty talking sprite)
    else
      Animation::showCurrentFrame();
  }
  void update(){
    if(Animation::isNextFrameReady()){
      Animation::nextFrame();
      timeLastFramePlayed = millis();
    }
    showCurrentFrame();
  }
};