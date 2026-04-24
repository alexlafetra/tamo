#pragma once

void idleSerial();
void awaitBytes(uint16_t size, uint8_t * buffer);
// downloads a packet and writes 
bool downloadSpritePacket(uint16_t size, uint16_t &EEPROM_write_location, uint8_t * buffer);
// main receiving function for saying hello & grabbing data
void receiveSpriteOverSerial();

//checks to see if board is connected
bool checkSerialConnection();