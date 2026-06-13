#include "Config.h"
#include "ThrottleWifi.h"
#include <Preferences.h>

unsigned long wifiTimer = 0;
unsigned long wifiTimeout = 0;

Preferences prefs;

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
        extern char ssid[33];
        extern char password[65];

        WiFi.begin((const char*)ssid, (const char*)password);

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


void loadWifiConfig()
{
  extern char ssid[33];
  extern char password[65];

  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));

  prefs.begin("wifi", true); // read-only

  String s = prefs.getString("ssid", ssid);
  String p = prefs.getString("pass", password);

  strncpy(ssid, s.c_str(), sizeof(ssid) - 1);
  ssid[sizeof(ssid) - 1] = '\0';

  strncpy(password, p.c_str(), sizeof(password) - 1);
  password[sizeof(password) - 1] = '\0';

  prefs.end();
}

void saveWifiConfig(const char* newSsid, const char* newPass)
{
  extern char ssid[33];
  extern char password[65];

  if (sizeof(newSsid) > 33 || sizeof(newPass) > 65) return;

  prefs.begin("wifi", false);

  prefs.putString("ssid", newSsid);
  prefs.putString("pass", newPass);

  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));
  
  strncpy(ssid, newSsid, sizeof(newSsid) - 1);
  strncpy(password, newPass, sizeof(newPass) - 1);

  prefs.end();
}

