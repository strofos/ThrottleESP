#include "Config.h"
#include "ThrottleWifi.h"

unsigned long wifiTimer = 0;
unsigned long wifiTimeout = 0;

void startWifiConnect()
{
  extern WifiState wifiState;
  wifiState = WIFI_RESET_1;
  wifiTimer = millis();

  Serial.println("Starting WiFi...");
}

void checkWifiConnection()
{
  extern WifiState wifiState;
  switch (wifiState)
  {
    case WIFI_IDLE:
      wifiState = WIFI_RESET_1;
      break;

    case WIFI_RESET_1:
      WiFi.disconnect(true, true);
      wifiTimer = millis() + 500;
      wifiState = WIFI_RESET_2;
      break;

    case WIFI_RESET_2:
      if (millis() < wifiTimer)
        break;

      WiFi.mode(WIFI_OFF);
      wifiTimer = millis() + 500;
      wifiState = WIFI_RESET_3;
      break;

    case WIFI_RESET_3:
      if (millis() < wifiTimer)
        break;

      WiFi.mode(WIFI_STA);
      wifiTimer = millis() + 500;
      wifiState = WIFI_CONNECTING;
      break;

    case WIFI_CONNECTING:
      if (wifiTimer > millis())
        break;

      WiFi.setSleep(false);

      static bool started = false;
      if (!started)
      {
        extern const char* ssid;
        extern const char* password;

        WiFi.begin(ssid, password);

        Serial.print("WiFi");
        wifiTimeout = millis() + 5000;
        wifiTimer = millis() + 500;

        started = true;
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("\nWiFi OK");
        Serial.println(WiFi.localIP());

        wifiState = WIFI_CONNECTED;
        started = false;

        extern WiFiUDP udp;
        udp.begin(21105);
      }
      else if (millis() > wifiTimeout)
      {
        Serial.println("\nWiFi timeout");

        wifiState = WIFI_FAILED;
        started = false;
      }
      else if (millis() > wifiTimer)
      {
        Serial.print(".");
        wifiTimer = millis() + 500;
      }
      break;

    case WIFI_CONNECTED:
      break;

    case WIFI_FAILED:
      wifiState = WIFI_RESET_1;
      break;
  }
}