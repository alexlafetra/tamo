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

void debug_nvmctrl() {
    Serial.print("NVMCTRL.STATUS raw: 0x");
    Serial.println(NVMCTRL.STATUS, HEX);
    Serial.print("NVMCTRL_FBUSY_bm value: 0x");
    Serial.println((uint8_t)NVMCTRL_FBUSY_bm, HEX);
    Serial.print("STATUS & FBUSY_bm: 0x");
    Serial.println((uint8_t)(NVMCTRL.STATUS & NVMCTRL_FBUSY_bm), HEX);
    Serial.print("NVMCTRL.CTRLA: 0x");
    Serial.println(NVMCTRL.CTRLA, HEX);

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
    //reading bytes out
    // Serial.print("spritesheet ptr: 0x");
    // Serial.println((uint16_t)spritesheet, HEX);
    // Serial.print("BITMAP_FLASH_ADDR: 0x");
    // Serial.println(BITMAP_FLASH_ADDR, HEX);
    
    // // Read first 16 bytes before and after the write address
    // Serial.println("Bytes at BITMAP_VMA-16 to BITMAP_VMA+16 before write:");
    // const uint8_t* p = (const uint8_t*)(BITMAP_VMA - 16);
    // for (int i = -16; i < 16; i++) {
    //     Serial.print(i); Serial.print(":0x");
    //     Serial.print(p[16+i], HEX); Serial.print(" ");
    // }
    // Serial.println();
    Serial.print("spritesheet ptr: 0x");
    Serial.println((uint16_t)spritesheet, HEX);
    Serial.print("BITMAP_FLASH_ADDR: 0x");
    Serial.println((uint16_t)BITMAP_FLASH_ADDR, HEX);
    Serial.print("NVMCTRL.STATUS before: 0x");
    Serial.println(NVMCTRL.STATUS, HEX);
    Serial.print("NVMCTRL.CTRLA before: 0x");
    Serial.println(NVMCTRL.CTRLA, HEX);
    
    memset(write_buf, 0, FLASH_PAGE_SIZE);
    cli();
    bool ok = flash_write_page_safe(BITMAP_FLASH_ADDR, write_buf);
    sei();
    
    Serial.print("Write attempted: ");
    Serial.println(ok ? "YES" : "NO");
    Serial.print("NVMCTRL.STATUS after: 0x");
    Serial.println(NVMCTRL.STATUS, HEX);
    
    // Read back via VMA
    const uint8_t* readback = (const uint8_t*)BITMAP_VMA;
    Serial.print("First 8 bytes after write: ");
    for (int i = 0; i < 8; i++) {
        Serial.print(readback[i], HEX); Serial.print(" ");
    }
    Serial.println();
}

void debug_addrs() {
}