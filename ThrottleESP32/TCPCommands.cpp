#include <Arduino.h>
#include "Config.h"
#include "TCPCommands.h"
#include <WiFi.h>
#include <WiFiClient.h>


WiFiServer server(8983);
WiFiClient client;
String tcpBuffer = "";

void setupTCPCommmands() {
}

void parseTCPCommands() 
{
  if (!client || !client.connected()) {
    client = server.available();
    return;
  }

  unsigned clientTimeout = millis() + 300;
  while (client.available() && clientTimeout > millis()) {
    char c = client.read();

    if (c == '\n') {

      if (tcpBuffer.startsWith("loco adr=")) {
        parseTCPLocoAddress();
        // we parse only packets like:
        // <ping>          -> returns <ping:[IPADDRESS]>
        // <wifi ssi="MyWifi" pass="MyPass"> -> returns <ok>
        // <loco adr="45"> -> lock the loco address to 45
        // <loco adr="*">  -> unlock the loco address
        // <lock>          -> locks the maus, no action can be done
        // <unlock>        -> unlocks/frees the maus; also <loco> commands can be used
        //parsePacket(buffer);
      }
      else {
        //Serial.println("IGNORED: " + buffer);
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

  int start = tcpBuffer.indexOf('"');
  int end = tcpBuffer.indexOf('"', start + 1);

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