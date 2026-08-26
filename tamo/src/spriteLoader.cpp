#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "Tamo.h"
#include "spriteLoader.h"
#include "flash.cpp"
#include "Display.h"

enum SERIAL_COMMANDS:uint8_t{
    WEBSERIAL_SPRITE_UPLOAD = 0x01,
    TAMO_HELLO = 0x02,
    REQUEST_NEXT_DATA_PACKET = 0x03,
    SET_IDENTITY = 0x04,
    TAMO_DISCONNECT = 0x05,
    TARGET_SPRITE_SLOT = 0x06,
    SET_MODE = 0x07,
    WEBSERIAL_SLIDESHOW_UPLOAD = 0x08,
    INITIATE_SPRITE_SWAP = 0x09,
    SPRITE_SWAP_READY = 0x0A,
    EXPORT_SPRITE = 0x0D,
    EXPORT_SPRITE_READY = 0x0E
};

enum PACKET_STATUS:uint8_t{
    NO_DATA_RECEIVED,
    EEPROM_FULL,
    FLASH_FULL,
    FLASH_WRITE_ERROR,
    READY_TO_CONTINUE
};

#define TAMO_SERIAL_BUFFER_SIZE 64
#define SERIAL_IDLE_TIME 100 //0.1s


void clearSerialBuffer(){
    while(Serial.available()){
        Serial.read();
    }
}

bool idleSerial(uint16_t amount){
    uint32_t time = millis();
    while((Serial.available() < amount)){
        if((millis() - time) > SERIAL_IDLE_TIME)
            return false;
    }
    return true;
}
bool idleSerial(){
    return idleSerial(1);
}

bool awaitBytes(uint8_t * buffer,uint16_t size){

    //wait for connection to timeout
    if(!idleSerial())
        return false;

    //read in the sprite data
    // return (Serial.readBytes(buffer,size) == size);
    Serial.readBytes(buffer,size);
    return true;
}

// requests a packet, and a size, then waits for it to be received
PACKET_STATUS downloadDataPacket(uint8_t * buffer, uint8_t size){
    //clear out buffer
    clearSerialBuffer();
    memset(buffer, 0x00, size);
    uint8_t request[2] = {REQUEST_NEXT_DATA_PACKET,size};
    //request the next data packet
    Serial.write(request,2);
    Serial.flush();

    if(!awaitBytes(buffer,size)){
        return NO_DATA_RECEIVED;
    }
    else{
        return READY_TO_CONTINUE;
    }
}

// main receiving function for saying hello & grabbing data
bool loadDataFromSerial(uint32_t flash_write_location){

    //wait for web response
    if(!idleSerial()){
        return false;
    }
    
    //get the size of the data
    uint8_t size[2] = {0};
    Serial.readBytes(size,2);

    uint16_t dataSize = (((uint16_t)size[0])<<8)|((uint16_t)size[1]);
    uint16_t numberOfPacketsNeeded = dataSize/TAMO_SERIAL_BUFFER_SIZE;

    //get sprite data in 64-byte packets, since that's the Serial buffer's size
    for(uint8_t i = 0; i<numberOfPacketsNeeded; i++){
        //if you run out of space, get outta here
        PACKET_STATUS download_status = downloadDataPacket(flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE);
        FLASH_WRITE_RESULT write_status= update_flash_data(flash_write_location,flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE,spritesheet);
        switch(download_status){
            case READY_TO_CONTINUE:
                flash_write_location += TAMO_SERIAL_BUFFER_SIZE;
                continue;
            case FLASH_FULL:
            case NO_DATA_RECEIVED:
            default:
                return false;
        }
    }
    //if it's not a clean multiple of 64, grab the rest of it
    if(dataSize%TAMO_SERIAL_BUFFER_SIZE){
        PACKET_STATUS download_status = downloadDataPacket(flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE);
        FLASH_WRITE_RESULT write_status= update_flash_data(flash_write_location,flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE,spritesheet);
        switch(download_status){
            case READY_TO_CONTINUE:
                break;
            case FLASH_FULL:
            case NO_DATA_RECEIVED:
            default:
                return false;
        }
    }
    return true;
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

//Note about the exchange functions:
/*
These are kind of delicate because they send data by filling
the serial buffer completely, and don't check if there's not enough data to do so

if the sprite data WASN'T a multiple of TAMO_SERIAL_BUFFER_SIZE, this would be a problem!

*/


//waits for a request byte, then sends a packet of that size
uint8_t sendDataPacket(uint8_t * buffer, uint32_t& flash_read_location){
    //wait for go ahead
    if(!idleSerial())
        return 0;
    uint8_t command[2] = {0};
    Serial.readBytes(command,2);
    if(command[0] != REQUEST_NEXT_DATA_PACKET)
        return 0;
    uint8_t size = command[1];

    //send your data
    //fill read buffer
    memset(buffer,0,size);
    for(uint8_t i = 0; i<size; i++){
        buffer[i] = spritesheet[flash_read_location+i];
    }
    Serial.write(buffer,size);
    return size;
}

// void requestSpriteExchange(){

// }

//run the sprite swap routine as the leader
bool exchangeSprites(bool leader){

    SPRITE_IDENTITY otherSpriteID = NO_IDENTITY;

    //this tamo is starting it, send sprite_swap command and wait
    if(leader){
        Serial.write(INITIATE_SPRITE_SWAP);
        //wait for response
        if(!idleSerial())
            return false;
        uint8_t swapResponse[2] = {0,0};
        Serial.readBytes(swapResponse,2);
        //if response wasn't "ready for sprite swap", just return
        if(swapResponse[0] != SPRITE_SWAP_READY)
            return false;
        
        otherSpriteID = SPRITE_IDENTITY(swapResponse[1]);
    }

    //tell the other tamo you're ready, and your identity
    uint8_t startSwap[2] = {SPRITE_SWAP_READY,tamo.identity};
    Serial.write(startSwap,2);

    //wait for the leader to respond & tell you its identity
    if(!leader){
        //wait for response
        if(!idleSerial())
            return false;
        uint8_t swapResponse[2] = {0,0};
        Serial.readBytes(swapResponse,2);
        //if response wasn't "ready for sprite swap", just return
        if(swapResponse[0] != SPRITE_SWAP_READY)
            return false;
        
        otherSpriteID = SPRITE_IDENTITY(swapResponse[1]);
    }

    if(leader){
        Serial.write(REQUEST_NEXT_DATA_PACKET);
    }

    //walk sprite offscreen
    tamo.walkOff();
    
    //if either tamo has a custom sprite active, run this
    if(tamo.identity == CUSTOM_SPRITE || otherSpriteID == CUSTOM_SPRITE){
        //it's always this
        uint32_t flash_read_location = custom_sprite_idle_1;
        while(flash_read_location < (custom_sprite_eating_2)){
            uint8_t size_of_packet_sent = 0;
            //switch send/receive order based on who's the leader
            if(leader){
                PACKET_STATUS received = NO_DATA_RECEIVED;
                //receive sprite data
                if(otherSpriteID == CUSTOM_SPRITE){
                    received = downloadDataPacket(flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE);
                }
                //send sprite data
                if(tamo.identity == CUSTOM_SPRITE){
                    size_of_packet_sent = sendDataPacket(flash_read_buffer,flash_read_location);
                }
                //only update your own flash data AFTER you sent your sprite data! 
                if(otherSpriteID == CUSTOM_SPRITE && received != NO_DATA_RECEIVED){
                    update_flash_data(flash_read_location,flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE,spritesheet);
                    //if you're not sending data, make sure to still increment your write loc!
                    if(tamo.identity != CUSTOM_SPRITE){
                        flash_read_location += TAMO_SERIAL_BUFFER_SIZE;
                    }
                }
            }
            else{
                //send sprite data
                if(tamo.identity == CUSTOM_SPRITE){
                    size_of_packet_sent = sendDataPacket(flash_read_buffer,flash_read_location);
                }
                //receive sprite data
                if(otherSpriteID == CUSTOM_SPRITE ){
                    PACKET_STATUS received = downloadDataPacket(flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE);
                    //only update your own flash data AFTER you sent your sprite data! 
                    if(received != NO_DATA_RECEIVED){
                        update_flash_data(flash_read_location,flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE,spritesheet);
                        //if you're not sending data, make sure to still increment your write loc!
                        if(tamo.identity != CUSTOM_SPRITE){
                            flash_read_location += TAMO_SERIAL_BUFFER_SIZE;
                        }
                    }
                }
            }

            //increment your write location
            flash_read_location += size_of_packet_sent;
        }
    }
    //set tamo identity
    tamo.setIdentity(otherSpriteID);
    tamo.setMoodSprite();

    //walk sprite back onscreen
    tamo.walkOn();

    Serial.write(TAMO_DISCONNECT);
    Serial.write(TAMO_DISCONNECT);

    return true;
}

bool uploadSpriteToSerial(uint32_t flash_read_location){
    //tell web designer you're ready
    Serial.write(EXPORT_SPRITE_READY);

    while(flash_read_location <= (custom_sprite_eating_2)){
        //if you don't get anything back, something's wrong
        uint8_t size_of_packet_sent = sendDataPacket(flash_read_buffer,flash_read_location);
        if(size_of_packet_sent == 0)
            return false;
        flash_read_location += size_of_packet_sent;
    }
    return true;
}

//checks to see if board is connected
bool checkSerialConnection(){
    //return immediately if nothing to read
    if(!Serial.available())
        return false;
    
    uint8_t firstByte[1] = {0};
    Serial.readBytes(firstByte,1);
    switch(firstByte[0]){

        //download a sprite from the web
        case WEBSERIAL_SPRITE_UPLOAD:
            //send hello to web
            Serial.write(TAMO_HELLO);
            tamo.walkOff();
            if(loadDataFromSerial(custom_sprite_idle_1)){
                //tell the browser you're done
                Serial.write(TAMO_DISCONNECT);
                Serial.write(TAMO_DISCONNECT);//twice for browser compatibility...v silly

                //set tamo's identity to the custom sprite
                tamo.setIdentity(CUSTOM_SPRITE);
                tamo.setMoodSprite();

                tamo.walkOn();
                return true;
            }
            tamo.walkOn();
            break;

        //sprite swap as the follower
        case INITIATE_SPRITE_SWAP:
            exchangeSprites(false);
            break;
        case EXPORT_SPRITE:
            uploadSpriteToSerial(custom_sprite_idle_1);
            break;
        case SET_IDENTITY:{
            uint8_t secondByte[1] = {0};
            if(!idleSerial())
                return;
            Serial.readBytes(secondByte,1);
            tamo.setIdentity(SPRITE_IDENTITY(secondByte[0]));
            tamo.setMoodSprite();

            return true;
        }
        case SET_MODE:{
            uint8_t secondByte[1] = {0};
            if(!idleSerial())
                return;
            Serial.readBytes(secondByte,1);
            EEPROM.update(EEPROM_MODE_ADDR,secondByte[0]);
            //reset the system, do a lil dance first
            blinkForABit(10,10);
            _PROTECTED_WRITE(RSTCTRL.SWRR,1);
            return true;
        }
        case WEBSERIAL_SLIDESHOW_UPLOAD:

            uint8_t settingsBytes[4] = {0};
            if(!awaitBytes(settingsBytes,4)){
                return false;
            }
            EEPROM.update(EEPROM_SLIDESHOW_FRAME_COUNT_ADDR,settingsBytes[0]);
            EEPROM.update(EEPROM_SLIDESHOW_SPEED_ADDR,settingsBytes[1]);
            EEPROM.update(EEPROM_SLIDESHOW_SLEEP_TIME_ADDR,settingsBytes[2]);
            EEPROM.update(EEPROM_SLIDESHOW_BLINK_TIME_ADDR,settingsBytes[3]);

            //send hello to web
            Serial.write(TAMO_HELLO);

            if(loadDataFromSerial(slideshow_frame_0)){
                Serial.write(TAMO_DISCONNECT);
                Serial.write(TAMO_DISCONNECT);//twice for browser compatibility...v silly

                //set tamo's mode to slideshow mode
                EEPROM.update(EEPROM_MODE_ADDR,SLIDESHOW);

                //reset the system, do a lil dance first
                blinkForABit(10,10);
                _PROTECTED_WRITE(RSTCTRL.SWRR,1);
                return true;
            }
            break;
        default:
            break;
    }
    
    return false;
}