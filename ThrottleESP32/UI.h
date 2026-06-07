#ifndef UI_H
#define UI_H

typedef void (*KeyHandler)(char);

extern KeyHandler onShortKeyHandler;
extern KeyHandler onLongKeyHandler;

#define UI_THROTTLE    0
#define UI_MENU        1
#define UI_LOK_ADDRESS 2
#define UI_WIFI_SELECT 3
#define UI_WIFI_PASS   4 

char scanKeypad();
int8_t keyToIndex(char k);
void parseKeyPress();
void setupUI();
void drawUI();

void drawUIMenu();
void drawUIThrottle();
void drawUIFunctions();
void drawUILokAddress();
void drawUIWifiSelect();
void showUIKeysTest();

bool isUIUsingWifi();
// void showUILokConfig();
// void showUITopTrack();

void onShortKeyPress(char k);
void onLongKeyPress(char k);

void testUI();

#endif