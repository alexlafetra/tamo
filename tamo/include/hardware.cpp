#define BUTTON_PIN 1
#define LED_PIN 3 //primary LED (blue)
#define AUX_LED_PIN 4 //secondary LED

#define LONG_PRESS_TIME 500;
#define DOUBLE_CLICK_TIME 100;

// these prob don't need to be volatile, since readButtons() isn't called from an interrupt
volatile bool BUTTON = false;
volatile bool LONG_PRESS = false;
volatile bool DOUBLE_CLICK = false;
volatile bool SINGLE_CLICK = false;

volatile uint32_t timeOfLastButtonPress = 0;


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

//This is partially from chatGPT :+)
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


void updateBreathLED(uint8_t LED){
  uint16_t value = (millis()/40)%128;
  if(value > 64){
    value = 128 - value;
  }
  if( value < 16){
    digitalWrite(LED,LOW);
  }
  else{
    value = (value)*2;
    analogWrite(LED,value);
  }
}

void talkingLED(){
  digitalWrite(LED_PIN,((millis()/200)%2)?HIGH:LOW);
}

//reading inputs
void readButtons(){
  bool val = !digitalRead(BUTTON_PIN);
  //if the button is pressed
  if(val){
    //if the button wasn't previously pressed, then it's a fresh press
    if(!BUTTON){
      if(millis()-timeOfLastButtonPress < 100){
        // DOUBLE_CLICK = true;
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
    digitalWrite(AUX_LED_PIN,HIGH);
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
    digitalWrite(AUX_LED_PIN,LOW);
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
  digitalWrite(LED_PIN,LONG_PRESS);
}


void initOled(){
  //start i2c communication w little oled
  oled.begin(72, 40, sizeof(tiny4koled_init_72x40br), tiny4koled_init_72x40br);
  //zoom the oled in
  oled.enableZoomIn();//Need this so the sprites aren't all weird
  oled.setRotation(2);//flip display upside-down
  oled.on();
  oled.clear();
}


