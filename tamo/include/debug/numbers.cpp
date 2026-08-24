
//------------------------------------
/* numbers */
//------------------------------------

#include "FrameBuffer.h"

// 'zero_bmp', 3x8px
const unsigned char  zero_bmp []   = {
	0x0e, 0x11, 0x0e
};
// 'two_bmp', 3x8px
const unsigned char  two_bmp []   = {
	0x19, 0x15, 0x16
};
// 'three_bmp', 3x8px
const unsigned char  three_bmp []   = {
	0x11, 0x15, 0x0a
};
// 'six_bmp', 3x8px
const unsigned char  six_bmp []   = {
	0x1e, 0x15, 0x19
};
// 'seven_bmp', 3x8px
const unsigned char  seven_bmp []   = {
	0x19, 0x05, 0x03
};
// 'one_bmp', 3x8px
const unsigned char  one_bmp []   = {
	0x12, 0x1f, 0x10
};
// 'nine_bmp', 3x8px
const unsigned char  nine_bmp []   = {
	0x13, 0x15, 0x0f
};
// 'four_bmp', 3x8px
const unsigned char  four_bmp []   = {
	0x03, 0x04, 0x1e
};
// 'five_bmp', 3x8px
const unsigned char  five_bmp []   = {
	0x13, 0x15, 0x0d
};
// 'eight_bmp', 3x8px
const unsigned char  eight_bmp []   = {
	0x1b, 0x15, 0x1b
};

const uint8_t* number_bitmaps[10] = {
    zero_bmp,
    one_bmp,
    two_bmp,
    three_bmp,
    four_bmp,
    five_bmp,
    six_bmp,
    seven_bmp,
    eight_bmp,
    nine_bmp
};

void drawNumber(uint8_t val){
    uint8_t digits = val/10+1;
    uint8_t i = 0;
    do{
        uint8_t lastDigit = val%10;
        fbo.bitmap(16-(i*4),6,3,8,number_bitmaps[lastDigit]);
        val/=10;
        i++;
    }while(val);
}

// void printNumberString(String s){
//     const unsigned char* bitmap = nullptr;
//     for(uint8_t character = 0; character<s.length(); character++){
//         char c = s.charAt(character);
//         switch(c){
//             case '0':
//                 bitmap = zero_bmp;
//                 break;
//             case '1':
//                 bitmap = one_bmp;
//                 break;
//             case '2':
//                 bitmap = two_bmp;
//                 break;
//             case '3':
//                 bitmap = three_bmp;
//                 break;
//             case '4':
//                 bitmap = four_bmp;
//                 break;
//             case '5':
//                 bitmap = five_bmp;
//                 break;
//             case '6':
//                 bitmap = six_bmp;
//                 break;
//             case '7':
//                 bitmap = seven_bmp;
//                 break;
//             case '8':
//                 bitmap = eight_bmp;
//                 break;
//             case '9':
//                 bitmap = nine_bmp;
//                 break;
//         }
//         oled.bitmap2x(16+character*8,0,16+character*8+3,0,bitmap);
//     }
// }