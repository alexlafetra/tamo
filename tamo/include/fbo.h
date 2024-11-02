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
        clear();
    }
    void init(){
        //if there's an old buffer, free its mem
        if(buffer)
            delete [] buffer;
        bufferSize = width*height/8+1;
        buffer = new uint8_t [bufferSize];
    }
    void clear(){
        fill(0x00);
    }
    void fill(uint8_t c){
        for(uint16_t i = 0; i<bufferSize; i++){
            buffer[i] = c;
        }
    }
    void renderWireFrame(WireFrame& d){
        for(uint16_t edge = 0; edge<d.numberOfEdges; edge++){
            // Vertex v1 = d.verts[d.edges[edge][0]];
            // Vertex v2 = d.verts[d.edges[edge][1]];
            // drawLine(v1.x*d.scale+d.xPos,v1.y*d.scale+d.yPos,v2.x*d.scale+d.xPos,v2.y*d.scale+d.yPos,0);
            // drawLine(d.verts[d.edges[edge][0]].x*d.scale+d.xPos,d.verts[d.edges[edge][0]].y*d.scale+d.yPos,d.verts[d.edges[edge][1]].x*d.scale+d.xPos,d.verts[d.edges[edge][1]].y*d.scale+d.yPos,1);
        }
        for(uint16_t v = 0; v<d.numberOfVertices; v++){
            setPixel(d.verts[v].x*d.scale+d.xPos,d.verts[v].y*d.scale+d.yPos,0);
        }
        // delay(1000);
    }
    void setPixel(int16_t x, int16_t y, uint8_t c){
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
    void render(uint8_t x0, uint8_t y0){
        oled.renderFBO(x0,y0,width,3,buffer);
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