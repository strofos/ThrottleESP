#include "Z21Protocol.h"


void sendZ21(uint8_t* data, uint16_t len)
{
  extern WifiState wifiState;
  if (wifiState != WIFI_CONNECTED) return;

  extern WiFiUDP udp;
  udp.beginPacket(z21_ip, z21_port);
  udp.write(data, len);
  udp.endPacket();
}

void getZ21Commands() {  
  extern WifiState wifiState;
  if (wifiState != WIFI_CONNECTED) return;

  extern WiFiUDP udp;
  int size = udp.parsePacket();

  if (size)
  {
    uint8_t buf[100];
    udp.read(buf, size);
    parseZ21Response(buf, size);
  }
}

// ---------------- SET SPEED ----------------
void setSpeed(uint16_t addr, uint8_t speed, bool dir)
{
  uint8_t packet[10];

  packet[0] = 0x0A; packet[1] = 0x00;   // length
  packet[2] = 0x40; packet[3] = 0x00;   // X-Header

  packet[4] = 0xE4; // LAN_X_SET_LOCO_DRIVE
  packet[5] = 0x13; // DBO 0x1S S=0: DCC 14 speed steps...S=3: DCC 128 speed steps
  packet[6] = (0xc0 | (addr >> 8)); // DB1 - AdrMSB
  packet[7] = addr & 0xFF; // DB2 - AdrLSB
  packet[8] = speed | (dir ? 0x80 : 0x00); // DB3 - RVVVVVVV
  //Note: loco address = (Adr_MSB & 0x3F) << 8 + Adr_LSB 

  packet[9] = 0x00;
  for (int idx = 0; idx < 9; idx++)
    packet[9] ^= packet[idx];

  sendZ21(packet, 10);
}


// ---------------- SET FUNCTION ----------------
void setFunction(uint16_t addr, uint8_t func, bool on)
{
  uint8_t packet[10];

  packet[0] = 0x0A; packet[1] = 0x00; // DataLen
  packet[2] = 0x40; packet[3] = 0x00; // Header

  packet[4] = 0xE4; // LAN_X_SET_LOCO_FUNCTION
  packet[5] = 0xF8; // DB0
  packet[6] = (0xc0 | (addr >> 8)); // DB1 - AdrMSB
  packet[7] = addr & 0xFF; // DB2 - AdrLSB
  packet[8] = (on ? 0x40 : 0x00) + func; // DB3 - TTNN NNNN

  // Only one function is targeted, better to use LAN_X_SET_LOCO_FUNCTION_GROUP
  // NN NNNN - > Function index, 0x00=F0 (light), 0x01=F1 etc. F0 to F28 can be switched here

  packet[9] = 0x00;
  for (int idx = 0; idx < 9; idx++)
    packet[9] ^= packet[idx];

  sendZ21(packet, 10);
}

void setTrackPowerON()
{
  uint8_t packet[] = {
    0x07, 0x00,
    0x40, 0x00,
    0x21, 0x81, 0xa0   // LAN_X_SET_TRACK_POWER ON
  };

  sendZ21(packet, 7);
}

void setTrackPowerOFF()
{
  uint8_t packet[] = {
    0x07, 0x00,
    0x40, 0x00,
    0x21, 0x80, 0xa1   // LAN_X_SET_TRACK_POWER OFF
  };

  sendZ21(packet, 7);
}


void getLocoInfo(uint16_t addr)
{
  uint8_t packet[9];

  packet[0] = 0x09; packet[1] = 0x00; // DataLen
  packet[2] = 0x40; packet[3] = 0x00; // Header

  packet[4] = 0xE3;   // LAN_X_GET_LOCO_INFO
  packet[5] = 0xF0;   // DB0

  packet[6] = (0xc0 | (addr >> 8)); // DB1 - AdrMSB
  packet[7] = addr & 0xFF; // DB2 - AdrLSB

  packet[8] = 0x00;
  for (int idx = 0; idx < 8; idx++)
    packet[8] ^= packet[idx];

  sendZ21(packet, 9);
}

void getSerialInfo(){
 // Z21: LAN_GET_SERIAL_NUMBER
  uint8_t packet[] = {
    0x04, 0x00,   // length
    0x10, 0x00    // X-Header = System Info
  };

  //Serial.println("Z21 request serial number...");
  sendZ21(packet, 4);
}

void getStatus(){
 // Z21: LAN_X_GET_STATUS
  uint8_t packet[] = {
    0x07, 0x00,   // length
    0x40, 0x00,    // X-Header
    0x21, 0x24, 0x05
  };
  
  sendZ21(packet, 7);
}

void getSystemInfo(){
 // Z21: LAN_SYSTEMSTATE_GETDATA mV/C/mA
  uint8_t packet[] = {
    0x04, 0x00,   // length
    0x85, 0x00
  };
  
  sendZ21(packet, 4);
}


void printZ21Frame(uint8_t* buf, int size) {
  Serial.print("Z21: ");
  for (int i = 0; i < size; i++)
  {
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void parseZ21LocoInfo(uint8_t* buf, int size) {
  if (size < 12) return; // too small

  uint16_t addr = ((buf[5] & 0x3F) << 8) | buf[6];
  uint8_t speedSteps = buf[7] & 0x07;
  if (speedSteps == 0) speedSteps = 14;
  else if (speedSteps == 2) speedSteps = 28;
  else if (speedSteps == 4) speedSteps = 128;
  bool isSlaveController = ((buf[7] & 0x08) == 0x08);
  uint8_t speed = buf[8] & 0x7F;
  bool dir = ((buf[8] & 0x80) == 0x80);

  extern volatile bool locoFunctionsStates[10];
  uint8_t f0f4 = buf[9] & 0x1F;
  locoFunctionsStates[0] = ((f0f4 & 0x10) == 0x10);
  for (uint8_t idx = 0; idx < 4; idx++) {
    uint8_t maskValue = (1 << idx);
    locoFunctionsStates[1 + idx] = ((f0f4 & maskValue) == maskValue);
  }

  uint8_t f5fc = buf[10];
  for (uint8_t idy = 0; idy < 5; idy++) { // functions have a max value of 10
    uint8_t maskValue5c = (1 << idy);
    locoFunctionsStates[5 + idy] = ((f5fc & maskValue5c) == maskValue5c);
  }

  uint8_t fdfx = buf[11];

  // if (isSlaveController) Serial.print("LOK: ");
  // else Serial.print("LOK-M: ");
  // Serial.print(addr);
  // Serial.print(dir ? ",FWD: " : ",REV: ");
  // Serial.print(speed);
  // Serial.print(" / ");
  // Serial.print(speedSteps);
  // Serial.print(", F: ");
  // Serial.print(f0f4, BIN);
  // Serial.print(" ");
  // Serial.print(f5fc, BIN);
  // Serial.print(" ");
  // Serial.println(fdfx, BIN);
}

void parseZ21SystemInfo(uint8_t* buf, int size) {
  if (size < 12) return; // too small

  int16_t mainCurrent =
    ((int16_t)buf[5] << 8) | buf[4];

  int16_t progCurrent =
    ((int16_t)buf[7] << 8) | buf[6];

  int16_t filteredMainCurrent =
    ((int16_t)buf[9] << 8) | buf[8];

  int16_t temperature =
    ((int16_t)buf[11] << 8) | buf[10];

  uint16_t supplyVoltage =
    ((uint16_t)buf[13] << 8) | buf[12];

  uint16_t internalVoltage =
    ((uint16_t)buf[15] << 8) | buf[14];

  uint8_t centralState = buf[16];
  uint8_t centralStateEx = buf[17];
  uint8_t capabilities = buf[19];

  // Serial.println("===== Z21 SYSTEM STATE =====");

  // Serial.print("Main Current: ");
  // Serial.print(mainCurrent);
  // Serial.println(" mA");

  // Serial.print("Prog: ");
  // Serial.print(progCurrent);
  // Serial.print(" mA, ");

  // Serial.print("Main: ");
  // Serial.print(filteredMainCurrent);
  // Serial.print(" mA, ");

  // Serial.print("Temperature: ");
  // Serial.print(temperature);
  // Serial.print(" C, ");

  // Serial.print("Supply: ");
  // Serial.print(supplyVoltage / 1000.0f);
  // Serial.print(" V, ");

  // Serial.print("Track: ");
  // Serial.print(internalVoltage / 1000.0f);
  // Serial.println(" V");

  // Serial.print("CentralState: 0x");
  // Serial.println(centralState, HEX);

  // Serial.print("CentralStateEx: 0x");
  // Serial.println(centralStateEx, HEX);

  // Serial.print("Capabilities: 0x");
  // Serial.println(capabilities, HEX);

  // Serial.println("============================");
  // Serial.flush();

//             0 1  2 3  4 5 6 7 8 9 a  b  c  d  e  f 0 1 2  3
// Z21 FRAME: 14 0 84 0 19 0 0 0 9 0 2C 0 F2 4A C2 45 0 0 5 7B 

  // printZ21Frame(buf, size);
}

void parseZ21Response(uint8_t* buf, int size)
{
  if (size < 4) return;

  extern volatile uint8_t trackPower;

  uint8_t header = buf[2];
  uint8_t xheader = buf[3];

  // ---------------- TRACK STATUS ----------------
  if (header == 0x40 && buf[4] == 0x61)
  {
    if (buf[5] == 0x00)       {
      // Serial.println("TRACK POWER: OFF");
      trackPower = TRACK_OFF; // 0x00;
    }
    else if (buf[5] == 0x01)  {
      // Serial.println("TRACK POWER: ON");
      trackPower = TRACK_ON; // 0x01;
    }    
    else if (buf[5] == 0x02) {
      // Serial.println("TRACK PROGRAMMING MODE");
      trackPower = TRACK_PROG; // 0x02;
    } 
    else if (buf[5] == 0x08) {
      // Serial.println("TRACK SHORT CIRCUIT");
      trackPower = TRACK_ESTOP; // 0x08;
    }  
    //else if (buf[5] == 0x81)  Serial.println("TRACK POWER: STOP");
    else if (buf[5] == 0x82) {
      printZ21Frame(buf, size);
      //Serial.println("UNKNOWN COMMAND");
    } 
    else {
      printZ21Frame(buf, size);
      // /Serial.println("Not implemented");
    }
  }

  else if (header == 0x40 && buf[4] == 0x62) {
    //Serial.println("S TRACK POWER");
    if (buf[6] == 0x00) {
      //Serial.println("STRACK POWER: ON");
      trackPower = TRACK_ON;// 0x01;
    }
    else if ((buf[6] & 0x01) == 0x01) { 
      //Serial.println("STRACK POWER: ESTOP");
      trackPower = TRACK_ESTOP; // 0x03;
    }
    else if ((buf[6] & 0x02) == 0x02) { 
      //Serial.println("STRACK POWER: OFF");
      trackPower = TRACK_OFF; // 0x00;
    }
    else if ((buf[6] & 0x04) == 0x04) { 
      // Serial.println("STRACK SHORT CIRCUIT");
      trackPower = TRACK_SHORT; // 0x08;
    }
    else if ((buf[6] & 0x20) == 0x20) { 
      // Serial.println("STRACK PROGRAMMING MODE");
      trackPower = TRACK_PROG; // 0x02;
    }
  }

  else if (header == 0x84) {
    //Serial.println("parseZ21SystemInfo");
    delay(100); // add some time for more info to be loaded
    int additionalSize = udp.parsePacket();
    if (additionalSize > 0) {
      udp.read(buf + size, additionalSize);
    }
    parseZ21SystemInfo(buf, size + additionalSize);
  }

  else if (header == 0x10) {
    // Z21 FRAME: 8 0 10 0 3A D1 2 0 
    // Serial.print("System Serial: ");
    // Serial.print(buf[4]);
    // Serial.print(".");
    // Serial.print(buf[5]);
    // Serial.print(".");
    // Serial.println(buf[6]);
  }

  // ---------------- LOCOMOTIVE INFO ----------------
  else if (header == 0x40 && buf[4] == 0xEF)
  {
    //Serial.println("parseZ21LocoInfo");
    delay(100); // add some time for more info to be loaded
    int additionalSize = udp.parsePacket();
    if (additionalSize > 0) {
      udp.read(buf + size, additionalSize);
    }
    parseZ21LocoInfo(buf, size + additionalSize);    
  }

  else {
    printZ21Frame(buf, size);
    // Serial.println("BAD Packet/Header");
  }
}