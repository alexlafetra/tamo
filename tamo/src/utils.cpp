#include "spritesheet.h"
#include <Arduino.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "utils.h"
#include "Tamo.h"
#include "spriteLoader.h"

#define BUTTON_PIN PIN_PB3
#define LED_A PIN_PA6
#define LED_B PIN_PC0
#define BATTERY_PIN PIN_PA7
#define UART_RX_PIN PIN_PB3
#define UART_TX_PIN PIN_PB2

#define LONG_PRESS_TIME 1000
#define DOUBLE_CLICK_TIME 200

// these prob don't need to be volatile, since readButtons() isn't called from an interrupt
volatile bool BUTTON = false;
volatile bool LONG_PRESS = false;
volatile bool DOUBLE_CLICK = false;
volatile bool SINGLE_CLICK = false;

volatile uint32_t timeOfLastButtonPress = 0;
uint32_t lastTime = 0;

extern Tamo tamo;
// extern uint8_t customSpriteData[64];

uint8_t get_sprite_data(uint16_t offset){
  // super hacky
  // if(tamo.identity == CUSTOM_SPRITE){
  //   return customSpriteData[offset%64];
  // }
  // else
  return spritesheet[offset];
}

bool itsbeen(uint32_t time){
  return((millis()-lastTime)>time);  
}

//returns a random number from 0 - range, exclusive
uint8_t randomInt(uint8_t range){
  return millis()%range;
}

uint16_t readVcc() {
  // uint16_t result = analogRead(BATTERY_PIN);
  // return result;
  return 0;
}

//reading inputs
void checkInput(){
  checkSerialConnection();
  uint8_t val = digitalRead(BUTTON_PIN);
  // uint8_t val = (PINB & (1<<PB1));//check the value from the PB1 register
  //if the button is pressed
  if(!val){
    //if the button wasn't previously pressed, then it's a fresh press
    if(!BUTTON){
      if((millis()-timeOfLastButtonPress) < (DOUBLE_CLICK_TIME)){
        DOUBLE_CLICK = true;
      }
      else{
        DOUBLE_CLICK = false;
        SINGLE_CLICK = false;
      }
      timeOfLastButtonPress = millis();
    }
    //turn on the LED
    digitalWrite(LED_A,true);
    //set the button flag
    BUTTON = true;
    //check to see if it's been held
    if((millis() - timeOfLastButtonPress) > (LONG_PRESS_TIME) ){
      LONG_PRESS = true;
    }
  }
  //if the button is released
  else{
    //turn off the LED
    // PORTB &= ~(1<<BOTTOM_LED_PIN);
    digitalWrite(LED_A,false);
    //if the button *was* held, then you just released it
    if(BUTTON){
      //if it was held for a while, it's a long press
      if((millis() - timeOfLastButtonPress) > (LONG_PRESS_TIME) ){
        LONG_PRESS = true;
        DOUBLE_CLICK = false;
      }
      //if it wasn't, then it's a single click
      else{
        SINGLE_CLICK = true;
        DOUBLE_CLICK = false;
      }
    }
    //if the button wasn't pressed down before, then don't do anything
    else{
      SINGLE_CLICK = false;
      LONG_PRESS = false;
      DOUBLE_CLICK = false;
    }
    BUTTON = false;
  }
}

void sleepHardware(){
  //turn off OLED, LEDs
  // oled.off();
  digitalWrite(LED_A,false);
  digitalWrite(LED_B,false);
}

void wakeHardware(){
  sleep_disable();                       // first thing after waking from sleep: disable sleep
  
  //reset button states, so wakeup doesn't trigger anything
  SINGLE_CLICK = false;
  LONG_PRESS = false;
  lastTime = millis();

  // oled.on();//turn screen back on
  digitalWrite(LED_A,true);
}