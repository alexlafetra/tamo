

class Animation{
  public:
    bool hasPlayedAtLeastOnce = false;
    int8_t xCoord;
    int8_t yCoord;
    uint8_t width;
    uint8_t height;
    //stores the current frame
    uint8_t currentFrame;
    //stores total number of frames
    uint8_t numberOfFrames;
    //stores time the last frame played
    uint32_t timeLastFramePlayed;
    //stores milliseconds per frame (framerate)
    uint32_t msPerFrame;
    //checks to see if it's been enough time
    bool isFrameReady();
    //draws current frame
    void showCurrentFrame();
    void update();
    void nextFrame();

    const unsigned char ** frames;

    Animation();
    Animation(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const unsigned char * buffer[], uint16_t frameCount, uint32_t frameRate, uint16_t c);
};

Animation::Animation(){
}

Animation::Animation(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const unsigned char * buffer[], uint16_t frameCount, uint32_t frameRate, uint16_t c){
  currentFrame = 0;
  numberOfFrames = frameCount;
  msPerFrame = frameRate;
  timeLastFramePlayed = millis();
  //allocate mem for pointers to bitmaps
  frames = new const unsigned char *[frameCount];
  //copy in pointers
  for(uint16_t i = 0; i<frameCount; i++){
    frames[i] = buffer[i];
  }
  xCoord = x1;
  yCoord = y1/8;
  width = w;
  height = h;
  hasPlayedAtLeastOnce = false;
}
//returns TRUE if it's been enough time for the next frame to be shown
bool Animation::isFrameReady(){
  if(millis()-timeLastFramePlayed>=msPerFrame){
    return true;
  }
  else{
    return false;
  }
}
void Animation::showCurrentFrame(){
  #ifdef FULLSIZE
  // oled.bitmap2x(xCoord,yCoord/8,xCoord+width,yCoord/8+height/8,frames[currentFrame]);
  oled.bitmap2x(xCoord,yCoord,xCoord+width,yCoord+height/16,frames[currentFrame]);
  #else
  // oled.bitmap(xCoord,yCoord/8,xCoord+width,yCoord/8+height/8,frames[currentFrame]);
  oled.bitmap(xCoord,yCoord,xCoord+width,yCoord+height/16,frames[currentFrame]);
  #endif
}

void Animation::nextFrame(){
  currentFrame++;
  if(currentFrame>=numberOfFrames){
    currentFrame = 0;
    hasPlayedAtLeastOnce = true;
  }
}
void Animation::update(){
  if(isFrameReady()){
    nextFrame();
    timeLastFramePlayed = millis();
    // clearScreen();
  }
  showCurrentFrame();
}