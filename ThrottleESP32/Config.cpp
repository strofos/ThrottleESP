#include "Config.h"

WifiState wifiState = WIFI_IDLE;

const char* ssid = "UPCD22662E";
const char* password = "hTxwhrv3J4sc";

// const char* ssid = "DIGI_Pirvu";
// const char* password = "G4b1tz41";

const IPAddress z21_ip(192,168,0,111);
const uint16_t z21_port = 21105;

WiFiUDP udp;

volatile uint8_t trackPower = TRACK_OFF; // 0 = off, 1 = on, 2 = prog, 8 = short, 

// locomotivă DCC
uint16_t locoAddr = 3;
uint8_t  locoSpeed = 0;
int8_t   locoDirection = LOK_STOP; // 1 = forward

