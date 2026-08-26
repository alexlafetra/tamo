#pragma once

bool idleSerial();
bool awaitBytes(uint16_t size, uint8_t * buffer);
// downloads a packet and writes 
// PACKET_STATUS downloadDataPacket(uint16_t size, uint16_t &EEPROM_write_location, uint8_t * buffer);
// main receiving function for saying hello & grabbing data
bool receiveSpriteOverSerial();

//checks to see if board is connected
bool checkSerialConnection();

bool exchangeSprites_leader();