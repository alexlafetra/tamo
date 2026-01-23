
//https://github.com/Lorandil/ATTiny85-optimization-guide?tab=readme-ov-file

#include <TinyWireM.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <EEPROM.h>

#include "sprites.h"
#include "display.cpp"

using namespace std;

void hardwareSleep();

uint32_t lastTime = 0;

bool itsbeen(uint32_t time){
  return((millis()-lastTime)>time);  
}

//time (ms) before tamo sleeps
// #define TIME_BEFORE_SLEEP 60000
#define TIME_BEFORE_SLEEP 12000

void hardwareSleepCheck(){
  if(itsbeen(TIME_BEFORE_SLEEP)){
    hardwareSleep();
  }
}

void readButtons();
uint16_t readVcc();
#include "hardware.cpp"
#include "Tamo.cpp"

Tamo tamo;
void sleep(){
  //turn off OLED, LEDs
  oled.off();
  PORTB &= ~(1<<BOTTOM_LED_PIN);
  PORTB &= ~(1<<TOP_LED_PIN);

  // //https://bigdanzblog.wordpress.com/2014/08/10/attiny85-wake-from-sleep-on-pin-state-change-code-example/
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT1);                   // Use PB1 as interrupt pin
}

void wake(){
  sleep_disable();                       // first thing after waking from sleep: disable sleep
  PCMSK &= ~_BV(PCINT1);                  // Turn off PB1 interrupt
  
  //reset button states, so wakeup doesn't trigger anything
  DOUBLE_CLICK = false;
  SINGLE_CLICK = false;
  LONG_PRESS = false;
  lastTime = millis();

  oled.on();//turn screen back on
}

void hardwareSleep(){
  tamo.setStatusBit(IS_ASLEEP_BIT,true);

  sleep();

  //set tamo into sleep mode
  //when the WDT interrupt finishes it goes to this line and tamo can go back to sleep
  while(tamo.isAsleep()){
    //this sleep mode only leaves the WDT running (millis() won't update)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    //allows the CPU to go to sleep
    sleep_enable();
    //put the attiny to sleep
    sleep_cpu();
  }

  wake();
}

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

  //You need to manually setup PB4 as an analog output
  // Set Timer1 to PWM mode (10-bit phase correct)
  // TCCR1 = _BV(PWM1A) | _BV(PWM1B) | _BV(COM1A1) | _BV(COM1B1) | _BV(CS10);

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
