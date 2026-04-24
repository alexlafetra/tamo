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
PACKET_STATUS downloadSpritePacket(uint16_t size, uint16_t &EEPROM_write_location, uint8_t * buffer){
    
    //request the next data packet
    Serial.write(REQUEST_NEXT_SPRITE_PACKET);
    //ask for this many bytes
    Serial.write(size);
    if(!awaitBytes(size,buffer))
        return NO_DATA_RECEIVED;

    //write this chunk of sprite data to eeprom
    for(uint8_t j = 0; j<size; j++){
        EEPROM.update(EEPROM_write_location,buffer[j]);
        EEPROM_write_location++;
        if(EEPROM_write_location == (EEPROM_SIZE - 1)){
            Serial.write(TAMO_DISCONNECT);
            return EEPROM_FULL;
        }
    }
    return READY_TO_CONTINUE;
}

// main receiving function for saying hello & grabbing data
bool receiveSpriteOverSerial(){

    //wait for web response
    if(!idleSerial())
        return false;
    
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
        PACKET_STATUS status = downloadSpritePacket(TAMO_SERIAL_BUFFER_SIZE,EEPROM_write_location,spriteDataBuffer);
        switch(status){
            case READY_TO_CONTINUE:
                continue;
            case EEPROM_FULL:
                return true;
            case NO_DATA_RECEIVED:
            default:
                return false;
        }
    }
    //if it's not a clean multiple of 64, grab the rest of it
    if(spriteDataSize[0]%TAMO_SERIAL_BUFFER_SIZE){
        //and if you run out of space here, get outta here
        PACKET_STATUS status = downloadSpritePacket(TAMO_SERIAL_BUFFER_SIZE,EEPROM_write_location,spriteDataBuffer);
        switch(status){
            case READY_TO_CONTINUE:
                break;
            case EEPROM_FULL:
                return true;
            case NO_DATA_RECEIVED:
            default:
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

    if(firstByte[0] == WEBSERIAL_HELLO){
        
        digitalWrite(LED_A,HIGH);

        //send hello to web
        Serial.write(TAMO_HELLO);

        if(receiveSpriteOverSerial()){
            Serial.write(TAMO_DISCONNECT);

            //set tamo's identity to the custom sprite
            EEPROM.update(EEPROM_IDENTITY_ADDR,CUSTOM_SPRITE);
            tamo.identity = CUSTOM_SPRITE;
            tamo.mood = MOOD_BIRTH;

            //reset the system (NOT WORKING RN)
            uint16_t i = 0;
            while(i < 10){
                digitalWrite(LED_A,i%2);
                digitalWrite(LED_B,!(i%2));
                delay(200);
                i++;
            }
            digitalWrite(LED_A,LOW);
            digitalWrite(LED_B,LOW);
            delay(1000);

            _PROTECTED_WRITE(RSTCTRL.SWRR,1);
            return true;
        }
    }
    digitalWrite(LED_A,LOW);
    digitalWrite(LED_B,LOW);
    return false;
}
