#include <Arduino.h>
#include <EEPROM.h>
#include "Tamo.h"
#include "debug/numbers.cpp"


enum WEBCOMMANDS:uint8_t{
    WEBSERIAL_HELLO = 0x67,
    TAMO_HELLO = 0x68,
    REQUEST_NEXT_SPRITE_PACKET = 0x69,
    SET_IDENTITY = 0x6A,
    TAMO_DISCONNECT = 0x6B
};

#define TAMO_SERIAL_BUFFER_SIZE 64

extern Tamo tamo;
void receiveSpriteOverSerial();

void testNumbers(){
    for(uint8_t i = 0; i<200; i++){
        fbo.clear();
        drawNumber(i);
        oled.renderFBO2x(4,0,36,3,fbo.buffer);
        delay(10);
    }
}

void idleSerial(){
    while(!Serial.available() && !itsbeen(4000)){}
    if(itsbeen(4000))
        return;
}

uint8_t spriteDataBuffer[TAMO_SERIAL_BUFFER_SIZE] = {0};

void awaitBytes(uint16_t size, uint8_t * buffer){
    //request the sprite data
    Serial.write(REQUEST_NEXT_SPRITE_PACKET);
    //ask for this many bytes
    Serial.write(size);
    //wait for web response
    lastTime = millis();
    idleSerial();

    //read in the sprite data
    Serial.readBytes(buffer,size);
    //...and repeat!
}


//checks to see if board is connected
bool checkSerialConnection(){
    //return immediately if nothing to read
    if(!Serial.available())
        return false;
    
    uint8_t firstByte[1] = {0};
    Serial.readBytes(firstByte,1);

    if(firstByte[0] == WEBSERIAL_HELLO){
        receiveSpriteOverSerial();
        //set tamo's identity to the custom sprite
        EEPROM.update(EEPROM_IDENTITY_ADDR,CUSTOM_SPRITE);
        tamo.identity = CUSTOM_SPRITE;
        //reset the system (NOT WORKING RN)
        // _PROTECTED_WRITE(RSTCTRL.SWRR,1);
        return true;
    }
    else return false;
}

void receiveSpriteOverSerial(){

    //send hello to web
    Serial.write(TAMO_HELLO);

    //wait for web response
    idleSerial();
    
    //get the size of the sprite data
    uint8_t spriteDataSize[2] = {0};
    Serial.readBytes(spriteDataSize,2);

    uint16_t dataSize = (((uint16_t)spriteDataSize[0])<<8)|((uint16_t)spriteDataSize[1]);
    uint16_t numberOfPacketsNeeded = dataSize/TAMO_SERIAL_BUFFER_SIZE;

    //initialize buffer
    uint16_t EEPROM_write_location = CUSTOM_SPRITE_DATA_ADDR;

    //get sprite data in 64-byte packets, since that's the Serial buffer's size
    for(uint8_t i = 0; i<numberOfPacketsNeeded; i++){
        awaitBytes(TAMO_SERIAL_BUFFER_SIZE,spriteDataBuffer);
        //write this chunk of sprite data to eeprom
        for(uint8_t j = 0; j<TAMO_SERIAL_BUFFER_SIZE; j++){
            EEPROM.update(EEPROM_write_location,spriteDataBuffer[j]);
            EEPROM_write_location++;
            if(EEPROM_write_location == (EEPROM_SIZE - 1)){
                Serial.write(TAMO_DISCONNECT);
                return;
            }
        }
    }
    //if it's not a clean multiple of 64, grab the rest of it
    if(spriteDataSize[0]%TAMO_SERIAL_BUFFER_SIZE){
        awaitBytes(spriteDataSize[0]%TAMO_SERIAL_BUFFER_SIZE,spriteDataBuffer);
        //write this chunk of sprite data to eeprom
        for(uint8_t j = 0; j<(spriteDataSize[0]%TAMO_SERIAL_BUFFER_SIZE); j++){
            EEPROM.update(EEPROM_write_location,spriteDataBuffer[j]);
            EEPROM_write_location++;
            if(EEPROM_write_location == (EEPROM_SIZE - 1)){
                Serial.write(TAMO_DISCONNECT);
                return;
            }
        }
    }
    Serial.write(TAMO_DISCONNECT);
}