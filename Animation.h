

class Animation{
  public:
    //either 0 (stopped), 1 (playing forward), or -1 (playing backward)
    int8_t playState;
    bool hasPlayedAtLeastOnce;
    int8_t xCoord;
    int8_t yCoord;
    uint8_t width;
    uint8_t height;
    // uint16_t color;
    //stores the current frame
    uint8_t currentFrame;
    //stores total number of frames
    uint8_t numberOfFrames;
    //stores time the last frame played
    uint32_t timeLastFramePlayed;
    //stores milliseconds per frame (framerate)
    uint32_t msPerFrame;
    Animation();
    Animation(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const unsigned char * buffer[], uint16_t frameCount, uint32_t frameRate, uint16_t c);
    //checks to see if it's been enough time
    bool isFrameReady();
    //draws current frame
    void showCurrentFrame();
    void update();
    void nextFrame();
    Animation * introAnim;
    bool introAnimation;
    const unsigned char ** frames;

    //destructor
    // ~Animation(){
    //   delete frames;
    // }
};

Animation::Animation(){
}

Animation::Animation(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const unsigned char * buffer[], uint16_t frameCount, uint32_t frameRate, uint16_t c){
  playState = 1;
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
  yCoord = y1;
  width = w;
  height = h;
  // color = c;
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
  oled.bitmap2x(xCoord,yCoord/8,xCoord+width,yCoord/8+height/8,frames[currentFrame]);
  #else
  oled.bitmap(xCoord,yCoord/8,xCoord+width,yCoord/8+height/8,frames[currentFrame]);
  #endif
}

void Animation::nextFrame(){
  switch(playState){
    //stopped, just return
    case 0:
      break;
    case 1:
      currentFrame++;
      if(currentFrame>=numberOfFrames){
        currentFrame = 0;
        hasPlayedAtLeastOnce = true;
      }
      break;
    case -1:
      if(currentFrame == 0){
        currentFrame = numberOfFrames-1;
        hasPlayedAtLeastOnce = true;
      }
      else
        currentFrame--;
      break;
  }
}
void Animation::update(){
  //if it's not playing, just return
  if(playState == 0){
    return;
  }
  //if the intro animation is still on
  if(introAnimation){
    // (introAnim)->update();
    // if(introAnim->hasPlayedAtLeastOnce){
    //   introAnimation = false;
    // }
    // return;
  }
  if(isFrameReady()){
    nextFrame();
    timeLastFramePlayed = millis();
  }
  showCurrentFrame();
  // const int16_t limit = 100;
  // analogWrite(LED_PIN,abs(limit/2-((millis()/100)%limit)));
}