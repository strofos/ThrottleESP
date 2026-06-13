#include <sys/_types.h>
#include <Arduino.h>
#include "Config.h"
#include "UI.h"
#include "Z21Protocol.h"
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// display object (software SPI)
Adafruit_PCD8544 display(LCD_CLK, LCD_DIN, LCD_DC, LCD_CE, LCD_RST);
uint8_t uiState = UI_THROTTLE;

const char keys[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

volatile bool locoFunctionsStates[10] = {
  false, false, false, false, false,
  false, false, false, false, false
};
const char locoFunctionsLetters[10] = {
  'L', '1', '2', '3', '4',
  '5', '6', '7', '8', '9'
};


// 24x8 STOP
const unsigned char icon_stop [] PROGMEM = {
	0x0f, 0xff, 0xf0, 0x10, 0x00, 0x08, 0x27, 0x77, 0x64, 0x24, 0x25, 0x54, 
  0x23, 0x25, 0x74, 0x27, 0x27, 0x44, 0x10, 0x00, 0x08, 0x0f, 0xff, 0xf0
};

const unsigned char icon_short [] PROGMEM = {
  0x0C, 0x7F, 0x18, 0x1C, 0xFF, 0x9C, 0x1E, 0xDD, 0xBC, 0x00, 0x9C, 0x80,
  0x04, 0xFF, 0x90, 0x1E, 0xF7, 0xBC, 0x1C, 0x3E, 0x1C, 0x0C, 0x2A, 0x18
};

const unsigned char icon_prog [] PROGMEM = {
	0x1f, 0xff, 0xf0, 0x20, 0x00, 0x08, 0x4c, 0xee, 0xe4, 0x4a, 0xaa, 0x84, 
  0x4e, 0xca, 0xa4, 0x48, 0xae, 0xe4, 0x20, 0x00, 0x08, 0x1f, 0xff, 0xf0
};

KeyHandler onShortKeyHandler = nullptr;
KeyHandler onLongKeyHandler = nullptr;

int rowPins[4] = {ROW1, ROW2, ROW3, ROW4};
int colPins[3] = {COL1, COL2, COL3};


unsigned long pressTime = 0;
bool keyDown = false;
char lastKey = 0;

uint8_t tempLocoAddress = 0;

char scanKeypad()
{
  for (int r = 0; r < 4; r++)
  {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);

    for (int c = 0; c < 3; c++)
    {
      pinMode(colPins[c], INPUT_PULLUP);

      if (digitalRead(colPins[c]) == LOW)
      {
        delay(20); // debounce
        if (digitalRead(colPins[c]) == LOW)
        {
          return keys[r][c];
        }
      }
    }

    pinMode(rowPins[r], INPUT);
  }

  return 0;
}


void setupUI() {
  display.begin();

  // contrast (ajusteaza daca e prea intunecat / luminos)
  display.setRotation(2); // rotit 180 grade
  display.setContrast(90);
  display.invertDisplay(false);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(BLACK);

  // configurare pini tastaura
  for (int i = 0; i < 4; i++) {
    pinMode(rowPins[i], INPUT);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // configurare ADC
  analogReadResolution(12); // 0 - 4095
  analogSetPinAttenuation(POT_PIN, ADC_11db); // range complet 0-3.3V

  // alocate the default handlers from the UI
  onShortKeyHandler = onShortKeyPress;
  onLongKeyHandler = onLongKeyPress;
}


bool isUIUsingWifi() {
  // the menu is taking the wifi control
  return false;
}


void parseKeyPress() {
  char key = scanKeypad();
  // ---------------- KEY DOWN ----------------
  if (key && !keyDown)
  {
    keyDown = true;
    lastKey = key;
    pressTime = millis();
  }

  // ---------------- KEY UP ----------------
  if (!key && keyDown)
  {
    keyDown = false;

    unsigned long duration = millis() - pressTime;

    if (duration < 500)
    {
      if (onShortKeyHandler) onShortKeyHandler(lastKey);
    }
    else
    {
      if (onLongKeyHandler) onLongKeyHandler(lastKey);
    }
  }
    
  int raw = analogRead(POT_PIN);   // 0 - 4095
  extern uint8_t locoSpeed;
  extern int8_t  locoDirection;

  // setSpeed(locoAddr, locoSpeed, locoDirection == LOK_FORWARD);
  // setFunction(locoAddr, 0, locoSpeed > 50);

  int speed = map(raw, 0, 4095, -126, 126);
  if (-10 < speed && speed < 10) {
    locoSpeed = 0;
    locoDirection = LOK_STOP;
  }
  else if (speed < 0){ // this is actually forward
    locoSpeed = -speed;
    locoDirection = LOK_FORWARD;
  }
  else {
    locoSpeed = speed;
    locoDirection = LOK_REVERSE;
  }
}


void showUIKeysTest() {  
   int raw = analogRead(POT_PIN);   // 0 - 4095
  // mapare la DCC speed (0-126)
  int speed = map(raw, 0, 4095, -126, 126);
  speed = -speed;
  
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("SPEED:");

  display.setCursor(38, 0);
  display.print(speed);
  
  display.setCursor(0, 12);
  display.print("KEY:");
  display.setCursor(38, 12);
  display.print(lastKey);
  display.display();
}


void testUI(){  
  int startChar = 32;
  int totalChars = 256;
  int charsPerPage = 42; // Ajustat pentru a încăpea perfect pe ecran (6 linii x 7 caractere)

  for(int g = 124; g < totalChars; g++) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(String(g) + " ");
    display.write((char)g);
    display.display();
    delay(1000);
  }

//16, 17
//125,147157

  for (int i = startChar; i < totalChars; i += charsPerPage) {
    display.clearDisplay();
    display.setCursor(0, 0);


    // Randează un grup de caractere care încap pe ecran
    for (int j = i; j < i + charsPerPage && j < totalChars; j++) {
      // Afișăm caracterul corespunzător codului ASCII numeric
      display.write((char)j); 

      
      // Adăugăm un spațiu între caractere pentru lizibilitate
      display.print(" "); 
    }

    display.display(); // Trimite datele către ecran pentru a fi randate vizual
    delay(3000);       // Pauză de 3 secunde înainte de următoarea pagină
  }
}


void drawUIThrottle() {
  extern volatile uint8_t trackPower;
  extern uint16_t locoAddr;
  extern bool     locoLocked;
  extern uint8_t  locoSpeed;
  extern int8_t   locoDirection;

  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);

  // -------------------------
  // R1 (0..15)
  // -------------------------
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(String(locoAddr));

  // DIRECTION
  display.setTextSize(2);
  display.setCursor(32, 10);
  switch(locoDirection)
  {
    case LOK_REVERSE: display.write(17); display.write(17);  break;
    case LOK_FORWARD: display.write(16); display.write(16);  break;
    case LOK_STOP: display.write(220); display.write(220); break;//display.print("||"); break; 
    default: display.write(220); display.write(220); break;
  }

  // TRACK STATE
  switch(trackPower)
  {
    //case TRACK_OFF: 
    case TRACK_ON: display.fillRect(50, 0, 24, 8, WHITE); break;
    case TRACK_PROG: display.drawBitmap(50, 0, icon_prog, 24, 8, BLACK); break; //programming
    case TRACK_SHORT: display.drawBitmap(50, 0, icon_short, 24, 8, BLACK); break;
    default: display.drawBitmap(50, 0, icon_stop, 24, 8, BLACK); break; // OFF or ESTOP
  }

  // WifiState
  extern WifiState wifiState;
  display.setTextSize(1);
  display.setCursor(76, 0);
  if (wifiState == WIFI_CONNECTED) display.print("W");
  else display.print("_");
  
  // -------------------------
  // R4 (36..47)
  // -------------------------
  if (!locoLocked) {
    display.setCursor(0, 40);
    display.print("*MENU    #STOP");
  }

  drawUIFunctions();//15 e lumina

  display.display();
}


void drawUIFunctions() {
  // -------------------------
  // R3 (24..35)
  // -------------------------
  display.setTextSize(1);
  int functionCharWidth = 5;
  int functionCharWithSpacer = 8;

  for (int idx = 0; idx < 10; idx++) {

    int x = 1 + idx * functionCharWithSpacer;

    if (locoFunctionsStates[idx]) {
      // SELECTAT: fundal negru, text alb
      display.setTextColor(WHITE, BLACK);
      display.fillRect(x, 28, functionCharWithSpacer, 8, BLACK);
    } else {
      // NORMAL: fundal alb, text negru
      display.setTextColor(BLACK, WHITE);
      display.fillRect(x, 28, functionCharWithSpacer, 8, WHITE);
    }

    display.setCursor(x+1, 28);
    display.print(locoFunctionsLetters[idx]);
  }
}

void drawUIMenu() {  
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("1 LOK ADDRESS");
  display.setCursor(0, 10);
  display.print("2 WIFI");
  display.setCursor(0, 40);
  display.print("* EXIT");

  display.display();
}

void drawUILokAddress() {
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("ADDRESS:");
  display.setCursor(48, 0);
  display.print(tempLocoAddress);
  
  display.setCursor(0, 40);
  display.print("* OK  # CANCEL");

  display.display();
}


unsigned long wifiSelectCoolOff = 0;
void drawUIWifiSelect() {
  if (wifiSelectCoolOff > millis()) return;
  wifiSelectCoolOff = millis() + 60000;

  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Scanning...");
  display.display();
  
  WiFi.scanDelete(); // eliberează memoria
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  int n = WiFi.scanNetworks();

  if (n <= 0) {
    display.setCursor(0, 0);
    display.print("Lipsa retele!");
    display.display();
    return;
  }

  display.clearDisplay();
  int maxNetworks = min(n, 6);
  for(int idx = 0; idx < maxNetworks ;idx++) {
    display.setCursor(0, idx * 8);
    display.print(String(idx + 1) + " " + WiFi.SSID(idx).c_str());
  }

  display.display();
}


void drawUIWifiInfo() {
  if (wifiSelectCoolOff > millis()) return;
  wifiSelectCoolOff = millis() + 500;

  extern WifiState wifiState;
  extern char ssid[33];

  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(ssid);    
  
  display.setCursor(0, 10);
  if (wifiState == WIFI_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    display.print(ip[0]);
    display.print(".");
    display.print(ip[1]);
    display.print(".");
    display.print(ip[2]);
    display.print(".");
    display.print(ip[3]);
  }
  else if (wifiState == WIFI_CONNECTING) {
    display.print("Connecting...");
  }
  else if (wifiState == WIFI_RESET_1 || wifiState == WIFI_RESET_2 || wifiState == WIFI_RESET_3) {
    display.print("Reseting...");
  }
  else {
    display.print("idle/failed");
  }

  display.setCursor(0, 40);
  display.print("* OK  # CANCEL");
  
  display.display();
}

int8_t keyToIndex(char k) {
  if (k >= '0' && k <= '9')
    return k - '0';

  if (k == '*') return 10;
  if (k == '#') return 11;

  return -1;
}


void onShortKeyPress(char k){
  Serial.print("SHORT PRESS: ");
  Serial.println(k);

  extern uint16_t locoAddr;

  switch(uiState) {
    case UI_THROTTLE:
      if (k == '#' || k == '*') {
        // ignore
      }
      else {
        int functionIdx = keyToIndex(k);
        if (functionIdx < 0 || functionIdx >= 10 ) return;
        locoFunctionsStates[functionIdx] = !locoFunctionsStates[functionIdx];
        // send the loco function imediately
        setFunction(locoAddr, functionIdx, locoFunctionsStates[functionIdx]);
      }
      break;
    case UI_MENU:
      if (k == '1') {
        uiState = UI_LOK_ADDRESS;
        tempLocoAddress = 0;
      }
      else if (k == '2') uiState = UI_WIFI_INFO;
      else if (k == '*') uiState = UI_THROTTLE;
      break;
    case UI_LOK_ADDRESS:
      if (k == '*') { // OK
        locoAddr = tempLocoAddress;
        getLocoInfo(locoAddr);
        uiState = UI_THROTTLE;
      }
      else if (k == '#') { // CANCEL
        uiState = UI_THROTTLE;
      }
      else {
        int valueIdx = keyToIndex(k);
        if (valueIdx < 0 || valueIdx >= 10 ) return;
        tempLocoAddress = 10 * tempLocoAddress + valueIdx;
      }
      break;
    default:
      if (k == '*') uiState = UI_THROTTLE;
      break;
  }
}
void onLongKeyPress(char k) {
  Serial.print("LONG PRESS: ");
  Serial.println(k);

  extern volatile uint8_t trackPower;
  extern bool     locoLocked; 

  switch(uiState) {
    case UI_THROTTLE:
      if (k == '#') {
        if (locoLocked) return;

        if (trackPower == TRACK_ON)
          setTrackPowerOFF();
        else
          setTrackPowerON();
      
      }
      else if (k == '*') {
        if (locoLocked) return;

        uiState = UI_MENU;
      }
      else {
        onShortKeyPress(k);
      }
      break;
    default:
      onShortKeyPress(k);
      break;
  }
}


unsigned long uiCooloff = 0;
void drawUI() {
  // let the display refresh 1/3 seconds
  if (uiCooloff > millis()) return;
  uiCooloff = millis() + 300;

  extern bool locoLocked;
  if (locoLocked) {
    drawUIThrottle();
    return;
  }

  switch (uiState) {
    case UI_MENU: drawUIMenu(); break;
    case UI_LOK_ADDRESS: drawUILokAddress(); break;
    case UI_WIFI_INFO: drawUIWifiInfo(); break;
    default: //UI_THROTTLE
      drawUIThrottle();
      break;
  }
}


void loopUILocked() {
  if (uiCooloff > millis()) return;
  uiCooloff = millis() + 300;
  
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(2);
  display.setCursor(3, 0);
  display.print("BLOCAT"); 
  display.setCursor(3, 20);
  display.print("LOCKED");
  display.display();   
}