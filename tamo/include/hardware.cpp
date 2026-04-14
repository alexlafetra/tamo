#define LONG_PRESS_TIME 1000
#define DOUBLE_CLICK_TIME 200

// these prob don't need to be volatile, since readButtons() isn't called from an interrupt
volatile bool BUTTON = false;
volatile bool LONG_PRESS = false;
volatile bool DOUBLE_CLICK = false;
volatile bool SINGLE_CLICK = false;

volatile uint32_t timeOfLastButtonPress = 0;

uint16_t readVcc() {
  uint16_t result = analogRead(BATTERY_PIN);
  return result;
}

//reading inputs
void readButton(){
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

//time (ms) before tamo sleeps
// #define TIME_BEFORE_SLEEP 60000
#define TIME_BEFORE_SLEEP 12000

void sleepHardware(){
  //turn off OLED, LEDs
  oled.off();
  digitalWrite(LED_A,false);
  digitalWrite(LED_B,false);
}

void wakeHardware(){
  sleep_disable();                       // first thing after waking from sleep: disable sleep
  
  //reset button states, so wakeup doesn't trigger anything
  SINGLE_CLICK = false;
  LONG_PRESS = false;
  lastTime = millis();

  oled.on();//turn screen back on
  digitalWrite(LED_A,true);
}



