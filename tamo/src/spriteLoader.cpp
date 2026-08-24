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

#define TAMO_SERIAL_BUFFER_SIZE 64
#define SERIAL_IDLE_TIME 1000 //1s

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

#include "debug/numbers.cpp";

bool awaitBytes(uint16_t size, uint8_t * buffer){
    // oled.fill(size);
    drawNumber(size);
    //wait for web response
    lastTime = millis();
    //wait for connection to timeout
    // if(!idleSerial(10))
    //     return false;

    //read in the sprite data
    // return (Serial.readBytes(buffer,size) == size);
    Serial.readBytes(buffer,size);
    return true;
}

// downloads a packet and writes 
PACKET_STATUS downloadDataPacket(uint16_t size, uint32_t &flash_write_location, uint8_t * buffer){
    //clear out buffer
    memset(buffer, 0x00, size);

    //request the next data packet
    Serial.write(REQUEST_NEXT_DATA_PACKET);
    //ask for this many bytes
    Serial.write(size);
    Serial.flush();

    if(!awaitBytes(size,buffer)){
        digitalWrite(LED_A,HIGH);
        digitalWrite(LED_B,HIGH);
        return NO_DATA_RECEIVED;
    }
    switch(update_flash_data(flash_write_location,buffer,size,spritesheet)){
        case WRITE_OK:
            break; 
        case NOT_A_PAGE_BOUNDARY:
            break;
        case ADDRESS_IN_BOOT_SECTION:
            break;
        case ADDRESS_BEYOND_FLASH:
            break;
        case WRITE_ERROR:
            break;
        default:
            break;
    }
    flash_write_location += size;
    return READY_TO_CONTINUE;
}

// main receiving function for saying hello & grabbing data
bool loadDataFromSerial(uint32_t flash_write_location){

    //wait for web response
    if(!idleSerial())
        return false;
    
    //get the size of the data
    uint8_t packetSize[2] = {0};
    Serial.readBytes(packetSize,2);

    uint16_t dataSize = (((uint16_t)packetSize[0])<<8)|((uint16_t)packetSize[1]);
    uint16_t numberOfPacketsNeeded = dataSize/TAMO_SERIAL_BUFFER_SIZE;

    //get sprite data in 64-byte packets, since that's the Serial buffer's size
    for(uint8_t i = 0; i<numberOfPacketsNeeded; i++){
        //if you run out of space, get outta here
        PACKET_STATUS status = downloadDataPacket(TAMO_SERIAL_BUFFER_SIZE,flash_write_location,flash_write_buffer);
        switch(status){
            case READY_TO_CONTINUE:
                continue;
            case FLASH_FULL:
            case NO_DATA_RECEIVED:
            default:
                return false;
        }
    }
    //if it's not a clean multiple of 64, grab the rest of it
    if(dataSize%TAMO_SERIAL_BUFFER_SIZE){
        //and if you run out of space here, get outta here
        PACKET_STATUS status = downloadDataPacket(dataSize%TAMO_SERIAL_BUFFER_SIZE,flash_write_location,flash_write_buffer);
        switch(status){
            case READY_TO_CONTINUE:
                break;
            case FLASH_FULL:
            case EEPROM_FULL:
                return true;
            case NO_DATA_RECEIVED:
            default:
                // blinkForABit(500,4);
                return false;
        }
    }

    //tell the browser you're done
    Serial.write(TAMO_DISCONNECT);
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

void clearSerialBuffer(){
    while(Serial.available()){
        Serial.read();
    }
}


//Note about the exchange functions:
/*
These are kind of delicate because they send data by filling
the serial buffer completely, and don't check if there's not enough data to do so

if the sprite data WASN'T a multiple of TAMO_SERIAL_BUFFER_SIZE, this would be a problem!

*/
bool exchangeSprites_follower(){

    uint32_t flash_read_location = custom_sprite_idle_1;

    //tell other tamo you're ready
    Serial.write(SPRITE_SWAP_READY);

    while(flash_read_location <= (custom_sprite_eating_2)){
        //if you don't get anything back, something's wrong
        if(!idleSerial())
            return false;

        //check to make sure leader is asking for a data chunk
        uint8_t firstByte[1] = {0};
        Serial.readBytes(firstByte,1);
        if(firstByte[0] != REQUEST_NEXT_DATA_PACKET)
            return false;
        clearSerialBuffer();
        //send your data to the leader
        //fill read buffer
        for(uint8_t i = 0; i<TAMO_SERIAL_BUFFER_SIZE; i++){
            flash_read_buffer[i] = spritesheet[flash_read_location+i];
        }
        //send data
        Serial.write(flash_read_buffer,TAMO_SERIAL_BUFFER_SIZE);
        if(!awaitBytes(TAMO_SERIAL_BUFFER_SIZE,flash_write_buffer))
            return false;

        //write data to flash
        update_flash_data(flash_read_location,flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE,spritesheet);

        flash_read_location += TAMO_SERIAL_BUFFER_SIZE;
    }
    return true;
}

//run the sprite swap routine as the leader
bool exchangeSprites_leader(){
    //it's always this
    uint32_t flash_read_location = custom_sprite_idle_1;

    //this tamo is starting it, send sprite_swap command and wait
    Serial.write(INITIATE_SPRITE_SWAP);
    //wait for response
    if(!idleSerial())
        return false;
    uint8_t firstByte[1] = {0};
    Serial.readBytes(firstByte,1);
    //if response wasn't "ready for sprite swap", just return
    if(firstByte[0] != SPRITE_SWAP_READY)
        return false;
    
    while(flash_read_location <= (custom_sprite_eating_2)){
        //empty out input buffer
        clearSerialBuffer();
        //get data from follower
        Serial.write(REQUEST_NEXT_DATA_PACKET);
        if(!awaitBytes(TAMO_SERIAL_BUFFER_SIZE,flash_write_buffer))
            return false; //return false if you didn't get enough data! this will be a huge prob, since sprites will be partially overwritten, but oh well
        //future solution is writing received data to a totally diff location in flash, then overwriting at the end. Idk if there's enough flash space for that though
        
        //send your data to the follower
        //fill read buffer
        for(uint8_t i = 0; i<TAMO_SERIAL_BUFFER_SIZE; i++){
            flash_read_buffer[i] = spritesheet[flash_read_location+i];
        }
        Serial.write(flash_read_buffer,TAMO_SERIAL_BUFFER_SIZE);

        //write data to flash
        update_flash_data(flash_read_location,flash_write_buffer,TAMO_SERIAL_BUFFER_SIZE,spritesheet);

        //increment your write location
        flash_read_location += TAMO_SERIAL_BUFFER_SIZE;
    }
    return true;
}

bool uploadSpriteToSerial(uint32_t flash_read_location){
    //tell web designer you're ready
    Serial.write(EXPORT_SPRITE_READY);

    while(flash_read_location <= (custom_sprite_eating_2)){
        //if you don't get anything back, something's wrong
        if(!idleSerial())
            return false;

        //check to make sure leader is asking for a data chunk
        uint8_t firstByte[1] = {0};
        Serial.readBytes(firstByte,1);
        if(firstByte[0] != REQUEST_NEXT_DATA_PACKET)
            return false;

        //send data to the webserial port
        //fill read buffer
        for(uint8_t i = 0; i<TAMO_SERIAL_BUFFER_SIZE; i++){
            flash_read_buffer[i] = spritesheet[flash_read_location+i];
        }
        //send data
        Serial.write(flash_read_buffer,TAMO_SERIAL_BUFFER_SIZE);

        flash_read_location += TAMO_SERIAL_BUFFER_SIZE;
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
            if(loadDataFromSerial(custom_sprite_idle_1)){
                Serial.write(TAMO_DISCONNECT);

                //set tamo's identity to the custom sprite
                EEPROM.update(EEPROM_IDENTITY_ADDR,CUSTOM_SPRITE);
                EEPROM.update(EEPROM_MODE_ADDR,NORMAL_TAMO);

                //reset the system, do a lil dance first
                // blinkForABit(50,10);
                _PROTECTED_WRITE(RSTCTRL.SWRR,1);
                return true;
            }
            break;

        //sprite swap
        case INITIATE_SPRITE_SWAP:
            exchangeSprites_follower();
            break;
        case EXPORT_SPRITE:
            uploadSpriteToSerial(custom_sprite_idle_1);
            break;
        case SET_IDENTITY:{
            uint8_t secondByte[1] = {0};
            if(!idleSerial())
                return;
            Serial.readBytes(secondByte,1);
            EEPROM.update(EEPROM_IDENTITY_ADDR,secondByte[0]);
            //reset the system, do a lil dance first
            // blinkForABit(50,10);
            _PROTECTED_WRITE(RSTCTRL.SWRR,1);
            return true;
        }
        case SET_MODE:{
            uint8_t secondByte[1] = {0};
            if(!idleSerial())
                return;
            Serial.readBytes(secondByte,1);
            EEPROM.update(EEPROM_MODE_ADDR,secondByte[0]);
            //reset the system, do a lil dance first
            // blinkForABit(50,10);
            _PROTECTED_WRITE(RSTCTRL.SWRR,1);
            return true;
        }
        case WEBSERIAL_SLIDESHOW_UPLOAD:

            uint8_t settingsBytes[4] = {0};
            if(!awaitBytes(4,settingsBytes)){
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

                //set tamo's mode to slideshow mode
                EEPROM.update(EEPROM_MODE_ADDR,SLIDESHOW);

                //reset the system, do a lil dance first
                blinkForABit(50,10);
                _PROTECTED_WRITE(RSTCTRL.SWRR,1);
                return true;
            }
            break;
        default:
            break;
    }
    
    return false;
}