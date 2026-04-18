
//https://github.com/Lorandil/ATTiny85-optimization-guide?tab=readme-ov-file

//pi pico UART bridge for programming:
//https://github.com/Noltari/pico-uart-bridge

// GPIO16 (Pin 21)	UART0 TX
// GPIO17 (Pin 22)	UART0 RX
// GPIO4 (Pin 6)	UART1 TX
// GPIO5 (Pin 7)	UART1 RX

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

#define BUTTON_PIN PIN_PB3
#define LED_A PIN_PA6
#define LED_B PIN_PC0
#define BATTERY_PIN PIN_PA7

#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <EEPROM.h>

using namespace std;

uint32_t lastTime = 0;

bool itsbeen(uint32_t time){
  return((millis()-lastTime)>time);  
} __attribute__((noinline));

//returns a random number from 0 - range, exclusive
uint8_t randomInt(uint8_t range){
  return millis()%range;
}

#include "spriteFrames.cpp"
#include "display.cpp"
#include <Wire.h>
SSD1306Device oled;
#include "wireframe/fbo.h"

//this stores the active graphics area! which is half-res of the screen, sprites are drawn 2x
FrameBuffer fbo(36,24);


const Vertex verts[9] = {
  //outline
  Vertex(-2.5,-1.5,0),Vertex(2.5,-1.5,0),Vertex(2.5,1.5,0),Vertex(-2.5,1.5,0),
  //triangle tip
  Vertex(-1,0,0),
  //stripes
  Vertex(-1.25,-0.25,0),Vertex(2.5,-0.25,0),Vertex(-1.25,0.25,0),Vertex(2.5,0.25,0)
};

const uint8_t edges[8][2] = {
  //rect
  {0,1},{1,2},{2,3},{3,0},
  //triangle
  {0,4},{4,3},
  //stripes
  {5,6},{7,8}
};

WireFrame flag(9,verts,8,edges);

#include "hardware.cpp"
#include "Tamo.cpp"

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


// UART tutorial
// https://avr8.com/attiny3217-uart-tutorial/
void initUART(){
  PORTB.DIRSET = PIN2_bm;         // Set PB2 as an output for USART0 Tx
  // USART0.BAUD = 1389;             // Set UART baud rate to 9600 (20MHz / 6 clock)
  USART0.BAUD = 1111;             // Set UART baud rate to 9600 (16MHz / 6 clock)
  USART0.CTRLB = USART_TXEN_bm;   // UART transmitter enable
}

// Transmit a string using the USART
void str_tx(char *txt)
{
    int index = 0;  // Index into the string
    
    // Send characters from the string until the string terminator is found
    while (txt[index] != '\0') {
        if (USART0.STATUS & USART_DREIF_bm) {   // Check if data can be sent
            USART0.STATUS = USART_TXCIF_bm;     // Clear the transmit complete flag
            USART0.TXDATAL = txt[index];        // Sent a character
            index++;                            // Point to next character
        }
    }
}

// Transmit a string using the USART
void data_tx(unsigned char *data, uint16_t size)
{
  for(uint16_t i = 0; i<size; i++){
    // wait until data can be sent
    while (!(USART0.STATUS & USART_DREIF_bm));
    if (USART0.STATUS & USART_DREIF_bm) {
        USART0.STATUS = USART_TXCIF_bm;     // Clear the transmit complete flag
        USART0.TXDATAL = data[i];        // send byte
    }
  }
}

void setup() {

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

  //initialize UART
  initUART();

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

  //wireframe testing
  flag.scale = 3.0;
  flag.xPos = 16;
  flag.yPos = 8;
  flag.rotate(15,0);
  delay(500);
}

uint8_t d[] = {
  '1'
};

void loop() {
  // tamo.live();
  data_tx(d,sizeof(d));

  // flag.rotate(4,1);
  // fbo.clear();
  // fbo.renderWireFrame(flag,1);
  // oled.renderFBO2x(4,0,36,3,fbo.buffer);
}
