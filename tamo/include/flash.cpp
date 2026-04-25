/*
  some boilerplate nvm functions for reading/writing from flash mem!

  some notes on this process:
  getting this to work was hard. Here're the basics:

  1. Fuse 7 in attiny needs to be set to something other than '0', this is the BOOTEND fuse and setting it to a fixed size
  lets us put data in APPCODE/APPDATA, where we're allowed to write over it.
  
  2. A custom linker script needs to create a section for the data we want to overwrite, and this section has to be after the BOOTEND
  fuse value, so that it's placed in APP. We tell platform.io to use this script with:

  board_build.ldscript = path_to_script.ld

  3. Attiny automatically places interrupt vectors in APP when BOOTEND is set (lowkey...silly af), so we need to update the Interrupt Vector Flag
  with this core, we can do that before anything else by overloading the onBeforeInit() function:
  void onBeforeInit(){
    CCP = CCP_IOREG_gc;           // unlock protected register, 4-cycle window
    CPUINT.CTRLA = CPUINT_IVSEL_bm; // set IVSEL=1: vectors at start of BOOT (0x0000)
  }

  3. The data we want placed in that section needs :  __attribute__((section(".section_name"))) in its initializer

*/

#include "spritesheet.h"
#include "Arduino.h"
#include "Sprite.h"
#include "FrameBuffer.h"
#include "Tamo.h"

#define FLASH_PAGE_SIZE 64

// Fixed addresses — no pointer arithmetic, no linker symbols
#define BITMAP_FLASH_ADDR 0x6000 // LMA, used for NVMCTRL writes
#define SPRITESHEET_VMA 0xE000        // VMA, used for readback verification

// LMA address of the spritesheet, used for WRITING to it
#define SPRITESHEET_LMA ((uint16_t)((uint16_t)spritesheet - (uint16_t)0x8000))

// Round down to page boundary (in case of any remaining misalignment)
#define SPRITESHEET_PAGE ((uint16_t)(SPRITESHEET_LMA & ~((uint16_t)63)))

// Safe floor = the bitmap page itself, derived from the pointer we trust
#define SAFE_WRITE_FLOOR 0x6000

enum FLASH_WRITE_RESULT : unsigned char
{
  NOT_A_PAGE_BOUNDARY = 0,
  ADDRESS_TOO_LOW = 1,
  ADDRESS_TOO_HIGH = 2,
  WRITE_OK = 3,
  WRITE_ERROR = 4
};

//waits for the NVM controller to be ready
static void nvm_wait() {
    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm));
}

FLASH_WRITE_RESULT flash_write_page_safe(uint32_t flash_addr, const uint8_t* data) {
    if (flash_addr < SAFE_WRITE_FLOOR) return ADDRESS_TOO_LOW;
    if (flash_addr % FLASH_PAGE_SIZE != 0) return NOT_A_PAGE_BOUNDARY;
    if (flash_addr > 0x7FFF) return ADDRESS_TOO_HIGH;

    nvm_wait();

    // Fill page buffer — plain byte writes, no SPM needed
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
      *((volatile uint8_t*)(flash_addr + i + 0x8000)) = data[i];
    }

    // Issue erase+write with SPM-key protected write to NVMCTRL.CTRLA
    uint8_t sreg = SREG;
    cli();
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
    SREG = sreg;

    nvm_wait();

    if (NVMCTRL.STATUS & NVMCTRL_WRERROR_bm) {
        NVMCTRL.STATUS = NVMCTRL_WRERROR_bm;
        return WRITE_ERROR;
    }
    return WRITE_OK;
}

// Global — not on the stack
static uint8_t flash_write_buffer[FLASH_PAGE_SIZE] = {0};
static uint8_t flash_read_buffer[FLASH_PAGE_SIZE] = {0};

//safely "overlays" new data, can handle writing across multiple flash pages :)
void flash_update_page(uint32_t write_location, const uint8_t* newData, const uint8_t* oldData, uint16_t data_size){
  //hardcoded for rn, since we're just working with spritesheet[]
  uint32_t write_addr = SPRITESHEET_LMA+write_location;
  uint16_t offset = write_addr % FLASH_PAGE_SIZE;
  uint16_t numberOfFlashPages = (offset + data_size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
  Serial.print("number of flash pages: ");Serial.println(numberOfFlashPages);
  Serial.print("from index: ");Serial.println(write_location-offset);
  Serial.print("to index: ");Serial.println(write_location-offset + numberOfFlashPages*FLASH_PAGE_SIZE);
  Serial.print("data size: ");Serial.println(data_size);
  Serial.print("data starts ");Serial.print(offset);Serial.println(" bytes into first page");
  Serial.print("data ends ");Serial.print((data_size + offset) % FLASH_PAGE_SIZE);Serial.println(" bytes into last page");
  uint32_t aligned_flash_start_write_addr = write_addr - offset;
  uint16_t locationInData = 0;

  memset(flash_read_buffer,0,FLASH_PAGE_SIZE);
  //copy over the full pages
  for(int32_t page = 0; page < numberOfFlashPages; page++){
    for(int32_t i = 0; i<FLASH_PAGE_SIZE; i++){
      uint32_t index = (page * FLASH_PAGE_SIZE) + i;
      //if theres data for this part of the flash, then copy it into the buffer
      if(( index >= offset ) && locationInData < data_size){
        flash_read_buffer[i] = newData[locationInData];
        locationInData++;
      }
      //if you're not writing new data to this part, just copy whatever was there before
      else{
        //read data into buffer
        flash_read_buffer[i] = oldData[index + write_location - offset];
      }
    }
    //rewrite this page to flash
    flash_write_page_safe(aligned_flash_start_write_addr + page * FLASH_PAGE_SIZE,flash_read_buffer);
  }
}

extern FrameBuffer fbo;

void testUpdateFlashPage(){
  memset(flash_write_buffer,0b10101010,FLASH_PAGE_SIZE);
  flash_update_page(tamo_sprite_idle_1,flash_write_buffer,spritesheet,64);
  fbo.bitmap_from_spritesheet(8,0,16,16,tamo_sprite_idle_2);
  printScreenToSerial();
  fbo.clear();
  fbo.bitmap_from_spritesheet(8,0,16,16,tamo_sprite_idle_1);
  printScreenToSerial();
  fbo.clear();
  fbo.bitmap_from_spritesheet(8,0,16,16,tamo_sprite_sad_2);
  printScreenToSerial();
  fbo.clear();
  fbo.bitmap_from_spritesheet(8,0,16,16,tamo_sprite_sad_1);
  printScreenToSerial();
}

void testOverwriteSprites() {
  Serial.print("CTRLA at start: 0x");
  Serial.println(NVMCTRL.CTRLA, HEX);

  // Read before
  Serial.print("Before: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(spritesheet[tamo_sprite_idle_1+i],HEX); Serial.print(" ");
  }
  Serial.println();
  memset(flash_write_buffer, 0b10101010, FLASH_PAGE_SIZE);
  uint16_t write_addr = (SPRITESHEET_LMA + tamo_sprite_idle_1 - (tamo_sprite_idle_1%64));
  FLASH_WRITE_RESULT result = flash_write_page_safe(write_addr,flash_write_buffer);
  Serial.println(uint8_t(result));
  Serial.print("NVMCTRL.STATUS after write: 0x");
  Serial.println(NVMCTRL.STATUS, HEX);
  Serial.print("NVMCTRL.CTRLA after write: 0x");
  Serial.println(NVMCTRL.CTRLA, HEX);

  // Read after
  Serial.print("After: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(spritesheet[tamo_sprite_idle_1+i],HEX); Serial.print(" ");
  }
  Serial.println();
  fbo.bitmap_from_spritesheet(8,0,16,16,tamo_sprite_idle_1);
  printScreenToSerial();
}