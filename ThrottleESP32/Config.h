#ifndef CONFIG_H
#define CONFIG_H
#include <WiFi.h>
#include <WiFiUdp.h>

#define LCD_RST   22
#define LCD_CE    21
#define LCD_DC    19
#define LCD_DIN   18
#define LCD_CLK    5

#define POT_PIN   34
#define PWR_PIN   35

// C2 R1 C1 R4 C3 R3 R2
#define COL1      27
#define COL2      12
#define COL3      25
#define ROW1      14
#define ROW2      32
#define ROW3      33
#define ROW4      26

#define LOK_FORWARD  1
#define LOK_STOP     0
#define LOK_REVERSE -1

#define TRACK_OFF   0
#define TRACK_ON    1
#define TRACK_PROG  2
#define TRACK_ESTOP 3
#define TRACK_SHORT 8 

enum WifiState
{
  WIFI_IDLE,
  WIFI_RESET_1,
  WIFI_RESET_2,
  WIFI_RESET_3,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_FAILED
};

extern WifiState wifiState;

extern char ssid[33];
extern char password[65];

extern const IPAddress z21_ip;
extern const uint16_t z21_port;

extern WiFiUDP udp;

extern volatile uint8_t  trackPower;
extern uint16_t locoAddr;
extern bool     locoLocked;
extern uint8_t  locoSpeed;
extern int8_t   locoDirection;
extern volatile bool locoFunctionsStates[10];

#endif