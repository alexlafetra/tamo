#pragma once

bool idleSerial();
bool awaitBytes(uint8_t * buffer,uint16_t size);
// downloads a packet and writes 
// PACKET_STATUS downloadDataPacket(uint16_t size, uint16_t &EEPROM_write_location, uint8_t * buffer);
// main receiving function for saying hello & grabbing data
//checks to see if board is connected
bool checkSerialConnection();
bool exchangeSprites(bool);
