
//------------------------------------
/* numbers */
//------------------------------------

//zero_bmp: 3x8
const unsigned char zero_bmp[3] PROGMEM = {
	0xe, 0x11, 0xe
};
//six_bmp: 3x8
const unsigned char six_bmp[3] PROGMEM = {
	0x1e, 0x15, 0x19
};
//two_bmp: 3x8
const unsigned char two_bmp[3] PROGMEM = {
	0x19, 0x15, 0x16
};
//eight_bmp: 3x8
const unsigned char eight_bmp[3] PROGMEM = {
	0x1b, 0x15, 0x1b
};
//four_bmp: 3x8
const unsigned char four_bmp[3] PROGMEM = {
	0x3, 0x4, 0x1e
};
//five_bmp: 3x8
const unsigned char five_bmp[3] PROGMEM = {
	0x13, 0x15, 0xd
};
//three_bmp: 3x8
const unsigned char three_bmp[3] PROGMEM = {
	0x11, 0x15, 0xa
};
//one_bmp: 3x8
const unsigned char one_bmp[3] PROGMEM = {
	0x12, 0x1f, 0x10
};
//seven_bmp: 3x8
const unsigned char seven_bmp[3] PROGMEM = {
	0x19, 0x5, 0x3
};
//nine_bmp: 3x8
const unsigned char nine_bmp[3] PROGMEM = {
	0x13, 0x15, 0xf
};


void printNumberString(String s){
    const unsigned char* bitmap = nullptr;
    for(uint8_t character = 0; character<s.length(); character++){
        char c = s.charAt(character);
        switch(c){
            case '0':
                bitmap = zero_bmp;
                break;
            case '1':
                bitmap = one_bmp;
                break;
            case '2':
                bitmap = two_bmp;
                break;
            case '3':
                bitmap = three_bmp;
                break;
            case '4':
                bitmap = four_bmp;
                break;
            case '5':
                bitmap = five_bmp;
                break;
            case '6':
                bitmap = six_bmp;
                break;
            case '7':
                bitmap = seven_bmp;
                break;
            case '8':
                bitmap = eight_bmp;
                break;
            case '9':
                bitmap = nine_bmp;
                break;
        }
        oled.bitmap2x(16+character*8,0,16+character*8+3,0,bitmap);
    }
}