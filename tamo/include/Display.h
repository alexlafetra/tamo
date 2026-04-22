#pragma once

#define TINY4KOLED_NO_PRINT
#define TINY4KOLED_H

/*
 * Tiny4kOLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x32 displays
 *
 * Based on ssd1306xled, re-written and extended by Stephen Denne
 * from 2017-04-25 at https://github.com/datacute/Tiny4kOLED
 *
 * This file adds support for the I2C implementation from https://github.com/adafruit/Wire
 * Adafruit's version has a buffer overrun bugfix.
 * The bug resulted in the appearance of extra dots onscreen.
 */
#ifndef TINY4KOLED_Wire_H
#define TINY4KOLED_Wire_H
#endif

#include <Wire.h>
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
		SSD1306Device(){}
		void begin();
		void begin(uint8_t width, uint8_t height);
		void switchRenderFrame(void);
		void switchDisplayFrame(void);
		void switchFrame(void);
		uint8_t currentRenderFrame(void);
		uint8_t currentDisplayFrame(void);
		void setCombineFunction(uint8_t (*combineFunc)(uint8_t, uint8_t, uint8_t));
		void setCursor(uint8_t x, uint8_t y);
		void fill(uint8_t fill);
		void fillToEOP(uint8_t fill);
		void fillLength(uint8_t fill, uint8_t length);
		void clear(void);
		void clearToEOP(void);
		void bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
		void bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t bitmap[]);
		void renderFBO(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,  uint8_t bitmap[]);
		void renderFBO2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,  uint8_t bitmap[]);
		void bitmap2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
		void bitmap_from_spritesheet2x(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t offset);
		void bitmap_from_spritesheet(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t offset);
		void bitmap_from_spritesheet(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t offset, bool);
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

// Initialization sequence for bright rotated 72 x 40 screen
static const uint8_t tiny4koled_init_72x40br [] = {
	0xC8,         // Set COM Output Scan Direction
	0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA8, 0x27,   // Set multiplex ratio(1 to 64)
	0xAD, 0x30,   // Select internal IREF and higher current
	0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
};

#endif
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