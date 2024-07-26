/*

Tools > Board > attiny > 85
Port > (Arduino USB port)
Clock > Internal 8Mhz (i think this is accidentally swapped w 1mhz in arduino ide since it's slower)
Processor > Attiny85
Programmer > Arduino as ISP

*/

//If you need to debug the baby screen, use the 64x32 test sketch!
#define TINY4KOLED_NO_PRINT
#define FULLSIZE//enable zoom in, 2x bitmaps

#include <TinyWireM.h>
//this library is modified!
// #include "Tiny4kOLED/src/Tiny4kOLEDprintless.h"
#include "Tiny4kOLEDprintless.h"
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>    // Power management

//not sure what this does
// #define TINY4KOLED_QUICK_BEGIN

#define W 64
#define H 32

#define BUTTON_PIN 3
#define LED_PIN 4

#define display oled

using namespace std;

bool BUTTON = false;
uint16_t lastTime;

bool itsbeen(uint16_t time){
  return((millis()-lastTime)>time);  
}

void readButtons(){
  BUTTON = !digitalRead(BUTTON_PIN);
  if(BUTTON){
    digitalWrite(LED_PIN,HIGH);
  }
  else{
    digitalWrite(LED_PIN,LOW);
  }
}

void pulseLED(){
  analogWrite(LED_PIN,abs(sin(float(millis())/1.0))*255.0);
}

void hardwareSleep(){
  //turn off OLED, LED
  oled.off();
  digitalWrite(LED_PIN,LOW);

  //https://bigdanzblog.wordpress.com/2014/08/10/attiny85-wake-from-sleep-on-pin-state-change-code-example/
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  // sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  // cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on
}

#include "sprites.h"
//animation class
#include "Animation.h"
#include "Tamo.h"
Tamo tamo;

void titleScreen(){
  delay(100);
}

void initOled(){
  //start i2c communication w little oled
  // oled.begin(72,40, sizeof(tiny4koled_init_64x32br), tiny4koled_init_64x32br);
  oled.begin(72, 40, sizeof(tiny4koled_init_72x40br), tiny4koled_init_72x40br);
  #ifdef FULLSIZE
  oled.enableZoomIn();//Need this so the sprites aren't all weird
  #endif
  oled.setRotation(2);//flip display upside-down
  oled.switchFrame();
  // oled.setPages(12);
  oled.on();
}

void setup() {
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  srand(analogRead(BUTTON_PIN));
  digitalWrite(BUTTON_PIN,HIGH);
  pinMode(LED_PIN,OUTPUT);
  initOled();


  // cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  // ADCSRA |= _BV(ADEN);                    // ADC on

  //intro
  oled.clear();
  oled.bitmap2x(22,0,38,2,bitmap_hi);
  delay(600);
  oled.bitmap2x(20,0,39,2,bitmap_mai);
  delay(600);
  oled.bitmap2x(20,0,39,2,bitmap_li);
  delay(600);
}


void loop() {
  tamo.neutral();
  tamo.vibeCheck();
}
