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

// Initialization sequence for 72 x 40 screen
static const uint8_t tiny4koled_init_72x40 [] PROGMEM = {
	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for bright 72 x 40 screen
static const uint8_t tiny4koled_init_72x40b [] PROGMEM = {
	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for rotated 72 x 40 screen
static const uint8_t tiny4koled_init_72x40r [] PROGMEM = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for bright rotated 72 x 40 screen
static const uint8_t tiny4koled_init_72x40br [] PROGMEM = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for 64 x 48 screen
static const uint8_t tiny4koled_init_64x48 [] PROGMEM = {
	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for bright 64 x 48 screen
static const uint8_t tiny4koled_init_64x48b [] PROGMEM = {
	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for rotated 64 x 48 screen
static const uint8_t tiny4koled_init_64x48r [] PROGMEM = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for bright rotated 64 x 48 screen
static const uint8_t tiny4koled_init_64x48br [] PROGMEM = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x2F,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for 64 x 32 screen
static const uint8_t tiny4koled_init_64x32 [] PROGMEM = {
	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for bright 64 x 32 screen
static const uint8_t tiny4koled_init_64x32b [] PROGMEM = {
	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for rotated 64 x 32 screen
static const uint8_t tiny4koled_init_64x32r [] PROGMEM = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

// Initialization sequence for bright rotated 64 x 32 screen
static const uint8_t tiny4koled_init_64x32br [] PROGMEM = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x1F,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

#endif
