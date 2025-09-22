/*

Tools > Board > attiny > 85
Port > (Arduino USB port)
Clock > Internal 8Mhz (i think this is accidentally swapped w 1mhz in arduino ide since it's slower)
Processor > Attiny85
Programmer > Arduino as ISP


notes: measuring 0.2mA during sleep

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
#include <EEPROM.h>

#define RESET_EEPROM true

//Got this from:https://community.arduboy.com/t/progmem-functions-pgm-read-float-and-pgm-read-ptr/5771
//which avoids a c++ sensitivity to implicit typecasting
template<typename T> T * pgm_read_pointer(T * const * pointer)
{
	return reinterpret_cast<T *>(pgm_read_ptr(pointer));
}

using namespace std;

uint32_t lastTime = 0;

bool itsbeen(uint32_t time){
  return((millis()-lastTime)>time);  
}

void readButtons();
void hardwareSleep();
void clearEdges();
void clearEdges(uint8_t distanceL, uint8_t distanceR);
uint16_t readVcc();
void hardwareSleepCheck();

// int rndSeed = 0;
// uint8_t pseudoRandom(uint8_t min, uint8_t max){
//   return (millis()+rndSeed++)%(max-min)+min;
// }


#include "hardware.cpp"
#include "spritesheet.h"
#include "Animation.h"
#include "Tamo.cpp"
Tamo tamo;


void hardwareSleepCheck(){
  if(itsbeen(TIME_BEFORE_SLEEP)){
    hardwareSleep();
  }
}

void hardwareSleep(){
  tamo.setStatusBit(IS_ASLEEP,true);
  //turn off OLED, LED
  // oled.clear();
  oled.off();
  digitalWrite(LED_PIN,LOW);
  digitalWrite(AUX_LED_PIN,LOW);

  // TCCR1 = 0; //reset timer 1 control register

  // //https://bigdanzblog.wordpress.com/2014/08/10/attiny85-wake-from-sleep-on-pin-state-change-code-example/
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT1);                   // Use PB1 as interrupt pin

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

  sleep_disable();                       // first thing after waking from sleep: disable sleep...
  PCMSK &= ~_BV(PCINT1);                  // Turn off PB1 interrupt
  oled.on();//turn screen back on
  // oled.clear();//clear screen of noise
  lastTime = millis();
}

//Interrupt callback to wake Attiny back up
ISR(PCINT0_vect){
  tamo.setStatusBit(IS_ASLEEP,false);
}

// Watchdog timer interrupt to run tamo's health fn
ISR(WDT_vect) {
  tamo.body();
}


void initOled(){
  //start i2c communication w little oled
  oled.begin(72, 40, sizeof(tiny4koled_init_72x40br), tiny4koled_init_72x40br);
  // ssd1306_send_command2(0xD6, 0x01);
  oled.enableZoomIn();//Need this so the sprites aren't all weird
  oled.setRotation(2);//flip display upside-down
  oled.on();
  oled.clear();
}

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
  oled.setCursor(64-distanceR,0);
  oled.fillLength(0,64-distanceR);
  oled.setCursor(64-distanceR,1);
  oled.fillLength(0,64-distanceR);
}

void clearEdges(){
  clearEdges(20,20);
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

  #if RESET_EEPROM
  EEPROM.put(255,IDENTITY_ADDRESS);
  #endif
  
  //turn on/set up the screen
  initOled();

  //run the birth interaction
  tamo.birth();
  //grab new emotion depending on health & batt state
  tamo.vibeCheck();
}

void loop() {
  // tamo.debugCheckMoodSprites();
  tamo.live();
}
