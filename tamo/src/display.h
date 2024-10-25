class Display{
    public:
    uint8_t x = 0;
    uint8_t y = 0;
    Display(){};
    void setRotation(uint8_t rotation) {
        uint8_t rotationBit = (rotation & 0x01);
        ssd1306_send_command2(0xC0 | (rotationBit << 3), 0xA0 | rotationBit);
    }
    void setCursor(uint8_t x, uint8_t y) {
        //ok, trying to translate
        /*
                                            adding the rendering frame offset and the last 3 bits of (y+oledOffsetY), adding a 1 at the fifth place and then taking the first 4 bits of x+oledOffsetX, taking just the last four bits
        */
        ssd1306_send_command3(renderingFrame | ((y + oledOffsetY) & 0x07), 0x10 | (((x + oledOffsetX) & 0xf0) >> 4), (x + oledOffsetX) & 0x0f);
        oledX = x;
        oledY = y;
        writesSinceSetCursor = 0;
    }
    void fillLength
    void fillToEOP
    void off() {
        ssd1306_send_command(0xAE);
    }
    void on() {
        ssd1306_send_command(0xAF);
    }
    void begin(uint8_t init_sequence_length, const uint8_t init_sequence []) {
        ssd1306_begin();

        ssd1306_send_command_start();
        for (uint8_t i = 0; i < init_sequence_length; i++) {
            ssd1306_send_command_byte(pgm_read_byte(&init_sequence[i]));
        }
        ssd1306_send_stop();
    }
    void enableZoomIn() {
        ssd1306_send_command2(0xD6, 0x01);
    }

    void switchFrame
    void fill(uint8_t c) {
        for (uint8_t m = 0; m < oledPages; m++) {
            setCursor(0, m);
            fillToEOP(c);
        }
        setCursor(0, 0);
    }
    void clear(){
        fill(0);
    }
    void bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]) {
        uint16_t j = 0;
        for (uint8_t y = y0; y <= y1; y++) {
            setCursor(x0,y);
            ssd1306_send_data_start();
            for (uint8_t x = x0; x < x1; x++) {
                ssd1306_send_data_byte(pgm_read_byte(&bitmap[j++]));
            }
            ssd1306_send_stop();
        }
        setCursor(0, 0);
    }
    //this is p janky
    void bitmap2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]) {
        int8_t width = x1-x0;
        if(width<=0)
            return;
        uint16_t j = 0;
        for (uint8_t y = y0; y <= y1; y++) {
            setCursor(x0,y);
            ssd1306_send_data_start();
            for (uint8_t i = 0; i < width; i++) {
                ssd1306_send_data_byte(pgm_read_byte(&bitmap[j]));
                //sending it twice bc for some reason it's 1/2 width!
                ssd1306_send_data_byte(pgm_read_byte(&bitmap[j]));
                j++;
            }
            ssd1306_send_stop();
        }
        setCursor(0, 0);
    }
};

