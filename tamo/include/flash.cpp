/*
  some boilerplate nvm functions for reading/writing from flash mem!

  some notes on this process:
  getting this to work was hard. Here're the basics:

  1. Fuse 7 in attiny needs to be set to something other than '0', this is the BOOTEND fuse and setting it to a fixed size
  lets us put data in APPCODE/APPDATA, where we're allowed to write over it.
  
  2. A custom linker script needs to create a section for the data we want to overwrite, and 
  this section has to be after the BOOTEND fuse value, so that it's placed in APP.
  We tell platform.io to use this script with:

  board_build.ldscript = path_to_script.ld

  3. Attiny automatically places interrupt vectors in APP when BOOTEND is set (lowkey...silly af), so 
  we need to update the Interrupt Vector Flag to tell it to look in BOOT instead
  with this core, we can do that before anything else by overloading the onBeforeInit() function:

  void onBeforeInit(){
    CCP = CCP_IOREG_gc;           // unlock protected register, 4-cycle window
    CPUINT.CTRLA = CPUINT_IVSEL_bm; // set IVSEL=1: vectors at start of BOOT (0x0000)
  }

  3. The data we want placed in that section needs :  __attribute__((section(".section_name"))) in its initializer

  brother... note that i just spent one (1) day debugging the flash page overlay logic to find out that:
  A) code was working fine
  B) the page size is 64 WORDS, not BYTES. It's 128 bytes
*/

#include "spritesheet.h"
#include "Arduino.h"
#include "Sprite.h"
#include "FrameBuffer.h"
#include "Tamo.h"
#include "Display.h"

#define FLASH_PAGE_SIZE 128 //BROTHER... why. such a random ass number of bytes. why not 256. why not 64.

// LMA address of the spritesheet, used for WRITING to it
#define SPRITESHEET_LMA ((uint32_t)(((uint32_t)spritesheet & 0xFFFF) - 0x8000UL))

// Safe floor = the bitmap page itself, derived from the pointer we trust
#define SAFE_WRITE_FLOOR SPRITESHEET_LMA

enum FLASH_WRITE_RESULT : unsigned char
{
  NOT_A_PAGE_BOUNDARY = 0,
  ADDRESS_IN_BOOT_SECTION = 1,
  ADDRESS_BEYOND_FLASH = 2,
  WRITE_OK = 3,
  WRITE_ERROR = 4
};

//waits for the NVM controller to be ready
static void nvm_wait() {
    while (NVMCTRL.STATUS & (NVMCTRL_FBUSY_bm | NVMCTRL_EEBUSY_bm));
}

FLASH_WRITE_RESULT flash_write_page_safe(uint32_t flash_addr, const uint8_t* data) {
    if (flash_addr < SAFE_WRITE_FLOOR) return ADDRESS_IN_BOOT_SECTION;
    if (flash_addr % FLASH_PAGE_SIZE != 0) return NOT_A_PAGE_BOUNDARY;
    if (flash_addr > 0x7FFF) return ADDRESS_BEYOND_FLASH;

    nvm_wait();

    // Fill page buffer — plain byte writes, no SPM needed
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
      *((volatile uint8_t*)(flash_addr + i + 0x8000)) = data[i];
    }

    // Issue erase+write with SPM-key protected write to NVMCTRL.CTRLA
    uint8_t sreg = SREG;
    cli();
    _PROTECTED_WRITE_SPM(NVMCTRL.CTRLA, NVMCTRL_CMD_PAGEERASEWRITE_gc);
    nvm_wait();
    SREG = sreg;

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
bool update_flash_data(uint32_t byte_location, const uint8_t* newData, uint16_t data_size, const uint8_t* oldData){
  //hardcoded for rn, since we're just working with spritesheet[]
  uint32_t write_addr = SPRITESHEET_LMA+byte_location;
  uint16_t offset = byte_location % FLASH_PAGE_SIZE;
  uint16_t numberOfFlashPages = (offset + data_size) / FLASH_PAGE_SIZE + 1;
  uint32_t aligned_flash_start_write_addr = write_addr - offset;
  uint16_t locationInData = 0;

  //copy over the full pages
  for(int32_t page = 0; page < numberOfFlashPages; page++){
    memset(flash_read_buffer,0,FLASH_PAGE_SIZE);
    for(int32_t i = 0; i<FLASH_PAGE_SIZE; i++){
      uint32_t index = (page * FLASH_PAGE_SIZE) + i;
      //if theres data for this part of the flash, then copy it into the buffer
      if(( index >= offset ) && (locationInData < data_size)){
        flash_read_buffer[i] = newData[locationInData];
        locationInData++;
      }
      else{
        flash_read_buffer[i] = oldData[index + byte_location - (byte_location % FLASH_PAGE_SIZE)];
      }
    }
    //rewrite this page to flash
    FLASH_WRITE_RESULT result = flash_write_page_safe(aligned_flash_start_write_addr + page * FLASH_PAGE_SIZE,flash_read_buffer);
    if(result == WRITE_OK)
      continue;
    else
      return false;
  }

  return true;
}