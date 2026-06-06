#pragma once

#include <stdint.h>

class FrameBuffer{
    public:
    uint16_t width = 32;
    uint16_t height = 32;
    uint8_t* buffer = nullptr;
    uint16_t bufferSize = 0;
    FrameBuffer();
    FrameBuffer(uint8_t w, uint8_t h);
    ~FrameBuffer();
    //copy constructor, to make a deep copy bc we dynamically allocate mem for frame data
    FrameBuffer(FrameBuffer& f);
    //overloaded assignment operator, w help from: https://www.learncpp.com/cpp-tutorial/overloading-the-assignment-operator/
    FrameBuffer& operator= (const FrameBuffer& f){
        //check for self assignment!
        if(this == &f)
            return *this;
            
        width = f.width;
        height = f.height;
        if(buffer)
            delete [] buffer;
        bufferSize = f.bufferSize;
        buffer = new uint8_t [bufferSize];
        for(uint8_t i = 0; i<bufferSize; i++){
            buffer[i] = f.buffer[i];
        }
    }  
    uint8_t getPixel(uint8_t x, uint8_t y);
    void fill(uint8_t c);
    void clear();
    // void renderWireFrame(WireFrame& d, uint8_t c);
    void setPixel(int8_t x, int8_t y, uint8_t c);
    //okay so when sprites are drawn and they horizontally break over the fbo, they drop down onto the next page
    void bitmap(int8_t x0, int8_t y0, uint8_t w, uint8_t h, const uint8_t * data);
    void bitmap_from_spritesheet(int8_t x0, int8_t y0, uint8_t w, uint8_t h, uint16_t spritesheet_offset);
    //stole this from Adafruit GFX!
    void drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, int8_t color);
};