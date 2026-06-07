#ifndef Z21_PROTOCOL_H
#define Z21_PROTOCOL_H
#include <Arduino.h>
#include "Config.h"

void sendZ21(uint8_t* data, uint16_t len);
void setSpeed(uint16_t addr, uint8_t speed, bool dir);
void setFunction(uint16_t addr, uint8_t func, bool on);

void setTrackPowerON();
void setTrackPowerOFF();

void getZ21Commands();

void getLocoInfo(uint16_t addr);
void getSerialInfo();
void getStatus();
void getSystemInfo();

void printZ21Frame(uint8_t* buf, int size);
void parseZ21LocoInfo(uint8_t* buf, int size);
void parseZ21SystemInfo(uint8_t* buf, int size);
void parseZ21Response(uint8_t* buf, int size);

#endif