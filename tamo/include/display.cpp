#define TINY4KOLED_NO_PRINT
#define TINY4KOLED_H

/*
 * Tiny4kOLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x32 displays
 *
 * Based on ssd1306xled, re-written and extended by Stephen Denne
 * from 2017-04-25 at https://github.com/datacute/Tiny4kOLED
 *
 * This file adds support for the I2C implementation from https://github.com/adafruit/TinyWireM
 * Adafruit's version has a buffer overrun bugfix.
 * The bug resulted in the appearance of extra dots onscreen.
 */
#ifndef TINY4KOLED_TINYWIREM_H
#define TINY4KOLED_TINYWIREM_H
#endif

// #include <TinyWireM.h>
/*
 * Tiny4kOLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x32 displays
 *
 * Based on ssd1306xled, re-written and extended by Stephen Denne
 * from 2017-04-25 at https://github.com/datacute/Tiny4kOLED
 *
 */
#include <stdint.h>
#include <Arduino.h>

#ifndef TINY4KOLEDCOMMON_H
#define TINY4KOLEDCOMMON_H

// ----------------------------------------------------------------------------

#ifndef SSD1306
#define SSD1306		0x3C	// Slave address
#endif

#define SSD1306_VOLTAGE_6_0 0x15
#define SSD1306_VOLTAGE_7_5 0x14
#define SSD1306_VOLTAGE_8_5 0x94
#define SSD1306_VOLTAGE_9_0 0x95


// ----------------------------------------------------------------------------

class SSD1306Device {

	public:
		SSD1306Device(void (*wireBeginFunc)(void), bool (*wireBeginTransmissionFunc)(void), bool (*wireWriteFunc)(uint8_t byte), uint8_t (*wireEndTransmissionFunc)(void));
		void begin(uint8_t init_sequence_length, const uint8_t init_sequence []);
		void begin(uint8_t width, uint8_t height, uint8_t init_sequence_length, const uint8_t init_sequence []);
		void begin(uint8_t xOffset, uint8_t yOffset, uint8_t width, uint8_t height, uint8_t init_sequence_length, const uint8_t init_sequence []);
		void switchRenderFrame(void);
		void switchDisplayFrame(void);
		void switchFrame(void);
		uint8_t currentRenderFrame(void);
		uint8_t currentDisplayFrame(void);
		void setCombineFunction(uint8_t (*combineFunc)(uint8_t, uint8_t, uint8_t));
		void setCursor(uint8_t x, uint8_t y);
		void fill(uint8_t fill);
		void fillToEOL(uint8_t fill);
		void fillToEOP(uint8_t fill);
		void fillLength(uint8_t fill, uint8_t length);
		void clear(void);
		void clearToEOL(void);
		void clearToEOP(void);
		void bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
		void bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t bitmap[]);
		void renderFBO(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,  uint8_t bitmap[]);
		void renderFBO2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,  uint8_t bitmap[]);
		void bitmap2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
		void bitmap_from_spritesheet2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t offset);
		void bitmap_from_spritesheet(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t offset);
		void overlay_bitmap_from_spritesheet2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t bitmapA_offset, uint8_t offsetX, uint8_t offsetY, uint8_t widthB, uint8_t heightB, uint16_t bitmapB_offset);
		void overlayBitmap2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmapA[], uint8_t offsetX, uint8_t offsetY, uint8_t widthB, uint8_t heightB, const uint8_t bitmapB[]);
		void startData(void);
		void sendData(const uint8_t data);
		void repeatData(uint8_t data, uint8_t length);
		void clearData(uint8_t length);
		void endData(void);
		void setPages(uint8_t pages);
		void setWidth(uint8_t width);
		void setHeight(uint8_t height);
		void setOffset(uint8_t xOffset, uint8_t yOffset);
		void setRotation(uint8_t rotation);
		void invertOutput(bool enable);

		// 1. Fundamental Command Table

		void setContrast(uint8_t contrast);
		void setEntireDisplayOn(bool enable);
		void setInverse(bool enable);
		void setExternalIref(void);
		void setInternalIref(bool bright);
		void off(void);
		void on(void);

		// 2. Scrolling Command Table

		void scrollRight(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t startColumn = 0x00, uint8_t endColumn = 0xFF);
		void scrollLeft(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t startColumn = 0x00, uint8_t endColumn = 0xFF);
		void scrollRightOffset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset);
		void scrollLeftOffset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset);
		void scrollContentRight(uint8_t startPage, uint8_t endPage, uint8_t startColumn, uint8_t endColumn);
		void scrollContentLeft(uint8_t startPage, uint8_t endPage, uint8_t startColumn, uint8_t endColumn);
		void deactivateScroll(void);
		void activateScroll(void);
		void setVerticalScrollArea(uint8_t top, uint8_t rows);

		// 3. Addressing Setting Command Table
		void setColumnStartAddress(uint8_t startAddress);
		void setMemoryAddressingMode(uint8_t mode);
		void setColumnAddress(uint8_t startAddress, uint8_t endAddress);
		void setPageAddress(uint8_t startPage, uint8_t endPage);
		void setPageStartAddress(uint8_t startPage);

		// 4. Hardware Configuration (Panel resolution and layout related) Command Table

		void setDisplayStartLine(uint8_t startLine);
		void setSegmentRemap(uint8_t remap);
		void setMultiplexRatio(uint8_t mux);
		void setComOutputDirection(uint8_t direction);
		void setDisplayOffset(uint8_t offset);
		void setComPinsHardwareConfiguration(uint8_t alternative, uint8_t enableLeftRightRemap);

		// 5. Timing and Driving Scheme Setting Command table

		void setDisplayClock(uint8_t divideRatio, uint8_t oscillatorFrequency);
		void setPrechargePeriod(uint8_t phaseOnePeriod, uint8_t phaseTwoPeriod);
		void setVcomhDeselectLevel(uint8_t level);
		void nop(void);

		// 6. Advance Graphic Command table

		void fadeOut(uint8_t interval);
		void blink(uint8_t interval);
		void disableFadeOutAndBlinking(void);
		void enableZoomIn(void);
		void disableZoomIn(void);

		// Charge Pump Settings

		void enableChargePump(uint8_t voltage = SSD1306_VOLTAGE_7_5);
		void disableChargePump(void);

		// Custom

		/*
		Clears area to the right and left of main sprite
		so you don't need to do a full screen clear when switching
		between different sprites
		*/

		void clearEdges(uint8_t L, uint8_t R){
			setCursor(0,0);
			fillLength(0,L);
			setCursor(0,1);
			fillLength(0,L);

			//right side
			setCursor(64-R,0);
			fillLength(0,64-R);
			setCursor(64-R,1);
			fillLength(0,64-R);
		}
		void clearEdges(){
			clearEdges(20,20);
		}
};

// ----------------------------------------------------------------------------

static const uint8_t tiny4koled_init_defaults [] PROGMEM = {	// Initialization Sequence
	0xAE,			// Display OFF (sleep mode)
	0x20, 0b00,		// Set Memory Addressing Mode
					// 00=Horizontal Addressing Mode; 01=Vertical Addressing Mode;
					// 10=Page Addressing Mode (RESET); 11=Invalid
	0xB0,			// Set Page Start Address for Page Addressing Mode, 0-7
	0xC0,			// Set COM Output Scan Direction
	0x00,			// Set low nibble of column address
	0x10,			// Set high nibble of column address
	0x40,			// Set display start line address
	0x81, 0x7F,		// Set contrast control register
	0xA0,			// Set Segment Re-map. A0=column 0 mapped to SEG0; A1=column 127 mapped to SEG0.
	0xA6,			// Set display mode. A6=Normal; A7=Inverse
	0xA8, 0x3F,		// Set multiplex ratio(1 to 64)
	0xA4,			// Output RAM to Display
					// 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
	0xD3, 0x00,		// Set display offset. 00 = no offset
	0xD5, 0x80,		// --set display clock divide ratio/oscillator frequency
	0xD9, 0x22,		// Set pre-charge period
	0xDA, 0x12,		// Set com pins hardware configuration
	0xDB, 0x20,		// --set vcomh 0x20 = 0.77xVcc
	0xAD, 0x00,		// Select external IREF. 0x10 or 0x30 for Internal current reference at 19uA or 30uA
	0x8D, 0x10		// Set DC-DC disabled
};

// Naming configuration for initialisation sequences:
// tiny4koled_init_{x}x{y}{b}{r}
// x = width in pixels
// y = height in pixels
// b = bright - turns on internal current reference set to the high current setting
// r = rotated - rotates the display 180 degrees.

// the four combinations of bright and rotated are supported for each of the following resolutions:
// 128 x 64
// 128 x 32
//  72 x 40 (These typically require the use of the internal current refference)
//  64 x 48
//  64 x 32

// // Initialization sequence for 72 x 40 screen
// static const uint8_t tiny4koled_init_72x40 [] PROGMEM = {
// 	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for bright 72 x 40 screen
// static const uint8_t tiny4koled_init_72x40b [] PROGMEM = {
// 	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
// 	0xAD, 0x30,   // Select internal IREF and higher current
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for rotated 72 x 40 screen
// static const uint8_t tiny4koled_init_72x40r [] PROGMEM = {
// 	0xC8,         // Set COM Output Scan Direction
// 	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
// 	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// Initialization sequence for bright rotated 72 x 40 screen
static const uint8_t tiny4koled_init_72x40br [] PROGMEM = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// // Initialization sequence for 64 x 48 screen
// static const uint8_t tiny4koled_init_64x48 [] PROGMEM = {
// 	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for bright 64 x 48 screen
// static const uint8_t tiny4koled_init_64x48b [] PROGMEM = {
// 	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
// 	0xAD, 0x30,   // Select internal IREF and higher current
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for rotated 64 x 48 screen
// static const uint8_t tiny4koled_init_64x48r [] PROGMEM = {
// 	0xC8,         // Set COM Output Scan Direction
// 	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
// 	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for bright rotated 64 x 48 screen
// static const uint8_t tiny4koled_init_64x48br [] PROGMEM = {
// 	0xC8,         // Set COM Output Scan Direction
// 	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
// 	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
// 	0xAD, 0x30,   // Select internal IREF and higher current
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for 64 x 32 screen
// static const uint8_t tiny4koled_init_64x32 [] PROGMEM = {
// 	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for bright 64 x 32 screen
// static const uint8_t tiny4koled_init_64x32b [] PROGMEM = {
// 	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
// 	0xAD, 0x30,   // Select internal IREF and higher current
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for rotated 64 x 32 screen
// static const uint8_t tiny4koled_init_64x32r [] PROGMEM = {
// 	0xC8,         // Set COM Output Scan Direction
// 	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
// 	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

// // Initialization sequence for bright rotated 64 x 32 screen
// static const uint8_t tiny4koled_init_64x32br [] PROGMEM = {
// 	0xC8,         // Set COM Output Scan Direction
// 	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
// 	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
// 	0xAD, 0x30,   // Select internal IREF and higher current
// 	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
// };

#endif

#ifndef DATACUTE_I2C_TINYWIREM
#define DATACUTE_I2C_TINYWIREM

static bool datacute_write_tinywirem(uint8_t byte) {
	return TinyWireM.write(byte);
}

static uint8_t datacute_read_tinywirem(void) __attribute__((unused));
static uint8_t datacute_read_tinywirem(void) {
	return TinyWireM.read();
}

static void datacute_end_read_tinywirem(void) __attribute__((unused));
static void datacute_end_read_tinywirem(void) {}

static uint8_t datacute_endTransmission_tinywirem(void) {
	return TinyWireM.endTransmission();
}

#endif

static bool tiny4koled_beginTransmission_tinywirem(void) {
	TinyWireM.beginTransmission(SSD1306);
	return true;
}

#ifndef TINY4KOLED_QUICK_BEGIN
static bool tiny4koled_check_tinywirem(void) {
	const uint8_t noError = 0x00;
	tiny4koled_beginTransmission_tinywirem();
	return (datacute_endTransmission_tinywirem()==noError);
}
#endif

static void tiny4koled_begin_tinywirem(void) {
	TinyWireM.begin();
#ifndef TINY4KOLED_QUICK_BEGIN
	while (!tiny4koled_check_tinywirem()) {
		// delay(10);
	}
#endif
}

SSD1306Device oled(&tiny4koled_begin_tinywirem, &tiny4koled_beginTransmission_tinywirem, &datacute_write_tinywirem, &datacute_endTransmission_tinywirem);

/*
 * Tiny4kOLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x32 displays
 *
 * Based on ssd1306xled, re-written and extended by Stephen Denne
 * from 2017-04-25 at https://github.com/datacute/Tiny4kOLED
 *
 */

// ----------------------------------------------------------------------------

// #include "Tiny4kOLED_common.h"

#define SSD1306_COLUMNS 128
#define SSD1306_PAGES 4

#define SSD1306_COMMAND 0x00
#define SSD1306_DATA 0x40

// ----------------------------------------------------------------------------

static uint8_t oledOffsetX = 0, oledOffsetY = 0; // pixels and pages
static uint8_t oledWidth = SSD1306_COLUMNS; // pixels and pages
static uint8_t oledPages = SSD1306_PAGES;

static uint8_t oledX = 0, oledY = 0;
static uint8_t renderingFrame = 0xB0, drawingFrame = 0x40;

static void (*wireBeginFn)(void);
static bool (*wireBeginTransmissionFn)(void);
static bool (*wireWriteFn)(uint8_t byte);
static uint8_t (*wireEndTransmissionFn)(void);

// static uint8_t (*combineFn)(uint8_t x, uint8_t y, uint8_t b) = 0;
static uint8_t writesSinceSetCursor = 0;

static void ssd1306_begin(void) {
	wireBeginFn();
}

static void ssd1306_send_start(void) {
	wireBeginTransmissionFn();
}

static bool ssd1306_send_byte(uint8_t byte) {
	return wireWriteFn(byte);
}

static void ssd1306_send_stop(void) {
	wireEndTransmissionFn();
}

static void ssd1306_send_command_start(void) {
	ssd1306_send_start();
	ssd1306_send_byte(SSD1306_COMMAND);
}

static void ssd1306_send_data_start(void) {
	ssd1306_send_start();
	ssd1306_send_byte(SSD1306_DATA);
}

static void ssd1306_send_command_byte(uint8_t byte) {
	if (ssd1306_send_byte(byte) == 0) {
		ssd1306_send_stop();
		ssd1306_send_command_start();
		ssd1306_send_byte(byte);
	}
}

static void ssd1306_send_data_byte(uint8_t byte) {
	// if (combineFn) byte = (*combineFn)(oledX + writesSinceSetCursor, oledY, byte);
	if (ssd1306_send_byte(byte) == 0) {
		ssd1306_send_stop();
		ssd1306_send_data_start();
		ssd1306_send_byte(byte);
	}
	writesSinceSetCursor++;
}

static void ssd1306_send_command(uint8_t command) {
	ssd1306_send_command_start();
	ssd1306_send_byte(command);
	ssd1306_send_stop();
}

static void ssd1306_send_command2(uint8_t command1, uint8_t command2) {
	ssd1306_send_command_start();
	ssd1306_send_byte(command1);
	ssd1306_send_byte(command2);
	ssd1306_send_stop();
}

static void ssd1306_send_command3(uint8_t command1, uint8_t command2, uint8_t command3) {
	ssd1306_send_command_start();
	ssd1306_send_byte(command1);
	ssd1306_send_byte(command2);
	ssd1306_send_byte(command3);
	ssd1306_send_stop();
}

static void ssd1306_send_command6(uint8_t command1, uint8_t command2, uint8_t command3, uint8_t command4, uint8_t command5, uint8_t command6) {
	ssd1306_send_command_start();
	ssd1306_send_byte(command1);
	ssd1306_send_byte(command2);
	ssd1306_send_byte(command3);
	ssd1306_send_byte(command4);
	ssd1306_send_byte(command5);
	ssd1306_send_byte(command6);
	ssd1306_send_stop();
}

static void ssd1306_send_command7(uint8_t command1, uint8_t command2, uint8_t command3, uint8_t command4, uint8_t command5, uint8_t command6, uint8_t command7) {
	ssd1306_send_command_start();
	ssd1306_send_byte(command1);
	ssd1306_send_byte(command2);
	ssd1306_send_byte(command3);
	ssd1306_send_byte(command4);
	ssd1306_send_byte(command5);
	ssd1306_send_byte(command6);
	ssd1306_send_byte(command7);
	ssd1306_send_stop();
}

// static uint8_t invertByte(uint8_t x, uint8_t y, uint8_t byte) {
// 	return byte ^ 0xff;
// }

SSD1306Device::SSD1306Device(void (*wireBeginFunc)(void), bool (*wireBeginTransmissionFunc)(void), bool (*wireWriteFunc)(uint8_t byte), uint8_t (*wireEndTransmissionFunc)(void)) {
	wireBeginFn = wireBeginFunc;
	wireBeginTransmissionFn = wireBeginTransmissionFunc;
	wireWriteFn = wireWriteFunc;
	wireEndTransmissionFn = wireEndTransmissionFunc;
}

void SSD1306Device::begin(uint8_t init_sequence_length, const uint8_t init_sequence []) {
	ssd1306_begin();

	ssd1306_send_command_start();
	for (uint8_t i = 0; i < init_sequence_length; i++) {
		ssd1306_send_command_byte(pgm_read_byte(&init_sequence[i]));
	}
	ssd1306_send_stop();
}

void SSD1306Device::begin(uint8_t width, uint8_t height, uint8_t init_sequence_length, const uint8_t init_sequence []) {
	oledOffsetX = (128 - width) >> 1;
	oledOffsetY = 0;
	oledWidth = width;
	oledPages = height >> 3;
	begin(init_sequence_length,init_sequence);
}

void SSD1306Device::begin(uint8_t xOffset, uint8_t yOffset, uint8_t width, uint8_t height, uint8_t init_sequence_length, const uint8_t init_sequence []) {
	oledOffsetX = xOffset;
	oledOffsetY = yOffset >> 3;
	oledWidth = width;
	oledPages = height >> 3;
	begin(init_sequence_length,init_sequence);
}

void SSD1306Device::setPages(uint8_t pages) {
	oledPages = pages;
}

void SSD1306Device::setWidth(uint8_t width) {
	oledWidth = width;
}

void SSD1306Device::setHeight(uint8_t height) {
	oledPages = height >> 3;
}

void SSD1306Device::setOffset(uint8_t xOffset, uint8_t yOffset) {
	oledOffsetX = xOffset;
	oledOffsetY = yOffset >> 3;
}

void SSD1306Device::setRotation(uint8_t rotation) {
	uint8_t rotationBit = (rotation & 0x01);
	ssd1306_send_command2(0xC0 | (rotationBit << 3), 0xA0 | rotationBit);
}

void SSD1306Device::setCombineFunction(uint8_t (*combineFunc)(uint8_t, uint8_t, uint8_t)) {
	// combineFn = combineFunc;
}

void SSD1306Device::setCursor(uint8_t x, uint8_t y) {
	// renderingFrame = 0;
	//ok, trying to translate
	/*
										adding the rendering frame offset and the last 3 bits of (y+oledOffsetY), adding a 1 at the fifth place and then taking the first 4 bits of x+oledOffsetX, taking just the last four bits
	*/
	ssd1306_send_command3(renderingFrame | ((y + oledOffsetY) & 0x07), 0x10 | (((x + oledOffsetX) & 0xf0) >> 4), (x + oledOffsetX) & 0x0f);
	oledX = x;
	oledY = y;
	writesSinceSetCursor = 0;
}
void SSD1306Device::clear(void) {
	fill(0x00);
}

void SSD1306Device::fill(uint8_t fill) {
	for (uint8_t m = 0; m < oledPages; m++) {
		setCursor(0, m);
		fillToEOP(fill);
	}
	setCursor(0, 0);
}
void SSD1306Device::bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]) {
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
void SSD1306Device::bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,  uint8_t bitmap[]) {
	uint16_t j = 0;
 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t x = x0; x < x1; x++) {
			ssd1306_send_data_byte(bitmap[j++]);
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}
void SSD1306Device::renderFBO(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h,  uint8_t* bitmap) {
	uint16_t j = 0;
 	for (uint8_t y = 0; y <= h; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t x = 0; x < w; x++) {
			ssd1306_send_data_byte(*(bitmap+j++));
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}
void SSD1306Device::renderFBO2x(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h,  uint8_t* bitmap) {
	uint16_t j = 0;
 	for (uint8_t y = 0; y <= h; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t x = 0; x < w; x++) {
			ssd1306_send_data_byte(*(bitmap+j));
			ssd1306_send_data_byte(*(bitmap+j++));
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}

//this one i wrote...it's p janky
void SSD1306Device::bitmap2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]) {
	uint16_t j = 0;
 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < (x1-x0); i++) {
			ssd1306_send_data_byte(pgm_read_byte(&bitmap[j]));
			//sending it twice bc for some reason it's 1/2 width when fullsize is enabled!
			ssd1306_send_data_byte(pgm_read_byte(&bitmap[j]));
			j++;
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}

void SSD1306Device::bitmap_from_spritesheet2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t offset) {
	uint16_t j = 0;
 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < (x1-x0); i++) {
			ssd1306_send_data_byte(pgm_read_byte(&spritesheet[j+offset]));
			ssd1306_send_data_byte(pgm_read_byte(&spritesheet[j+offset]));
			j++;
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}

void SSD1306Device::bitmap_from_spritesheet(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t offset) {
	uint16_t j = 0;
 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < (x1-x0); i++) {
			ssd1306_send_data_byte(pgm_read_byte(&spritesheet[j+offset]));
			j++;
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}

void SSD1306Device::overlay_bitmap_from_spritesheet2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t bitmapA_offset, uint8_t offsetX, uint8_t offsetY, uint8_t widthB, uint8_t heightB, uint16_t bitmapB_offset){

	uint8_t j = 0;
	uint8_t jB = 0;

 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < (x1-x0); i++) {
			uint8_t byteA = pgm_read_byte(&spritesheet[j+bitmapA_offset]);
			if(i>=offsetX && i<(offsetX+widthB) && y>=(offsetY/8) && y<((heightB+offsetY)/8)){
				uint8_t byteB = pgm_read_byte(&spritesheet[jB+bitmapB_offset]);
				jB++;
				byteB = byteB<<offsetY;//shift the sprite down
				byteA |= byteB;
			} 
			//sending it twice bc for some reason it's 1/2 width when fullsize is enabled!
			ssd1306_send_data_byte(byteA);
			ssd1306_send_data_byte(byteA);
			j++;
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}

void SSD1306Device::overlayBitmap2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmapA[], uint8_t offsetX, uint8_t offsetY, uint8_t widthB, uint8_t heightB, const uint8_t bitmapB[]){
	// int8_t width = x1-x0;
	// if(width<=0)
	// 	return;
	uint8_t j = 0;
	uint8_t jB = 0;

 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < (x1-x0); i++) {
			uint8_t byteA = pgm_read_byte(&bitmapA[j]);
			if(i>=offsetX && i<(offsetX+widthB) && y>=(offsetY/8) && y<((heightB+offsetY)/8)){
				uint8_t byteB = pgm_read_byte(&bitmapB[jB]);
				jB++;
				byteB = byteB<<offsetY;//shift the sprite down
				byteA |= byteB;
			} 
			//sending it twice bc for some reason it's 1/2 width when fullsize is enabled!
			ssd1306_send_data_byte(byteA);
			ssd1306_send_data_byte(byteA);
			j++;
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}


void SSD1306Device::clearToEOP(void) {
	fillToEOP(0x00);
}

void SSD1306Device::fillToEOP(uint8_t fill) {
	fillLength(fill, oledWidth - oledX);
}

void SSD1306Device::fillLength(uint8_t fill, uint8_t length) {
	if (length == 0) return;
	oledX += length;
	ssd1306_send_data_start();
	repeatData(fill, length);
	ssd1306_send_stop();
}

void SSD1306Device::startData(void) {
	ssd1306_send_data_start();
}

void SSD1306Device::sendData(const uint8_t data) {
	ssd1306_send_data_byte(data);
}

void SSD1306Device::repeatData(uint8_t data, uint8_t length) {
	for (uint8_t x = 0; x < length; x++) {
		ssd1306_send_data_byte(data);
	}
}

void SSD1306Device::clearData(uint8_t length) {
	repeatData(0, length);
}

void SSD1306Device::endData(void) {
	ssd1306_send_stop();
}

void SSD1306Device::invertOutput(bool enable) {
	// combineFn = enable ? &invertByte : NULL;
}	

void SSD1306Device::switchRenderFrame(void) {
	renderingFrame ^= 0x04;
}

void SSD1306Device::switchDisplayFrame(void) {
	drawingFrame ^= 0x20;
	ssd1306_send_command(drawingFrame);
}

void SSD1306Device::switchFrame(void) {
	switchDisplayFrame();
	switchRenderFrame();
}

uint8_t SSD1306Device::currentRenderFrame(void) {
	return (renderingFrame >> 2) & 0x01;
}

uint8_t SSD1306Device::currentDisplayFrame(void) {
	return (drawingFrame >> 5) & 0x01;
}

// 1. Fundamental Command Table

void SSD1306Device::setContrast(uint8_t contrast) {
	ssd1306_send_command2(0x81,contrast);
}

void SSD1306Device::setEntireDisplayOn(bool enable) {
	if (enable)
		ssd1306_send_command(0xA5);
	else
		ssd1306_send_command(0xA4);
}

void SSD1306Device::setInverse(bool enable) {
	if (enable)
		ssd1306_send_command(0xA7);
	else
		ssd1306_send_command(0xA6);
}

void SSD1306Device::setExternalIref(void) {
	ssd1306_send_command2(0xAD, 0x00);
}

void SSD1306Device::setInternalIref(bool bright) {
	ssd1306_send_command2(0xAD, ((bright & 0x01) << 5) | 0x10);
}

void SSD1306Device::off(void) {
	ssd1306_send_command(0xAE);
}

void SSD1306Device::on(void) {
	ssd1306_send_command(0xAF);
}

// 2. Scrolling Command Table

void SSD1306Device::scrollRight(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t startColumn, uint8_t endColumn) {
	ssd1306_send_command7(0x26, 0x00, startPage + oledOffsetY, interval, endPage + oledOffsetY, startColumn, endColumn);
}

void SSD1306Device::scrollLeft(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t startColumn, uint8_t endColumn) {
	ssd1306_send_command7(0x27, 0x00, startPage + oledOffsetY, interval, endPage + oledOffsetY, startColumn, endColumn);
}

void SSD1306Device::scrollRightOffset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset) {
	ssd1306_send_command6(0x29, 0x00, startPage + oledOffsetY, interval, endPage + oledOffsetY, offset);
}

void SSD1306Device::scrollLeftOffset(uint8_t startPage, uint8_t interval, uint8_t endPage, uint8_t offset) {
	ssd1306_send_command6(0x2A, 0x00, startPage + oledOffsetY, interval, endPage + oledOffsetY, offset);
}

void SSD1306Device::scrollContentRight(uint8_t startPage, uint8_t endPage, uint8_t startColumn, uint8_t endColumn) {
	ssd1306_send_command7(0x2C, 0x00, startPage + oledOffsetY, 0x01, endPage + oledOffsetY, startColumn + oledOffsetX, endColumn + oledOffsetX);
}

void SSD1306Device::scrollContentLeft(uint8_t startPage, uint8_t endPage, uint8_t startColumn, uint8_t endColumn) {
	ssd1306_send_command7(0x2D, 0x00, startPage + oledOffsetY, 0x01, endPage + oledOffsetY, startColumn + oledOffsetX, endColumn + oledOffsetX);
}

void SSD1306Device::deactivateScroll(void) {
	ssd1306_send_command(0x2E);
}

void SSD1306Device::activateScroll(void) {
	ssd1306_send_command(0x2F);
}

void SSD1306Device::setVerticalScrollArea(uint8_t top, uint8_t rows) {
	ssd1306_send_command3(0xA3, top, rows);
}

// 3. Addressing Setting Command Table

void SSD1306Device::setColumnStartAddress(uint8_t startAddress) {
	ssd1306_send_command2(startAddress & 0x0F, startAddress >> 4);
}

void SSD1306Device::setMemoryAddressingMode(uint8_t mode) {
	ssd1306_send_command2(0x20, mode & 0x03);
}

void SSD1306Device::setColumnAddress(uint8_t startAddress, uint8_t endAddress) {
	ssd1306_send_command3(0x21, startAddress & 0x7F, endAddress & 0x7F);
}

void SSD1306Device::setPageAddress(uint8_t startPage, uint8_t endPage) {
	ssd1306_send_command3(0x22, startPage & 0x07, endPage & 0x07);
}

void SSD1306Device::setPageStartAddress(uint8_t startPage) {
	ssd1306_send_command(0xB0 | (startPage & 0x07));
}

// 4. Hardware Configuration (Panel resolution and layout related) Command Table

void SSD1306Device::setDisplayStartLine(uint8_t startLine) {
	ssd1306_send_command(0x40 | (startLine & 0x3F));
}

void SSD1306Device::setSegmentRemap(uint8_t remap) {
	ssd1306_send_command(0xA0 | (remap & 0x01));
}

void SSD1306Device::setMultiplexRatio(uint8_t mux) {
	ssd1306_send_command2(0xA8, (mux - 1) & 0x3F);
}

void SSD1306Device::setComOutputDirection(uint8_t direction) {
	ssd1306_send_command(0xC0 | ((direction & 0x01) << 3));
}

void SSD1306Device::setDisplayOffset(uint8_t offset) {
	ssd1306_send_command2(0xD3, offset & 0x3F);
}

void SSD1306Device::setComPinsHardwareConfiguration(uint8_t alternative, uint8_t enableLeftRightRemap) {
	ssd1306_send_command2(0xDA, ((enableLeftRightRemap & 0x01) << 5) | ((alternative & 0x01) << 4) | 0x02 );
}

// 5. Timing and Driving Scheme Setting Command table

void SSD1306Device::setDisplayClock(uint8_t divideRatio, uint8_t oscillatorFrequency) {
	ssd1306_send_command2(0xD5, ((oscillatorFrequency & 0x0F) << 4) | ((divideRatio -1) & 0x0F));
}

void SSD1306Device::setPrechargePeriod(uint8_t phaseOnePeriod, uint8_t phaseTwoPeriod) {
	ssd1306_send_command2(0xD9, ((phaseTwoPeriod & 0x0F) << 4) | (phaseOnePeriod & 0x0F));
}

void SSD1306Device::setVcomhDeselectLevel(uint8_t level) {
	ssd1306_send_command2(0xDB, (level & 0x07) << 4);
}

void SSD1306Device::nop(void) {
	ssd1306_send_command(0xE3);
}

// 6. Advance Graphic Command table

void SSD1306Device::fadeOut(uint8_t interval) {
	ssd1306_send_command2(0x23, (0x20 | (interval & 0x0F)));
}

void SSD1306Device::blink(uint8_t interval) {
	ssd1306_send_command2(0x23, (0x30 | (interval & 0x0F)));
}

void SSD1306Device::disableFadeOutAndBlinking(void) {
	ssd1306_send_command2(0x23, 0x00);
}

void SSD1306Device::enableZoomIn(void) {
	ssd1306_send_command2(0xD6, 0x01);
}

void SSD1306Device::disableZoomIn(void) {
	ssd1306_send_command2(0xD6, 0x00);
}

// Charge Pump Settings

void SSD1306Device::enableChargePump(uint8_t voltage) {
	ssd1306_send_command2(0x8D, ((voltage | 0x14) & 0xD5));
}

void SSD1306Device::disableChargePump(void) {
	ssd1306_send_command2(0x8D, 0x10);
}

// ----------------------------------------------------------------------------
