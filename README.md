# ThrottleESP
Z21 throttle implementation on ESP32 like a light Z21 lok maus. You might even build it on ESP8266 as there is no ESP32 specific functions used. It uses a standard variable resistor (pot) to handle the speed. Only 126 DCC steps is implemented. A 3x4 keyboard is used to navigate throw the menu and handle the functions 0 to 9. The code gets data from the Z21 asyncronous, so if you change the locomotive or the tracks gets an EStop you get notified on the throttle also. 

# Setup the Wifi
The wifi SSID and password are set inside the Config.h file. Just edit it and put your settings on.

# How to make the enclosure
After you print it on a standard 3D filament printer you need some M3 insert-threads and M3 standard screws to put it all together. See the images for more information. I test-printed them on my Anycubic Kobra S1 3D printer.

# Copyright
Fell free to download, make and share this project! A like or share of this repo will be much appreciated!
