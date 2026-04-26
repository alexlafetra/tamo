#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define BUTTON_PIN PIN_PB3
#define LED_A PIN_PA6
#define LED_B PIN_PC0
#define BATTERY_PIN PIN_PA7
#define UART_RX_PIN PIN_PB3
#define UART_TX_PIN PIN_PB2

#define LONG_PRESS_TIME 1000
#define DOUBLE_CLICK_TIME 200

uint8_t get_sprite_data(uint16_t offset);
bool itsbeen(uint32_t time);

//returns a random number from 0 - range, exclusive
uint8_t randomInt(uint8_t range);

uint16_t readVcc();

//reading inputs
void checkInput();

//time (ms) before tamo sleeps
// #define TIME_BEFORE_SLEEP 60000
#define TIME_BEFORE_SLEEP 12000

void sleepHardware();
void wakeHardware();