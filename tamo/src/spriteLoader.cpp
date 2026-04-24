#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "Tamo.h"
#include "spriteLoader.h"

enum WEBCOMMANDS:uint8_t{
    WEBSERIAL_HELLO = 0x01,
    TAMO_HELLO = 0x02,
    REQUEST_NEXT_SPRITE_PACKET = 0x03,
    SET_IDENTITY = 0x04,
    TAMO_DISCONNECT = 0x05
};

#define TAMO_SERIAL_BUFFER_SIZE 64

extern Tamo tamo;
uint8_t spriteDataBuffer[TAMO_SERIAL_BUFFER_SIZE] = {0};

void idleSerial(){
    while(!Serial.available() && !itsbeen(4000)){}
    if(itsbeen(4000))
        return;
}

void awaitBytes(uint16_t size, uint8_t * buffer){
    //wait for web response
    lastTime = millis();
    idleSerial();

    //read in the sprite data
    Serial.readBytes(buffer,size);
    //...and repeat!
}

// downloads a packet and writes 
bool downloadSpritePacket(uint16_t size, uint16_t &EEPROM_write_location, uint8_t * buffer){
    
    //request the next data packet
    Serial.write(REQUEST_NEXT_SPRITE_PACKET);
    //ask for this many bytes
    Serial.write(size);
    awaitBytes(size,buffer);

    //write this chunk of sprite data to eeprom
    for(uint8_t j = 0; j<size; j++){
        EEPROM.update(EEPROM_write_location,buffer[j]);
        EEPROM_write_location++;
        if(EEPROM_write_location == (EEPROM_SIZE - 1)){
            Serial.write(TAMO_DISCONNECT);
            return false;
        }
    }
    return true;
}

// main receiving function for saying hello & grabbing data
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
        //if you run out of space, get outta here
        if(!downloadSpritePacket(TAMO_SERIAL_BUFFER_SIZE,EEPROM_write_location,spriteDataBuffer))
            return;
    }
    //if it's not a clean multiple of 64, grab the rest of it
    if(spriteDataSize[0]%TAMO_SERIAL_BUFFER_SIZE){
        //and if you run out of space here, get outta here
        if(!downloadSpritePacket(spriteDataSize[0]%TAMO_SERIAL_BUFFER_SIZE,EEPROM_write_location,spriteDataBuffer))
            return;
    }

    Serial.write(TAMO_DISCONNECT);
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
        delay(1000);
        //reset the system (NOT WORKING RN)
        _PROTECTED_WRITE(RSTCTRL.SWRR,1);
        return true;
    }
    else return false;
}
