#include "Display.h"
#include "utils.h"

static bool datacute_write_Wire(uint8_t byte) {
	return Wire.write(byte);
}

static uint8_t datacute_read_Wire(void) __attribute__((unused));
static uint8_t datacute_read_Wire(void) {
	return Wire.read();
}

static void datacute_end_read_Wire(void) __attribute__((unused));
static void datacute_end_read_Wire(void) {}

static uint8_t datacute_endTransmission_Wire(void) {
	return Wire.endTransmission();
}

static bool tiny4koled_beginTransmission_Wire(void) {
	Wire.beginTransmission(SSD1306);
	return true;
}

#ifndef TINY4KOLED_QUICK_BEGIN
static bool tiny4koled_check_Wire(void) {
	const uint8_t noError = 0x00;
	tiny4koled_beginTransmission_Wire();
	return (datacute_endTransmission_Wire()==noError);
}
#endif

static void tiny4koled_begin_Wire(void) {
	Wire.begin();
#ifndef TINY4KOLED_QUICK_BEGIN
	while (!tiny4koled_check_Wire()) {
		// delay(10);
	}
#endif
}

static void ssd1306_begin(void) {
	tiny4koled_begin_Wire();
}

static void ssd1306_send_start(void) {
	tiny4koled_beginTransmission_Wire();
}

static bool ssd1306_send_byte(uint8_t byte) {
	return datacute_write_Wire(byte);
}

static void ssd1306_send_stop(void) {
	datacute_endTransmission_Wire();
}

static void ssd1306_send_command_start(void) {
	ssd1306_send_start();
	ssd1306_send_byte(SSD1306_COMMAND);
}

static void ssd1306_send_data_start(void) {
	ssd1306_send_start();
	ssd1306_send_byte(SSD1306_DATA);
}

static void ssd1306_send_data_byte(uint8_t byte) {
	// if (combineFn) byte = (*combineFn)(oledX + writesSinceSetCursor, oledY, byte);
	if (ssd1306_send_byte(byte) == 0) {
		ssd1306_send_stop();
		ssd1306_send_data_start();
		ssd1306_send_byte(byte);
	}
	// writesSinceSetCursor++;
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

void SSD1306Device::begin(){
	ssd1306_begin();

	ssd1306_send_command_start();
	for (uint8_t i = 0; i < 8; i++) {
		ssd1306_send_data_byte(tiny4koled_init_72x40br[i]);
	}
	ssd1306_send_stop();

	// zoom the oled in
	enableZoomIn();//Need this so the sprites aren't all weird
	setRotation(2);//flip display upside-down
	on();
	clear();
}

void SSD1306Device::begin(uint8_t width, uint8_t height) {
	oledOffsetX = (128 - width) >> 1;
	oledOffsetY = 0;
	oledWidth = width;
	oledPages = height >> 3;
	begin();
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
	// writesSinceSetCursor = 0;
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
			ssd1306_send_data_byte(bitmap[j++]);
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
			ssd1306_send_data_byte(bitmap[j]);
			//sending it twice bc for some reason it's 1/2 width when fullsize is enabled!
			ssd1306_send_data_byte(bitmap[j]);
			j++;
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}

void SSD1306Device::bitmap_from_spritesheet2x(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint16_t offset) {
	bitmap_from_spritesheet(x0,y0,w,h,offset,true);
}
void SSD1306Device::bitmap_from_spritesheet(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint16_t offset) {
	bitmap_from_spritesheet(x0,y0,w,h,offset,false);
}
void SSD1306Device::bitmap_from_spritesheet(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint16_t offset, bool twoTimesScale) {
	uint16_t j = 0;
	uint16_t y1 = abs(y0 - h)/8;
 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < w; i++) {
			ssd1306_send_data_byte(get_sprite_data(j+offset));
			if(twoTimesScale)
				ssd1306_send_data_byte(get_sprite_data(j+offset));
			j++;
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}


void SSD1306Device::overlayBitmap2x(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, const uint8_t bitmapA[], uint8_t offsetX, uint8_t offsetY, uint8_t widthB, uint8_t heightB, const uint8_t bitmapB[]){
	// int8_t width = x1-x0;
	// if(width<=0)
	// 	return;
	uint8_t j = 0;
	uint8_t jB = 0;
	uint16_t y1 = abs(y0 - h)/8;

 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < w; i++) {
			uint8_t byteA = bitmapA[j];
			if(i>=offsetX && i<(offsetX+widthB) && y>=(offsetY/8) && y<((heightB+offsetY)/8)){
				uint8_t byteB = bitmapB[jB];
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

void SSD1306Device::overlay_bitmap_from_spritesheet2x(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint16_t bitmapA_offset, uint8_t offsetX, uint8_t offsetY, uint8_t widthB, uint8_t heightB, uint16_t bitmapB_offset){

	uint8_t j = 0;
	uint8_t jB = 0;

	uint16_t y1 = abs(y0 - h)/8;

 	for (uint8_t y = y0; y <= y1; y++) {
		setCursor(x0,y);
		ssd1306_send_data_start();
		for (uint8_t i = 0; i < w; i++) {
			uint8_t byteA = get_sprite_data(j+bitmapA_offset);
			if(i>=offsetX && i<(offsetX+widthB) && y>=(offsetY/8) && y<((heightB+offsetY)/8)){
				uint8_t byteB = get_sprite_data(jB+bitmapB_offset);
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
	if(zoomed)
		return;
	zoomed = true;
	ssd1306_send_command2(0xD6, 0x01);
}

void SSD1306Device::disableZoomIn(void) {
	if(!zoomed)
		return;
	zoomed = false;
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
