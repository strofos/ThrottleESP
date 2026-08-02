#include "HTTPCommands.h"
#include "Config.h"
#include <WiFi.h>
#include <WebServer.h>
#include "ThrottleWifi.h"

// Server HTTP pentru configurare
WebServer configServer(HTTP_COMMANDS_PORT);

// Pagina HTML simplă
const char CONFIG_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>WiFi Setup</title>
</head>
<body>
  <h2>Configure WiFi</h2>
  <form method="POST" action="/save">
    SSID:<br>
    <input type="text" name="ssid"><br><br>
    Password:<br>
    <input type="password" name="pass"><br><br>
    <input type="submit" value="Save">
  </form>
</body>
</html>
)rawliteral";

void startWifiSetupOverHTTP()
{
  // 1. Oprește serverul TCP existent
  extern WiFiServer server;
  server.stop();

  // 2. Oprește conexiunea WiFi curentă
  WiFi.disconnect(true, true);   // șterge conexiunea și oprește STA
  delay(500);

  // 3. Pornește în Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_MODE_SSID, AP_MODE_PASS);

  Serial.print("AP SSID: ");
  Serial.println(WiFi.softAPSSID());
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // 4. Configurează serverul HTTP
  configServer.stop();

  configServer.on("/", HTTP_GET, []() {
    configServer.send_P(200, "text/html", CONFIG_PAGE);
  });

  configServer.on("/save", HTTP_POST, []() {
    if (!configServer.hasArg("ssid")) return;
    if (!configServer.hasArg("pass")) return;

    String newSSID = configServer.arg("ssid");
    String newPASS = configServer.arg("pass");

    saveWifiConfig(newSSID, newPASS);

    configServer.send(200, "text/html",
      "<h3>Settings saved.</h3><p>Device will restart WiFi.</p>");

    Serial.print("New SSID: ");
    Serial.println(newSSID);

    Serial.print("New PASS: ");
    Serial.println(newPASS);

    delay(1000);

    // Repornește în modul normal
    configServer.stop();

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);

    ESP.restart();
  });

  configServer.begin();

  Serial.println("HTTP configuration server started");
}


void parseWifiSetupOverHTTP() {
  configServer.handleClient();
}
