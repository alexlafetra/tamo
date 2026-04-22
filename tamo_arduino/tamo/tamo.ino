
//https://github.com/Lorandil/ATTiny85-optimization-guide?tab=readme-ov-file

//pi pico UART bridge for programming:
//https://github.com/Noltari/pico-uart-bridge

// GPIO16 (Pin 21)	UART0 TX
// GPIO17 (Pin 22)	UART0 RX
// GPIO4 (Pin 6)	UART1 TX
// GPIO5 (Pin 7)	UART1 RX


#define BUTTON_PIN PIN_PB3
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

#include "utils.h"
// #include "flash.h"

//this stores the active graphics area! which is half-res of the screen, sprites are drawn 2x
#include "FrameBuffer.h"
FrameBuffer fbo(36,24);

#include <Wire.h>
#include "Display.h"
SSD1306Device oled;

#include "Tamo.h"
Tamo tamo;

//Interrupt callback to wake Attiny back up
ISR(PORTB_PORT_vect) {
  PORTB.INTFLAGS = PIN3_bm; // Clear interrupt flag for PIN 2
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

  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc; /* 4Hz
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
    PB3
    PA6
    PC0
  */
  const uint8_t unused_pins[] = {
    //updi pin, doesn't need to be disconnected
    // PIN_PA0,
    PIN_PA1,
    PIN_PA2,
    // PIN_PA_3,
    PIN_PA4,
    PIN_PA5,
    // PIN_PA6,
    // PIN_PA7,
    // PIN_PB0,
    // PIN_PB1,
    // PIN_PB2,
    // PIN_PB3,
    PIN_PB4,
    PIN_PB5,
    PIN_PB6,
    PIN_PB7,
    // PIN_PC0,
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


void setup_old(){

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

  //set pin change interrupt on the button pin
  //from: https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/Ref_PinInterrupts.md
  PORTB.PIN3CTRL|= 0x01; //ISC = 1 trigger both          <--- Change if no PORTB

  //set floating pins to OUTPUT (to save power during sleep)
  disconnectUnusedPins();

  //disabling ADC (it's enabled whenever tamo measures battery VCC, then disabled again)
  // ADCSRA &= ~_BV(ADEN);

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

  digitalWrite(LED_B,CHANGE);
}

void setup(){
    // cli();
    // Force interrupt vectors to BOOT section
    CPU_CCP = CCP_IOREG_gc;
    CPUINT.CTRLA = 0; // IVSEL=0 means vectors at 0x0000 (BOOT)
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  Serial.begin(115200);
  Serial.println("hello!");
  // Serial.println(RSTCTRL.RSTFR);
  // Serial.println(CLKCTRL.MCLKCTRLA, HEX);
  // Serial.println(CLKCTRL.MCLKCTRLB, HEX);
  // Serial.println(CLKCTRL.OSC20MCALIBA, HEX);
  // Serial.println(CLKCTRL.OSC20MCALIBB, HEX);
  digitalWrite(LED_B,CHANGE);
}

void sendFBOOverSerial(){
  Serial.println("*----------------*");
  for(uint8_t y = 0; y<fbo.height; y++){
    for(uint8_t x = 0; x<fbo.width; x++){
      uint8_t pixel = fbo.getPixel(x,y);
      if(pixel)
        Serial.print('0');
      else
        Serial.print(' ');
    }
    Serial.println(' ');
  }
  Serial.println("*----------------*");
}


void loop() {
  // tamo.live();
  // delay(2000);
  // Serial.println(millis());

  digitalWrite(LED_A,CHANGE);
  digitalWrite(LED_B,CHANGE);
  Serial.println(RSTCTRL.RSTFR);
  delay(1000);

  // fbo.clear();
  // // fbo.bitmap_from_spritesheet(8,0,16,16,0);
  // oled.renderFBO2x(4,0,36,3,fbo.buffer);
  // delay(2000);
  // // testOverwriteSprites();
  // debug_nvmctrl();
  // fbo.clear();
  // // fbo.bitmap_from_spritesheet(8,0,16,16,0);
  // oled.renderFBO2x(4,0,36,3,fbo.buffer);
  // digitalWrite(LED_A,LOW);
  // digitalWrite(LED_B,HIGH);
  // uint8_t i = 0;
  // i++;


  // uint8_t test = 0;
  // test--;
  // Serial.print("spritesheet ptr: 0x");
  // Serial.println((uint16_t)spritesheet, HEX);  // should print B000

  // fbo.bitmap_from_spritesheet(0,0,16,16,0);
  // oled.renderFBO2x(4,0,36,3,fbo.buffer);
  // sendFBOOverSerial();
  // delay(1000);

}
