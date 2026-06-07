#include <WiFi.h>
#include <WiFiUdp.h>
#include "Config.h"
#include "Z21Protocol.h"
#include "UI.h"
#include "ThrottleWifi.h"


// ---------------- SETUP ----------------
void setup()
{
  Serial.begin(115200);

  startWifiConnect();

  setupUI();

  getStatus();

  delay(1000);
}

// ---------------- LOOP ----------------
unsigned long cmdCooloff = 0;
uint8_t cmdFunctionIndex = 0;
unsigned long cmdFunctionCooloff = 0;
unsigned long cmdLokoInfoCooloff = 0;
unsigned long cmdStatusInfoCooloff = 0;

void loop()
{
  // testUI();
  // return;

  if (!isUIUsingWifi()) checkWifiConnection();

  parseKeyPress();

  getZ21Commands();

  drawUI();


  // should set speed and functions
  extern uint16_t locoAddr;
  extern uint8_t  locoSpeed;
  extern int8_t   locoDirection;
  extern volatile bool locoFunctionsStates[10];

  // async commands
  if (cmdCooloff < millis()) {
    cmdCooloff = millis() + 300;
    setSpeed(locoAddr, locoSpeed, locoDirection == LOK_FORWARD);
  }

  if (cmdFunctionCooloff < millis()) {
    setFunction(locoAddr, cmdFunctionIndex, locoFunctionsStates[cmdFunctionIndex]);
    cmdFunctionIndex++;
    if (cmdFunctionIndex >= 10) {
      cmdFunctionIndex = 0;
      cmdFunctionCooloff = millis() + 30000; // add a 30sec delay
    }
    else {
      cmdFunctionCooloff = millis() + 220;
    }
  }

  if (cmdLokoInfoCooloff < millis()) {
    getLocoInfo(locoAddr);
    cmdLokoInfoCooloff = millis() + 1030;
  }
  
  if (cmdStatusInfoCooloff < millis()) {
    getStatus();
    cmdStatusInfoCooloff = millis() + 1260;
  }

  //testSpeedSet();
  //testPowerOffOn();
  //getSystemInfo();
  //showUIKeysTest();
}

void testPowerOffOn() {
  if (trackPower == 0x01) {
    setTrackPowerOFF();
  }
  else {
    setTrackPowerON();
  }
}

void testSystemParams() {
  getSystemInfo();
}

void testSpeedSet()
{
  extern uint16_t locoAddr;
  extern uint8_t  locoSpeed;
  extern int8_t   locoDirection;

  // demo: creste viteza
  locoSpeed += 5;
  if (locoSpeed > 126) locoSpeed = 0;

  setSpeed(locoAddr, locoSpeed, locoDirection == LOK_FORWARD);

  // toggle lumina F0
  setFunction(locoAddr, 0, locoSpeed > 50);

  //getLocoInfo(locoAddr);

  //delay(500);
}

