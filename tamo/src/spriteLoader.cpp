#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "Tamo.h"
#include "spriteLoader.h"
#include "flash.cpp"

enum WEBCOMMANDS:uint8_t{
    WEBSERIAL_SPRITE_UPLOAD = 0x01,
    TAMO_HELLO = 0x02,
    REQUEST_NEXT_SPRITE_PACKET = 0x03,
    SET_IDENTITY = 0x04,
    TAMO_DISCONNECT = 0x05,
    TARGET_SPRITE_SLOT = 0x06,
};

#define TAMO_SERIAL_BUFFER_SIZE 64
#define SERIAL_IDLE_TIME 1000 //1s
extern Tamo tamo;
uint8_t spriteDataBuffer[TAMO_SERIAL_BUFFER_SIZE] = {0};

bool idleSerial(){
    uint32_t time = millis();
    while(!Serial.available()){
        if((millis() - time) > SERIAL_IDLE_TIME)
            return false;
    }
    return true;
}

bool awaitBytes(uint16_t size, uint8_t * buffer){
    //wait for web response
    lastTime = millis();
    if(!idleSerial())
        return false;

    //read in the sprite data
    Serial.readBytes(buffer,size);
    return true;
}

// downloads a packet and writes 
PACKET_STATUS downloadSpritePacket(uint16_t size, uint32_t &flash_write_location, uint8_t * buffer){
    
    //request the next data packet
    Serial.write(REQUEST_NEXT_SPRITE_PACKET);
    //ask for this many bytes
    Serial.write(size);
    if(!awaitBytes(size,buffer))
        return NO_DATA_RECEIVED;
    if(!update_flash_data(flash_write_location,buffer,size,spritesheet)){
        return FLASH_FULL;
    }
    flash_write_location+=size;
    return READY_TO_CONTINUE;
}

void blinkForABit(uint16_t d, uint8_t n){
    uint8_t i = 0;
    while(i < n){
        digitalWrite(LED_A,(i%2)?HIGH:LOW);
        digitalWrite(LED_B,(i%2)?LOW:HIGH);
        delay(d);
        i++;
    }
    digitalWrite(LED_A,LOW);
    digitalWrite(LED_B,HIGH);
}

// main receiving function for saying hello & grabbing data
bool receiveSpriteOverSerial(){

    //wait for web response
    if(!idleSerial())
        return false;

    digitalWrite(LED_B,HIGH);
    
    //get the size of the sprite data
    uint8_t spriteDataSize[2] = {0};
    Serial.readBytes(spriteDataSize,2);

    uint16_t dataSize = (((uint16_t)spriteDataSize[0])<<8)|((uint16_t)spriteDataSize[1]);
    uint16_t numberOfPacketsNeeded = dataSize/TAMO_SERIAL_BUFFER_SIZE;

    //set the read in location
    uint32_t flash_write_location = custom_sprite_idle_1;

    //get sprite data in 64-byte packets, since that's the Serial buffer's size
    for(uint8_t i = 0; i<numberOfPacketsNeeded; i++){
        //if you run out of space, get outta here
        PACKET_STATUS status = downloadSpritePacket(TAMO_SERIAL_BUFFER_SIZE,flash_write_location,flash_write_buffer);
        switch(status){
            case READY_TO_CONTINUE:
                continue;
            case FLASH_FULL:
            case EEPROM_FULL:
                return true;
            case NO_DATA_RECEIVED:
            default:
                blinkForABit(500,4);
                return false;
        }
    }
    //if it's not a clean multiple of 64, grab the rest of it
    if(dataSize%TAMO_SERIAL_BUFFER_SIZE){
        //and if you run out of space here, get outta here
        PACKET_STATUS status = downloadSpritePacket(dataSize%TAMO_SERIAL_BUFFER_SIZE,flash_write_location,flash_write_buffer);
        switch(status){
            case READY_TO_CONTINUE:
                break;
            case FLASH_FULL:
            case EEPROM_FULL:
                return true;
            case NO_DATA_RECEIVED:
            default:
                blinkForABit(500,4);
                return false;
        }
    }

    Serial.write(TAMO_DISCONNECT);
    return true;
}

//checks to see if board is connected
bool checkSerialConnection(){
    //return immediately if nothing to read
    if(!Serial.available())
        return false;
    
    uint8_t firstByte[1] = {0};
    Serial.readBytes(firstByte,1);

    if(firstByte[0] == WEBSERIAL_SPRITE_UPLOAD){
        
        digitalWrite(LED_A,HIGH);

        //send hello to web
        Serial.write(TAMO_HELLO);

        if(receiveSpriteOverSerial()){
            Serial.write(TAMO_DISCONNECT);

            //set tamo's identity to the custom sprite
            EEPROM.update(EEPROM_IDENTITY_ADDR,CUSTOM_SPRITE);
            tamo.identity = CUSTOM_SPRITE;

            //reset the system, do a lil dance first
            blinkForABit(100,10);
            _PROTECTED_WRITE(RSTCTRL.SWRR,1);
            return true;
        }
    }
    else if(firstByte[0] == SET_IDENTITY){
        uint8_t secondByte[1] = {0};
        if(!idleSerial())
            return;
        Serial.readBytes(secondByte,1);
        EEPROM.update(EEPROM_IDENTITY_ADDR,secondByte[0]);
        //reset the system, do a lil dance first
        blinkForABit(100,10);
        _PROTECTED_WRITE(RSTCTRL.SWRR,1);
        return true;
    }
    return false;
}