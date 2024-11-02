/*

Tools > Board > attiny > 85
Port > (Arduino USB port)
Clock > Internal 8Mhz (i think this is accidentally swapped w 1mhz in arduino ide since it's slower)
Processor > Attiny85
Programmer > Arduino as ISP

*/

//If you need to debug the baby screen, use the 64x32 test sketch!
#define TINY4KOLED_NO_PRINT
// #define FULLSIZE//enable zoom in, 2x bitmaps

#include <TinyWireM.h>
//this library is modified!
#include "Tiny4kOLEDprintless.h"
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>    // Power management
#include <EEPROM.h>
// #include "display.h"

#define W 64
#define H 32

#define BUTTON_PIN 3
#define LED_PIN 4
#define RAND_PIN 5 //PB5 is the ISP reset pin, but also can be used to get a rand number? maybe?
#define BRIGHTNESS 16
//time (ms) before tamo sleeps
#define TIME_BEFORE_SLEEP 6000
#define LONG_PRESS_TIME 500;

#define TAMO 0
#define BUG 1
#define CAT 2

#define CREATURE TAMO

void ledOn(){
  analogWrite(LED_PIN,BRIGHTNESS);
}
void ledOff(){
  digitalWrite(LED_PIN,LOW);
}

// #include "WireFrame.h"
// #include "fbo.h"

#define display oled

using namespace std;

bool BUTTON = false;
// bool randomGenSeeded = false;
uint64_t lastTime = 0;
uint32_t timeOfLastButtonPress = 0;

bool itsbeen(uint32_t time){
  return((millis()-lastTime)>time);  
}

void readButtons();
void hardwareSleep();
void clearEdges();
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

void clearEdges(){
  //left side
  oled.setCursor(0,0);
  oled.fillLength(0,20);
  oled.setCursor(0,1);
  oled.fillLength(0,20);

  //right side
  oled.setCursor(52,0);
  oled.fillLength(0,20);
  oled.setCursor(52,1);
  oled.fillLength(0,20);
}

//Clears visible area of screen
void clearScreen(){
  for (uint8_t m = 0; m < 2; m++) {
		oled.setCursor(0, m);
		oled.fillToEOP(0);
	}
}
//This is taken from:
//https://forum.arduino.cc/t/the-reliable-but-not-very-sexy-way-to-seed-random/65872

void seedRandomNumberGenerator()
{
  static const uint32_t HappyPrime = 17;
  union
  {
    uint32_t i;
    uint8_t b[4];
  }
  raw;
  uint8_t i;
  unsigned int seed;
  
  for ( i=0; i < sizeof(raw.b); ++i )
  {
    raw.b[i] = EEPROM.read( i );
  }

  do
  {
    raw.i += HappyPrime;
    seed = raw.i & 0x7FFFFFFF;
  }
  while ( (seed < 1) || (seed > 2147483646) );

  randomSeed( seed );  

  for ( i=0; i < sizeof(raw.b); ++i )
  {
    EEPROM.write( i, raw.b[i] );
  }
}

void readButtons(){
  bool val = !digitalRead(BUTTON_PIN);
  if(val){
    analogWrite(LED_PIN,BRIGHTNESS);
    BUTTON = true;
  }
  else{
    digitalWrite(LED_PIN,LOW);
    //if the button *was* held, then you just released it
    if(BUTTON)
      timeOfLastButtonPress = millis();
    BUTTON = false;
  }
}

void hardwareSleep(){
  //turn off OLED, LED
  oled.off();
  digitalWrite(LED_PIN,LOW);

  // //https://bigdanzblog.wordpress.com/2014/08/10/attiny85-wake-from-sleep-on-pin-state-change-code-example/
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();                          // enables the sleep bit in the mcucr register so sleep is possible  
  sleep_cpu();                            // sleep (idk what the diff is)
}

//Interrupt callback to wake Attiny back up
ISR(PCINT0_vect){
  sleep_disable();                       // first thing after waking from sleep: disable sleep...
  clearScreen();
  PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  oled.on();//turn screen back on
}

// #include "bitmaps.cpp"
// #include "sprites.h"
// #include "Animation.h"
// #include "Tamo.h"
// Tamo tamo;

void initOled(){
  //start i2c communication w little oled
  oled.begin(72, 40, sizeof(tiny4koled_init_72x40br), tiny4koled_init_72x40br);
  #ifdef FULLSIZE
  oled.enableZoomIn();//Need this so the sprites aren't all weird
  #endif
  oled.setRotation(2);//flip display upside-down
  oled.switchFrame();
  oled.on();
  oled.clear();
}

// WireFrame makeCube(){
//   Vertex vArray[4] = {Vertex(-1,-1,-1),Vertex(-1,1,-1),Vertex(1,1,-1),Vertex(1,-1,1)};
//   uint16_t edges[4][2] = {{0,1},{1,2},{2,3},{3,0}};
//   WireFrame sq(4,vArray,4,edges);
//   sq.scale = 3.0;
//   sq.xPos = 8;
//   sq.yPos = 8;
//   return sq;
// }

// FrameBuffer f;
// WireFrame w;

void setup() {
  //turn ADC off
  ADCSRA &= ~_BV(ADEN);

  //init button
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  digitalWrite(BUTTON_PIN,HIGH);
  //init led
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,LOW);

  // seedRandomNumberGenerator();

  initOled();

  //intro
  // oled.clear();
  // oled.bitmap2x(22,0,38,2,bitmap_hi);
  // delay(600);
  // oled.bitmap2x(20,0,39,2,bitmap_mai);
  // delay(600);
  // oled.bitmap2x(20,0,39,2,bitmap_li);
  // delay(600);

  // f = FrameBuffer(32,32);
  // f.fill(0xFF);

  // Vertex vArray[4] = {Vertex(-1,-1,0),Vertex(-1,1,0),Vertex(1,1,0),Vertex(1,-1,0)};
  // // uint8_t edges[4][2] = {{0,1},{1,2},{2,3},{3,0}};
  // uint8_t edges[3][2] = {{0,3},{1,2},{2,0}};
  // w = WireFrame(4,vArray,3,edges);
  // w.scale = 4.0;
  // w.xPos = 0;
  // w.yPos = 2;
}

uint8_t i = 0;
uint8_t j = 0;
void loop() {
  // ledOn();
  // w.xPos = i;
  // f.clear();
  // f.renderWireFrame(w);
  // // f.drawLine(1,1,i,31,0);
  // // f.setPixel(i,j,0);
  // f.render(5,0);
  // i++;
  // if(i>=32){
  //   i = 0;
  //   j++;
  //   if(j>=32){
  //     f.fill(0xFF);
  //     j = 0;
  //   }
  // }
  // ledOff();
  // oled.bitmap(0,0,16,2,sprite_mad_1);
  // delay(100);

  tamo.update();
}
