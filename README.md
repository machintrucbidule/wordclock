# wordclock
Word clock for an ESP8266 with a ws2812b led matrix

##1. Quick description
This project aim is to display a word clock on a ws2812b led matrix and a TEM6000 luminosity sensor, pluged on an ESP8266.
At the moment that's only available for a french word clock, but it could be updated to make an english one too. Letters placement is already available in the english_matrix.txt file, then the whole code would need to be updated in order to :
	- Turn on correct leds for other languages (ledarrays.ino and functions.ino)
	- Display the configuration page on other languages (data/index.html for most of the code, and also some texts in webserver.ino)
	
##2. Installation
This code is intented to be compiled using Arduino IDE: https://www.arduino.cc/en/software
Procedure to configure Arduino IDE for ESP8266: https://github.com/esp8266/Arduino
We also need to install "ESP8266 LittleFS Data Upload" for Arduino IDE (used to copy files on the ESP8266 flash memory): https://github.com/earlephilhower/arduino-esp8266littlefs-plugin
Then open the "horloge.ino" file within Arduino IDE. Before to compile, we need to install some libraries.
Go to Sketch > Include Library > Manage Libraries, then install following libraries:
	- NTPClient
	- Adafruit NeoPixel
	- ArduinoJson
	- EWMA
	- StringSplitter
	- WifiManager
	
From here project should compile without error.
To upload it on the ESP8266, choose Sketch > Upload
Then upload files to be copied on the flash memory by launching Tools > ESP8266 LittleFS Data Upload (serial monitor needs to be closed)
Reopen serial monitor, some data should display (wifi connection status, etc...)

##3. Configuration
Inside horloge.ino you'll find some configurable values:
	- #define PIN_LUM     A0 ==> Pin where luminosity sensor is plugged (A0 by default) - be careful, number written on the esp8266 is usually different, please refer to the pinout schema for your esp8266
	- #define PIN_LEDS    2  ==> Pin where led matrix is plugged (2 by default, meaning D4 on my esp8266)
	- #define NUMPIXELS   256==> Number of leds on the matrix (16x16) - shouldn't be different except if you update code for other sizes of led matrix
	- #define NTP_UPDATE_INTERVAL 60000*60*24 ==> Delay to refresh time using ntp (each day by default)
	
##4. Wiring
Easiest way (less solders) to connect everything (esp8266, led matrix and temt6000 sensor) together is, and knowing that ws2812 have : 3 connectors for power (red/black), 1 data input (green), 1 data output (green)
	- Connect power source (5v) to led matrix
	- Solder 2 wires from another power connector of the led matrix to the "Vin" (+) and "Gnd" (-) connectors of the ESP8266
	- Solder 1 wire from ESP8266 Led output (D4 by default) to the led matrix data input, and add a 470 Ohms resistor close to the led matrix
	- Solder the 3 wires for temt6000 as follow:
		- VCC => to the "3v" ESP8266 Pin
		- GND => to the last led matrix ground pinout
		- SIG => to the pin used for luminosity sensor on the ESP8266 (A0 by default)
		
##5. Usage
Once everything is installed and wired, plugin the power source (can be powered using esp8266 usb port, but depending of the luminosity, if too much current is drained, esp can crash)
It should display:
	- a blue ring (initialization)
	- then an orange ring (no wifi connection), meaning you have to configure wifi. Use your phone or computer to connect of the access point launched by the ESP8266 (named "horloge" by default). Then go to "Configure Wifi", select your network, enter password. ESP will reboot.

If WiFi has been successfully configured, it will display:
	- a blue ring (initialization)
	- a green ring (wifi connected, ntp time sync - can be quite fast and not visible)
	- TIME!
	
Then to configure the timezone, colors, luminosity, etc... Using your phone or a computer connected on the same network than the ESP, go to "http://horloge" (can be changed in the configuration).