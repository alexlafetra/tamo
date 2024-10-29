

class Animation{
  public:
    bool hasPlayedAtLeastOnce = false;
    int8_t xCoord = 0;
    int8_t yCoord = 0;
    uint8_t width = 0;
    uint8_t height = 0;
    //stores the current frame
    uint8_t currentFrame = 0;
    //stores total number of frames
    uint8_t numberOfFrames = 0;
    //stores time the last frame played
    uint32_t timeLastFramePlayed = 0;
    //stores milliseconds per frame (framerate)
    uint32_t msPerFrame = 0;
    //checks to see if it's been enough time
    bool isFrameReady();
    //draws current frame
    void showCurrentFrame();
    void update();
    void nextFrame();
    const unsigned char ** frames = nullptr;

    Animation(){};
    Animation(int16_t x1, int16_t y1, uint16_t w, uint16_t h, const unsigned char* const buffer[], uint16_t frameCount, uint32_t frameRate, uint16_t c){
      currentFrame = 0;
      numberOfFrames = frameCount;
      msPerFrame = frameRate;
      timeLastFramePlayed = millis();

      //allocate mem for pointers to bitmaps
      frames = new const unsigned char *[frameCount];
      //copy in pointers
      for(uint16_t i = 0; i<frameCount; i++){
        //need to access the bitmap data like this since the arrays of pointers to the bitmaps are also stored as PROGMEM
        //see: https://stackoverflow.com/questions/63447172/issues-with-pointers-and-progmem
        frames[i] = pgm_read_ptr(buffer+i);
      }
      xCoord = x1;
      yCoord = y1/8;
      width = w;
      height = h;
      hasPlayedAtLeastOnce = false;
    }
    //destructor, bc we dynamically allocate mem for the frame data
    ~Animation(){
      //deallocate mem
      delete [] frames;
    }
    //copy constructor, to make a deep copy bc we dynamically allocate mem for frame data
    Animation(Animation& a){

      //allocate a new array and copy over ptrs
      frames = new const unsigned char *[a.numberOfFrames];
      for(uint16_t i = 0; i<a.numberOfFrames; i++){
        frames[i] = a.frames[i];
      }

      //copy over members
      currentFrame = a.currentFrame;
      numberOfFrames = a.numberOfFrames;
      msPerFrame = a.msPerFrame;
      timeLastFramePlayed = a.timeLastFramePlayed;
      xCoord = a.xCoord;
      yCoord = a.yCoord;
      width = a.width;
      height = a.height;
      hasPlayedAtLeastOnce = a.hasPlayedAtLeastOnce;
    }
    //overloaded assignment operator, w help from: https://www.learncpp.com/cpp-tutorial/overloading-the-assignment-operator/
    Animation& operator= (const Animation&a){

      //check for self assignment!
      if(this == &a)
        return *this;
      
      //delete old frame data
      if(frames)
        delete [] frames;
      frames = nullptr;
      
      //create new frame array and copy in old values
      frames = new const unsigned char *[a.numberOfFrames];
      for(uint16_t i = 0; i<a.numberOfFrames; i++){
        frames[i] = a.frames[i];
      }

      //copy over members
      currentFrame = a.currentFrame;
      numberOfFrames = a.numberOfFrames;
      msPerFrame = a.msPerFrame;
      timeLastFramePlayed = a.timeLastFramePlayed;
      xCoord = a.xCoord;
      yCoord = a.yCoord;
      width = a.width;
      height = a.height;
      hasPlayedAtLeastOnce = a.hasPlayedAtLeastOnce;

      return *this;
    }
};
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
  }
  showCurrentFrame();
}