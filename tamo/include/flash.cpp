/*
  some boilerplate nvm functions for reading/writing from flash mem!
*/

#include "spritesheet.h"

//wait for nvm to be ready
static void nvm_wait() {
    while (NVMCTRL.STATUS & NVMCTRL_FBUSY_bm);
}
// execute nvm command
static void nvm_exec(uint8_t cmd) {
    CPU_CCP = CCP_SPM_gc;           // Unlock NVM with timed write
    NVMCTRL.CTRLA = cmd; //set command
}

// #define BITMAP_FLASH_ADDR ((uint16_t)((uint16_t)spritesheet - (uint16_t)0x8000))
#define FLASH_PAGE_SIZE 64

// Fixed addresses — no pointer arithmetic, no linker symbols
#define BITMAP_FLASH_ADDR  0x3000   // LMA, used for NVMCTRL writes
#define BITMAP_VMA         0xB000   // VMA, used for readback verification

// The only address calculation we need — and we've verified this works
#define SPRITESHEET_LMA ((uint16_t)((uint16_t)spritesheet - (uint16_t)0x8000))

// Round down to page boundary (in case of any remaining misalignment)
#define SPRITESHEET_PAGE ((uint16_t)(SPRITESHEET_LMA & ~((uint16_t)63)))

// Safe floor = the bitmap page itself, derived from the pointer we trust
#define SAFE_WRITE_FLOOR SPRITESHEET_PAGE

bool flash_write_page_safe(uint16_t flash_addr, const uint8_t* data) {
    // Hard checks before touching anything
    if (flash_addr < SAFE_WRITE_FLOOR) return false;
    if (flash_addr % FLASH_PAGE_SIZE != 0) return false;
    if (flash_addr > 0x7FFF) return false;  // must be in flash range

    nvm_wait();
    *(volatile uint8_t*)flash_addr = 0xFF;
    nvm_exec(NVMCTRL_CMD_PAGEERASE_gc);
    nvm_wait();

    volatile uint8_t* p = (volatile uint8_t*)flash_addr;
    for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        p[i] = data[i];
    }

    nvm_exec(NVMCTRL_CMD_PAGEWRITE_gc);
    nvm_wait();
    return true;
}

// erase + write a page of flash (64B)
// `flash_addr` must be page-aligned (multiple of 64) so you can't just increment this by 1
// `data` must point to exactly 64 bytes
//this was made with a little help from AI :'(
void __attribute__((noinline)) flash_write_page(uint16_t flash_addr, const uint8_t* data) {
  nvm_wait();

  // Erase the page
  // Write anything to an address in the page to select it, then erase
  *(volatile uint8_t*)flash_addr = 0xFF;   // dummy write to latch address
  nvm_exec(NVMCTRL_CMD_PAGEERASE_gc);//erase the page
  nvm_wait();

  // Step 2: Load the page buffer word by word
  volatile uint8_t* p = (volatile uint8_t*)flash_addr;
  for (uint8_t i = 0; i < FLASH_PAGE_SIZE; i++) {
    p[i] = data[i];//write into the page buffer
  }

  // Step 3: Write the page buffer to flash
  nvm_exec(NVMCTRL_CMD_PAGEWRITE_gc);
  nvm_wait();
}

// Global — not on the stack
static uint8_t write_buf[FLASH_PAGE_SIZE];

void testOverwriteSprites() {
}

void debug_addrs() {
}