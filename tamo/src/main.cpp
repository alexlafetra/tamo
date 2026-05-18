
//https://github.com/Lorandil/ATTiny85-optimization-guide?tab=readme-ov-file

//pi pico UART bridge for programming:
//https://github.com/Noltari/pico-uart-bridge

// GPIO16 (Pin 21)	UART0 TX
// GPIO17 (Pin 22)	UART0 RX
// GPIO4 (Pin 6)	UART1 TX
// GPIO5 (Pin 7)	UART1 RX

/*

ok some really convoluded upload stuff:
burning fuses/bootloader needs to be done with UDPI?
CLOCK needs to be 10MHz to communicate with arduino IDE, dunno why

SUPER annoying highkey annoying vibes
 --> flashing fuses/initial upload needs to use UPDI friend (dunno why it won't work with pi pico)
 --> Subsequent UART uploads need to be done within 8s of a power cycle, which means plug/unplug attiny (or connect a lil button)
 ^^THIS is important for the new board designs, bc the user shouldn't have to power cycle the board!
 ==> Test w the 1sec timeout and see if that works. Figure out how to "Reset"/power cycle Attiny via the USB-->UART bridge
 ^^Actually, i guess this isn't important for the board design since you don't need to "enter bootloader" unless you're actually
 uploading new code, at which point yeah you'll need to reset the tamo
*/


#define BUTTON_PIN PIN_PA2
#define LED_A PIN_PA6
#define LED_B PIN_PC0
#define BATTERY_PIN PIN_PA7
#define UART_RX_PIN PIN_PB3
#define UART_TX_PIN PIN_PB2


/*
notes on pins:

PA7 -- Battery vcc

PB0 -- screen SCL
PB1 -- screen SDA

PB2 -- UART TX
PB3 -- UART RX

PA6 -- DAC out / LED A
PC0 -- LED B
*/

using namespace std;

#include <EEPROM.h>
#include "utils.h"
#include "spriteLoader.h"
#include "spritesheet.h"

//this stores the active graphics area! which is half-res of the screen, sprites are drawn 2x
#include "FrameBuffer.h"
// FrameBuffer fbo(36,24);

#include <Wire.h>
#include "Display.h"
SSD1306Device oled;

#include "Tamo.h"
Tamo tamo;

//Interrupt callback to wake Attiny back up
ISR(PORTA_PORT_vect) {
  PORTA.INTFLAGS = PIN2_bm; // Clear interrupt flag for PIN 2
  tamo.setStatusBit(IS_ASLEEP_BIT,false);
}

//RTC 'body()' timer interrupt to update tamo's health/state
//taken from: https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/PowerSave.md#unused-pins-and-sleep-modes
ISR(RTC_PIT_vect)
{
  RTC.PITINTFLAGS = RTC_PI_bm;          /* Clear interrupt flag by writing '1' (required) */
  tamo.body();
}

//initialize RTC
//also taken from: https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/PowerSave.md#unused-pins-and-sleep-modes
void RTC_init(void)
{
  /* Initialize RTC: */
  while (RTC.STATUS > 0)
  {
    ;                                   /* Wait for all register to be synchronized */
  }
  RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;    /* 32.768kHz Internal Ultra-Low-Power Oscillator (OSCULP32K) */

  RTC.PITINTCTRL = RTC_PI_bm;           /* PIT Interrupt: enabled */

  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc /* 4Hz */
  | RTC_PITEN_bm;                       /* Enable PIT counter: enabled */
}

//important to do so you save power on sleep()
void disconnectUnusedPins(){
  //everything but the button pin and the ADC batt pin should be disabled
  //led pins don't need to be disabled since they're set to output
  //so everything except:
  /*
    PA7
    PB0
    PB1
    PB2
    PA6
    PC0
  */
  const uint8_t unused_pins[] = {
    //updi pin, doesn't need to be disconnected
    // PIN_PA0,
    PIN_PA1,
    // PIN_PA2, //Button
    PIN_PA3,
    PIN_PA4,
    PIN_PA5,
    // PIN_PA6, //LED A
    PIN_PA7,
    // PIN_PB0, //SCL
    // PIN_PB1, //SDA
    // PIN_PB2, //RX
    // PIN_PB3, //TX
    PIN_PB4,
    PIN_PB5,
    PIN_PB6,
    PIN_PB7,
    // PIN_PC0, //LED B
    PIN_PC1,
    PIN_PC2,
    PIN_PC3,
    PIN_PC4,
    PIN_PC5
  };
  for(uint8_t i = 0; i<sizeof(unused_pins); i++){
    pinMode(unused_pins[i],OUTPUT);
  }
}

//trying to set the IVSEL register, so the vectors are placed in BOOT instead of APPCODE
//which happens automatically when BOOT isn't 0x00
//src: https://www.microchip.com/content/dam/mchp/documents/OTH/ApplicationNotes/ApplicationNotes/AN1983WritingtoFlashandEEPROMonthetinyAVR1-series40001983A.pdf
//sec 2.2
void onBeforeInit(){
  CCP = CCP_IOREG_gc;           // unlock protected register, 4-cycle window
  CPUINT.CTRLA = CPUINT_IVSEL_bm; // set IVSEL=1: vectors at start of BOOT (0x0000)
}

void setup(){

  /*
      Turning on LED controls
  */
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  pinMode(BATTERY_PIN,INPUT);
  /*
    Initializing button
  */
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  //set floating pins to OUTPUT (to save power during sleep)
  disconnectUnusedPins();

  //this sleep mode only leaves the RTC running, and it's the one that saves the most energy
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  //allows the CPU to go to sleep
  sleep_enable();

  /*
      Turning on watchdog timer
  */
  RTC_init();

  // Enable global interrupts
  sei();
  
  //turn on/set up the screen
  oled.begin(72, 40);
  
  //initialize UART
  Serial.begin(115200);

  tamo.identity = EEPROM.read(EEPROM_IDENTITY_ADDR);
  oled.clear();
}

enum TamoMode:uint8_t{
  NORMAL_TAMO,
  SLIDESHOW,
  TEXT
};

TamoMode mode = NORMAL_TAMO;

// FrameBuffer fbo(72,48);
FrameBuffer fbo(36,24);

void loop() {
  // oled.disableZoomIn();
  // fbo.clear();
  // fbo.bitmap_from_spritesheet(0,0,16,16,tamo_sprite_idle_1);
  // oled.renderFBO(4,0,72,3,fbo.buffer);
  // digitalWrite(LED_A,CHANGE);
  // delay(1000);

  switch(mode){
    case NORMAL_TAMO:
      tamo.live();
      break;
    case SLIDESHOW:
      break;
  }
}

