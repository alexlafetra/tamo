/*

Tools > Board > attiny > 85
Port > (Arduino USB port)
Clock > Internal 8Mhz (i think this is accidentally swapped w 1mhz in arduino ide since it's slower)
Processor > Attiny85
Programmer > Arduino as ISP

*/

//https://github.com/Lorandil/ATTiny85-optimization-guide?tab=readme-ov-file

//If you need to debug the baby screen, use the 64x32 test sketch!
#define TINY4KOLED_NO_PRINT
#define FULLSIZE//enable zoom in, 2x bitmaps

#include <TinyWireM.h>
#include "display.cpp"

// #include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
// #include <avr/wdt.h>//to keep track of time

#define W 64
#define H 32

#define BUTTON_PIN 1
#define LED_PIN 4 //primary LED
#define AUX_LED_PIN 3 //secondary LED

//time (ms) before tamo sleeps
#define TIME_BEFORE_SLEEP 60000
#define LONG_PRESS_TIME 500;
#define DOUBLE_CLICK_TIME 100;
#define MAX_HUNGER 4320 // takes 12hr = 720min = 4320s to run out
#define MAX_MENTAL 8640 //takes 24hr to fully deplete

#define TAMO 0
#define BUG 1
#define PORCINI 3

using namespace std;

bool BUTTON = false;
bool LONG_PRESS = false;
bool DOUBLE_CLICK = false;
bool SINGLE_CLICK = false;

uint32_t lastTime = 0;
uint32_t timeOfLastButtonPress = 0;

bool itsbeen(uint32_t time){
  return((millis()-lastTime)>time);  
}

void readButtons();
void hardwareSleep();
void clearEdges();
void clearEdges(uint8_t distanceL, uint8_t distanceR);

#include "sprites.h"
#include "Animation.h"
#include "Tamo.h"
Tamo tamo;

/*
  Clears area to the right and left of main sprite
  so you don't need to do a full screen clear when switching
  between different sprites
*/

void clearEdges(uint8_t distanceL, uint8_t distanceR){
  //left side
  oled.setCursor(0,0);
  oled.fillLength(0,distanceL);
  oled.setCursor(0,1);
  oled.fillLength(0,distanceL);

  //right side
  oled.setCursor(48,0);
  oled.fillLength(0,64-distanceR);
  oled.setCursor(48,1);
  oled.fillLength(0,64-distanceR);
}

void clearEdges(){
  //left side
  oled.setCursor(0,0);
  oled.fillLength(0,20);
  oled.setCursor(0,1);
  oled.fillLength(0,20);

  //right side
  oled.setCursor(48,0);
  oled.fillLength(0,20);
  oled.setCursor(48,1);
  oled.fillLength(0,20);
}

//reading inputs
void readButtons(){
  bool val = !digitalRead(BUTTON_PIN);
  //if the button is pressed
  if(val){
    //if the button wasn't previously pressed, then it's a fresh press
    if(!BUTTON){
      if(millis()-timeOfLastButtonPress < 100){
        DOUBLE_CLICK = true;
        LONG_PRESS = false;
        SINGLE_CLICK = false;
      }
      else{
        DOUBLE_CLICK = false;
        SINGLE_CLICK = false;
      }
      timeOfLastButtonPress = millis();
    }
    //turn on the LED
    digitalWrite(LED_PIN,HIGH);
    //set the button flag
    BUTTON = true;
    //check to see if it's been held
    if((millis() - timeOfLastButtonPress) > (500) ){
      LONG_PRESS = true;
    }
  }
  //if the button is released
  else{
    //turn off the LED
    digitalWrite(LED_PIN,LOW);
    //if the button *was* held, then you just released it
    if(BUTTON){
      //if it was held for a while, it's a long press
      if((millis() - timeOfLastButtonPress) > (500) ){
        LONG_PRESS = true;
      }
      //if it wasn't, then it's a single click
      else{
        SINGLE_CLICK = true;
      }
    }
    //if the button wasn't pressed down before, then don't do anything
    else{
      SINGLE_CLICK = false;
      LONG_PRESS = false;
    }
    BUTTON = false;
  }
}

uint8_t pseudoRandom(uint8_t min, uint8_t max){
  return millis()%(max-min)+min;
}

void hardwareSleepCheck(){
  if(itsbeen(TIME_BEFORE_SLEEP)){
    hardwareSleep();
  }
}

void hardwareSleep(){
  //turn off OLED, LED
  oled.off();
  digitalWrite(LED_PIN,LOW);

  // //https://bigdanzblog.wordpress.com/2014/08/10/attiny85-wake-from-sleep-on-pin-state-change-code-example/
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT1);                   // Use PB1 as interrupt pin

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();                          // enables the sleep bit in the mcucr register so sleep is possible  
  sleep_cpu();                            // sleep (idk what the diff is)
}

//Interrupt callback to wake Attiny back up
ISR(PCINT0_vect){
  sleep_disable();                       // first thing after waking from sleep: disable sleep...
  oled.fill(0);
  PCMSK &= ~_BV(PCINT1);                  // Turn off PB1 interrupt
  oled.on();//turn screen back on
  lastTime = millis();
}


// Timer0 overflow interrupt, this triggers every 10 seconds!
// ISR(WDT_vect) {
//   tamo.body();//tamo gets hungrier
// }

void initOled(){
  //start i2c communication w little oled
  oled.begin(72, 40, sizeof(tiny4koled_init_72x40br), tiny4koled_init_72x40br);
  // ssd1306_send_command2(0xD6, 0x01);
  oled.enableZoomIn();//Need this so the sprites aren't all weird
  oled.setRotation(2);//flip display upside-down
  oled.on();
  oled.clear();
}

//lights up screen and LED until batt dies
void batteryStressTest(){
  oled.fill(0xFF);
  digitalWrite(LED_PIN,HIGH);
}

/*
Calibration notes:
@5V it's ~212
@3.9V it's ~260
@3.73 it's ~295
@3.3V it's ~358
@1.57 it's ~703

voltage = -150.70169*(measurement)+893.05592

attiny85 can operate from 1.8v - 5.5v

*/

//This is from chatGPT
uint16_t readVcc() {

  //Enable ADC
  ADCSRA |= 1<<ADEN;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(MUX3) | _BV(MUX2); // Select internal 1.1V (on ATTiny85)
  // delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // Wait until done
  uint16_t result = ADC;

  //disable ADC
  ADCSRA &= ~_BV(ADEN);
  return result;
}

void setup() {

  /*
      Initializing button
  */
  DDRB &= ~(1 << PB1); // Set the button pin PB1 as input (main button)
  PORTB |= (1 << PB1);  //activate pull-up resistor for PB1 (main button connects PB1 to GND)

  /*
      Turning on LED controls
  */
  DDRB |= ( 1 << PB4 );  //set led pin to output
  DDRB |= ( 1 << PB3 );  //set led2 pin to output

  /*
      Turning on watchdog timer
  */
  // WDTCR = (1 << WDCE) | (1 << WDE); // Enable changes to WDT
  // WDTCR = (1 << WDP3) | (1 << WDP0) | (1 << WDIE); // Set prescaler to 1s and enable interrupt

  // Enable global interrupts
  sei();
  
  initOled();
  digitalWrite(AUX_LED_PIN,HIGH);
}

void loop() {
  tamo.feel();
}
