// #include "wireframe/WireFrame.h"
#include "FrameBuffer.h"
#include <stdint.h>
#include "utils.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

FrameBuffer::FrameBuffer(){}
FrameBuffer::FrameBuffer(uint8_t w, uint8_t h){
    width = w;
    height = h;

    //if there's an old buffer, free its mem
    if(buffer)
        delete [] buffer;

    bufferSize = width*height/8;
    buffer = new uint8_t [bufferSize];

    fill(0x00);
}
FrameBuffer::~FrameBuffer(){
    delete [] buffer;
}
//copy constructor, to make a deep copy bc we dynamically allocate mem for frame data
FrameBuffer::FrameBuffer(FrameBuffer& f){
    width = f.width;
    height = f.height;
    //if there's an old buffer, free its mem
    if(buffer)
        delete [] buffer;
    bufferSize = f.bufferSize;
    buffer = new uint8_t [bufferSize];
    for(uint8_t i = 0; i<bufferSize; i++){
        buffer[i] = f.buffer[i];
    }
}
uint8_t FrameBuffer::getPixel(uint8_t x, uint8_t y){
    if((x < 0) || (y < 0) || (x > width) || (y > height))
        return;
    uint16_t index = width*(y/8)+x;

    if(index >= bufferSize)
        return;
    uint8_t shift = (y%8);

    return (buffer[index]>>shift)&1;
}
void FrameBuffer::fill(uint8_t c){
    for(uint8_t i = 0; i<bufferSize; i++){
        buffer[i] = c;
    }
}
void FrameBuffer::clear(){
    fill(0);
}
// void FrameBuffer::renderWireFrame(WireFrame& d, uint8_t c){
//     for(uint16_t edge = 0; edge<d.numberOfEdges; edge++){
//         drawLine(d.verts[d.edges[edge][0]].x*d.scale+d.xPos,d.verts[d.edges[edge][0]].y*d.scale+d.yPos,d.verts[d.edges[edge][1]].x*d.scale+d.xPos,d.verts[d.edges[edge][1]].y*d.scale+d.yPos,c);
//     }
// }
void FrameBuffer::setPixel(int8_t x, int8_t y, uint8_t c){
    if((x < 0) || (y < 0) || (x > width) || (y > height))
        return;
    uint16_t index = width*(y/8)+x;

    if(index >= bufferSize)
        return;
    uint8_t shift = (y%8);

    if(c == 1)
        buffer[index] |= uint8_t(1<<shift);
    else if(c == 0)
        buffer[index] &= ~uint8_t(1<<shift);
}
//okay so when sprites are drawn and they horizontally break over the fbo, they drop down onto the next page
void FrameBuffer::bitmap(int8_t x0, int8_t y0, uint8_t w, uint8_t h, const uint8_t * data){
    const uint16_t size = (w*h/8);
    uint8_t byte = 0;
    for(uint16_t i = 0; i<size; i++){
        //find the byte index in the fbo buffer
        uint8_t y = i / w;
        int16_t index = 0;
        index = x0+(i%w) + width*y + ((y0/8)*width);

        uint16_t bmp_vertical_offset = 0;
        if(y0 < 0){
            bmp_vertical_offset = y0*width;
        }

        //catching errors
        if(index < 0)
            continue;

        int8_t x = i % w;
        if((x + x0) >= width)
            continue;

        uint8_t byte = data[i];

        //tricky overlay to handle non-full-byte vertical offsets
        // int8_t y_offset = (y0%8);
        // if(y_offset > 0){
        //     byte = (byte<<y_offset);
        //     if(i > w){
        //         uint8_t aboveByte = data[i - w];
        //         byte |= (aboveByte>>(8-(y_offset)));
        //     }
        // }
        // else if(y_offset < 0){
        //     byte = (byte>>(-y_offset));
        //     if(i < w){
        //         uint8_t belowByte = data[i + w];
        //         byte |= (belowByte<<(8+(y_offset)));
        //     }

        // }

        buffer[index] = buffer[index] | byte;
    }
}

void FrameBuffer::bitmap_from_spritesheet(int8_t x0, int8_t y0, uint8_t w, uint8_t h, uint16_t spritesheet_offset){
    const uint16_t size = (w*h/8);
    uint8_t byte = 0;
    for(uint16_t i = 0; i<size; i++){
        //find the byte index in the fbo buffer
        uint8_t y = i / w;
        int16_t index = 0;
        index = x0+(i%w) + width*y + ((y0/8)*width);

        uint16_t bmp_vertical_offset = 0;
        if(y0 < 0){
            bmp_vertical_offset = y0*width;
        }

        //catching errors
        if(index < 0)
            continue;

        int8_t x = i % w;
        if((x + x0) >= width)
            continue;

        uint8_t byte = get_sprite_data(i + spritesheet_offset);

        //tricky overlay to handle non-full-byte vertical offsets
        int8_t y_offset = (y0%8);
        if(y_offset > 0){
            byte = (byte<<y_offset);
            if(i > w){
                uint8_t aboveByte = get_sprite_data(i - w + spritesheet_offset);
                byte |= (aboveByte>>(8-(y_offset)));
            }
        }
        else if(y_offset < 0){
            byte = (byte>>(-y_offset));
            if(i < w){
                uint8_t belowByte = get_sprite_data(i + w + spritesheet_offset);
                byte |= (belowByte<<(8+(y_offset)));
            }

        }

        buffer[index] = buffer[index] | byte;
    }
}
//stole this from Adafruit GFX!
void FrameBuffer::drawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, int8_t color){
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            setPixel(y0, x0, color);
        } else {
            setPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
        y0 += ystep;
        err += dx;
        }
    }
}
