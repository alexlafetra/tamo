class FrameBuffer{
    public:
    uint8_t width = 64;
    uint8_t height = 32;
    uint8_t* buffer = nullptr;
    uint16_t bufferSize = 0;
    FrameBuffer(){}
    FrameBuffer(uint8_t w, uint8_t h){
        width = w;
        height = h;
        init();
    }
    void init(){
        //if there's an old buffer, free its mem
        if(buffer)
            delete [] buffer;
        bufferSize = width*height/8+1;
        buffer = new uint8_t [bufferSize];
    }
    void setPixel(uint8_t x, uint8_t y, uint8_t c){
        uint16_t index = (x*width+y*height)/8;
        if(index > bufferSize)
            return;
        uint8_t shift = (x*width+y*height)%8;
        buffer[index] |= (c<<shift);
    }
};