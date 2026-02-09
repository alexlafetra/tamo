
//https://github.com/Lorandil/ATTiny85-optimization-guide?tab=readme-ov-file

#include <TinyWireM.h>
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
#include "hardware.cpp"
#include "Tamo.cpp"

Tamo tamo;

//Interrupt callback to wake Attiny back up
ISR(PCINT0_vect){
  tamo.setStatusBit(IS_ASLEEP_BIT,false);
}

// Watchdog timer interrupt to run tamo's health fn
ISR(WDT_vect) {
  tamo.body();
}

void setup() {

  //disabling ADC (it's enabled when VCC is being measured)
  ADCSRA &= ~_BV(ADEN);

  /*
      Initializing button
  */
  DDRB &= ~(1 << PB1); // Set the button pin PB1 as input (main button)
  PORTB |= (1 << PB1);  //activate pull-up resistor for PB1 (main button connects PB1 to GND)

  /*
      Turning on LED controls
  */
  DDRB |= ( 1 << PB3 );  //set led2 pin to output
  DDRB |= ( 1 << PB4 );  //set led pin to output

  /*
      Turning on watchdog timer
  */
  WDTCR = (1 << WDCE) | (1 << WDE); // Enable changes to WDT
  WDTCR = (1 << WDP3) | (1 << WDP0) | (1 << WDIE); // Set prescaler to 1s and enable interrupt

  // Enable global interrupts
  sei();
  
  //turn on/set up the screen
  initOled();
}

void loop() {
  tamo.live();
}
