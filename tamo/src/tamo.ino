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
//this library is modified!
#include "Tiny4kOLEDprintless.h"
// #include "Tiny4kOLED.h"

#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
// #include <avr/wdt.h>//to keep track of time

#define W 64
#define H 32

#define BUTTON_PIN 1
#define BATTERY_PIN 3

#define LED_PIN 4
#define RAND_PIN 5 //PB5 is the ISP reset pin, but also can be used to get a rand number? maybe?
#define AUX_LED_PIN 5 //second LED
#define BRIGHTNESS 16
//time (ms) before tamo sleeps
// #define TIME_BEFORE_SLEEP 120000
// #define TIME_BEFORE_SLEEP 12000
#define TIME_BEFORE_SLEEP 60000
#define LONG_PRESS_TIME 500;
#define DOUBLE_CLICK_TIME 100;
#define MAX_HUNGER 4320 // takes 12hr = 720min = 4320s to run out
#define MAX_MENTAL 8640 //takes 24hr to fully deplete

#define TAMO 0
#define BUG 1
#define PORCINI 3

#define CREATURE BUG

void ledOn(){
  // analogWrite(LED_PIN,BRIGHTNESS);
}
void ledOff(){
  // digitalWrite(LED_PIN,LOW);
}

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
void clearScreen();

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


//Clears visible area of screen
void clearScreen(){
  for (uint8_t m = 0; m < 2; m++) {
		oled.setCursor(0, m);
		oled.fillToEOP(0);
	}
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
    ledOn();
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
    // digitalWrite(LED_PIN,LOW);
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
  // digitalWrite(LED_PIN,LOW);

  // //https://bigdanzblog.wordpress.com/2014/08/10/attiny85-wake-from-sleep-on-pin-state-change-code-example/
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  // PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin
  PCMSK |= _BV(PCINT1);                   // Use PB3 as interrupt pin

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();                          // enables the sleep bit in the mcucr register so sleep is possible  
  sleep_cpu();                            // sleep (idk what the diff is)
}

//Interrupt callback to wake Attiny back up
ISR(PCINT0_vect){
  sleep_disable();                       // first thing after waking from sleep: disable sleep...
  clearScreen();
  // PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  PCMSK &= ~_BV(PCINT1);                  // Turn off PB3 as interrupt pin
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
  #ifdef FULLSIZE
  oled.enableZoomIn();//Need this so the sprites aren't all weird
  #endif
  oled.setRotation(2);//flip display upside-down
  oled.on();
  oled.clear();
}

//lights up screen and LED until batt dies
void batteryStressTest(){
  oled.fill(0xFF);
  analogWrite(LED_PIN,BRIGHTNESS);
}

#define MAX_ADC_VALUE 1024

uint16_t readADC() {
    // Start the conversion by setting the ADSC (ADC Start Conversion) bit
    ADCSRA |= (1 << ADSC);
    
    // Wait for the conversion to finish by checking if ADSC is cleared
    while (ADCSRA & (1 << ADSC)) {
        // Wait here while conversion is in progress
    }
    
    // The 10-bit result is stored in ADC (ADC is a 16-bit register, the result is in ADC[9:0])
    return ADC;
}

void testADC(){
  // oled.clear();  
  // uint16_t quantVal = analogRead(BUTTON_PIN);
  // //each val here is 8px tall, bc of how OLED pages work
  // const uint8_t length = 16;
  // uint8_t displayVal [length];
  // for(uint8_t i = 0; i<length; i++){
  //   displayVal[i] = 0;
  // }
  // uint8_t numberOf1s = quantVal * float(length)/float(255);
  // for(uint8_t i = 0; i<numberOf1s; i++){
  //   displayVal[i] = 255;
  // }
  // oled.renderFBO2x(16,0,length,0,displayVal);
}

#include "WireFrame.h"
#include "fbo.h"

FrameBuffer fbo(18,16);

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


void setup() {
  //turn ADC off
  // ADCSRA &= ~_BV(ADEN);

  /*
      Initializing I/O 
  */
  //main button
  DDRB &= ~(1 << PB1); // Set the button pin PB3 as input (main button)
  PORTB |= (1 << PB1);  //activate pull-up resistor for PB3 (main button connects PB3 to GND)
  //ADC3
  // pinMode(BATTERY_PIN,INPUT);

  /*
      Turning on LED control
  */
  DDRB |= ( 1 << PB4 );  //set led pin to output
  // digitalWrite(LED_PIN,LOW);

  /*
      Turning on watchdog timer
  */
  // WDTCR = (1 << WDCE) | (1 << WDE); // Enable changes to WDT
  // WDTCR = (1 << WDP3) | (1 << WDP0) | (1 << WDIE); // Set prescaler to 1s and enable interrupt

  // Enable global interrupts
  sei();
  
  initOled();
  // oled.clear();

  // oled.bitmap2x(8,0,37,2,free_palestine_bmp);
  // flag = WireFrame(4,verts,3,edges);
  flag.scale = 3.0;
  flag.xPos = 7;
  flag.yPos = 8;
  // flag.rotate(15,0);
}

void loop() {
  // flag.rotate(3,1);
  // fbo.fill(0);
  // fbo.renderWireFrame(flag,1);
  // oled.renderFBO2x(16,0,20,2,fbo.buffer);
  tamo.feel();
}
