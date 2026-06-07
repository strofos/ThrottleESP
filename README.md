# Z21 Throttle ESP
Z21 throttle implementation on ESP32 like a light Z21 lok maus. You might even build it on ESP8266 as there is no ESP32 specific functions used. It uses a standard variable resistor (pot) to handle the speed. Only 126 DCC steps is implemented. A 3x4 keyboard is used to navigate throw the menu and handle the functions 0 to 9. The code gets data from the Z21 asyncronous, so if you change the locomotive or the tracks gets an EStop you get notified on the throttle also. 

# Electronic Components
 1. ESP32 DEV board
 2. Nokia 5110 PCD8544 display module
 3. 5k or 10k variable resistor
 4. 3x4 keyboard
 5. 18650 single cell as power suply

# Flash the code
Use a standard Arduino IDE to flash the code. Just make sure you have the correct board selected and install the graphic libraries from Adafruit. I used a Nokia 5110 display, PCD8544 chipset, SPI driven, so is easy to adapt it to another SPI display.  

# Setup the Wifi
The wifi SSID and password are set inside the Config.h file. Just edit it and put your own settings.

# How to make the enclosure
After you print the Top and Bottom enclosure parts on a standard 3D filament printer you need some M3 insert-threads and M3 standard screws to put it all together. See the images for more information. I test-printed them on my Anycubic Kobra S1 3D printer. The 18650 cell lid is still under design so it is missing from the current repo.

# Copyright
Fell free to download, make and share this project! A like or share of this repo will be much appreciated!
