#include <Arduino.h>
#include "Config.h"
#include "TCPCommands.h"
#include <WiFi.h>
#include <WiFiClient.h>


WiFiServer server(TCP_COMMANDS_PORT);
WiFiClient client;
String tcpBuffer;
bool serverIsStarted = false;

void setupTCPCommmands() {
  tcpBuffer = "";
}

void parseTCPCommands() 
{
  if (!serverIsStarted) {
    extern WifiState wifiState;
    if (wifiState == WIFI_CONNECTED) {
      server.begin();
      serverIsStarted = true;
    }
  }

  if (!client || !client.connected()) {
    client = server.available();
    return;
  }

  unsigned clientTimeout = millis() + 300;
  while (client.available() && clientTimeout > millis()) {
    char c = client.read();

    if (c == '\n' || c == '>') {

      // we parse only packets like:
      // <ping>          -> returns <ping:[IPADDRESS]>
      // <wifi ssi="MyWifi" pass="MyPass"> -> returns <ok>
      // <loco adr="45"> -> lock the loco address to 45
      // <loco adr="*">  -> unlock the loco address
      // <lock>          -> locks the maus, no action can be done
      // <unlock>        -> unlocks/frees the maus; also <loco> commands can be used
      //parsePacket(buffer);

      if (tcpBuffer.indexOf("<loco adr=") >= 0) {
        parseTCPLocoAddress();
      }
      else if (tcpBuffer.indexOf("<lock") >= 0) {
        extern bool     throttleLocked;
        throttleLocked = true;
        tcpBuffer = "";
      }
      else if (tcpBuffer.indexOf("<unlock") >= 0) {
        extern bool     throttleLocked;
        throttleLocked = false;
        tcpBuffer = "";
      }
      
      else {
        Serial.println("IGNORED: " + tcpBuffer);
        tcpBuffer = "";
      }
  
    } else {
      tcpBuffer += c;
    }
  }
}

void parseTCPLocoAddress(){
  extern uint16_t locoAddr;
  extern bool     locoLocked;
  extern bool     throttleLocked;

  int start = tcpBuffer.indexOf("loco adr=");
  if (start < 0) {
    tcpBuffer = "";
    return;
  }
  start = tcpBuffer.indexOf('"');
  int end = tcpBuffer.indexOf('"', start + 1);

  throttleLocked = false;

  if (start != -1 && end != -1) {

    String value = tcpBuffer.substring(start + 1, end);

    if (value == "*") {
      // UNLOCK
      locoLocked = false;
      //locoAddr = -1;

      Serial.println("LOCO UNLOCKED");
    }
    else {
      // LOCK
      locoLocked = true;
      locoAddr = value.toInt();

      Serial.print("LOCO LOCKED: ");
      Serial.println(locoAddr);
    }
  }

  tcpBuffer = "";
}


void startWifiSetupOverHTTP(){
  // close the TCP server
  // stop the wifi
  // start the wifi in AP MODE
  // handle POST parameters
}

void parseWifiSetupOverHTTP() {

}