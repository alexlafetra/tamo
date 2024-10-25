#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

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
        uint16_t index = (y/8)+x*(width/8);
        if(index > bufferSize)
            return;
        uint8_t shift = (y%8);
        buffer[index] |= (c<<shift);
    }
    //stole this from Adafruit GFX!
    void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color){
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
};