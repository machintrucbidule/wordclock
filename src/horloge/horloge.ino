#include <ESP8266WiFi.h>        //ESP8266 Core WiFi Library
#include <ESP8266WebServer.h>   //Local WebServer
#include <DNSServer.h>          //Local DNS Server used to redirect all requests to the configuration portal
#include <WiFiManager.h>        //To automatically manage wifi connection
#include <map>                  //For maps (dictionaries)
#include <vector>               //For vectors (lists)
#include <NTPClient.h>          //To connect on an NTP server (time sync)
#include <WiFiUdp.h>            //UDP used for NTP
#include <Adafruit_NeoPixel.h>  //To manage leds
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "LittleFS.h"           //To manage config on flash
#include <ArduinoJson.h>        //To manage json config file
#include <StringSplitter.h>     //To manage strings splits
#include <Ewma.h>               //To smooth luminosity readings

#define PIN_LUM     A0              //Pin where luminosity sensor is plugged (A0)
#define PIN_LEDS    2               //Pin where led matrix is plugged (D4) - (add a 400-470ohms resistors on data wire, and optionaly a 1000uf capacitor between power wires)
#define NUMPIXELS   256             //Number of leds on the matrix (16x16)
//#define NTP_UPDATE_INTERVAL 60000 //NTP time synched each minute
#define NTP_UPDATE_INTERVAL 60000*60*24 //NTP time synched each day (each minute would be better, but can freeze leds updates during time sync)

const char *fileconfig       = "config.json";        //config file - to be loaded once separately thru "ESP8266 LittleFS Data Upload" in Arduino IDEp
const char *fileconfigBackup = "config.json.save";   //backup config file - will be be automatically created based on the backup one during first startu, then never updated, automatically used to fix config file if corrupted, or to manually reset to factory settings
const size_t capacityConfig = 4096;                  //buffer json loading

//Instantiation of objects
WiFiManager wifiManager;                                             //Wifi connection
std::unique_ptr<ESP8266WebServer> server;                            //Pointer to webserver
WiFiUDP ntpUDP;                                                      //UDP for NTP
NTPClient timeClient(ntpUDP);                                        //NTP time sync
Adafruit_NeoPixel pixels(NUMPIXELS, PIN_LEDS, NEO_GRB + NEO_KHZ800); //Leds management
DynamicJsonDocument docConfig(capacityConfig);                       //Json config
Ewma adcFilter(0.05);                                                //Filter for light values, with 5% weight (less noise, slow changes)

//Following variables will contain synchronzed time (yes I know, a struct would be better)
int time_hours   =   NULL;
int time_minutes =   NULL;
int time_seconds =   NULL;

//Dictionaries to store the leds ids for each word to be displayed (details in ledarrays.ino)
std::map<String, std::vector<int>> ledsarray_start;
std::map<String, std::vector<int>> ledsarray_hours;
std::map<String, std::vector<int>> ledsarray_minutes;
std::map<String, std::vector<int>> ledsarray_seconds;

//Light values
int last_light_val=0;
int light_val=0;

//Ran once on startup
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  
  pinMode(PIN_LUM, INPUT);                        //Initialize PIN input for light sensor
  WiFi.setPhyMode(WIFI_PHY_MODE_11B);             //Force WiFi B, lower bandwidth, but better range
  WiFi.setOutputPower(20.5);                      //WiFi highest power
  
  Serial.println("Initialization of leds ids in dictionnaries");
  initLedsArrays();
  
  //Initiating leds matrix
  pixels.begin();
  pixels.setBrightness(255);
  //hideTime(true, true);

  
  //Initialization, we display a blue ring
  Serial.println("Display blue ring while initialization");
  showRing(0,0,255);  //Not working on cold start, but works if reboot. Tried to add a 2s delay to let time to led array to initialize, but doesn't change anything...
  showRing(0,0,255);  // ... but with a second one, it seems to be working...

  //Mounting file system to manage configuration file
  Serial.println("Mounting file system");
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    Serial.println("Display red ring");
    showRing(255,0,0);
    return;
  }

  //Load config values
  if (!loadConfig()){
    Serial.println("Failed to load config");
    Serial.println("Display red ring");
    showRing(255,0,0);
    return;
  }

  //Try to connect on wifi
  Serial.println("Try to connect on wifi");
  wifiManager.setAPCallback(captivePortalCallback);                 //Callback when launching captive portal (to display orange leds and warn user he has to connect to enter his wifi credentials)
  wifiManager.setSaveConfigCallback(wifiSavedCallback);             //Callback when config has been save and is working, to restart esp (only way to really refresh the hostname on router side)
  wifiManager.setHostname(docConfig["hostname"].as<const char*>()); //Hostname to have an easy address to reach the esp from a browser

  if (wifiManager.autoConnect(docConfig["hostname"].as<const char*>())){
    //Connection established
    Serial.println("Wifi connection established");

    //Wifi connected, green ring while to load data
    Serial.println("Display green ring");
    showRing(0,255,0);

    Serial.print("WiFi mode = ");
    Serial.println(WiFi.getPhyMode());
  }

  //Let's synchronize hour
  Serial.println("Starting NTP Sync");
  timeClient.begin();

  //Start webserver to manage configuration updates
  server.reset(new ESP8266WebServer(WiFi.localIP(), 80));

  //Dynamic pages
  server->on("/config", HTTP_POST, handleConfig);         //Handle to manage the configuration updates (target of the config form)
  server->on("/getLightLevel.json", handleGetLightLevel); //Handle page which returns current light level
  
  //Static files used as librairies
  server->serveStatic("/css/horloge.css", LittleFS, "/css/horloge.css");
  server->serveStatic("/js/jquery.ui.touch-punch.js", LittleFS, "/js/jquery.ui.touch-punch.js");  
  server->serveStatic("/js/horloge.js", LittleFS, "/js/horloge.js");
  server->serveStatic("/favicon.ico", LittleFS, "/favicon.ico");
  server->serveStatic("/", LittleFS, "/index.html");
  server->serveStatic("/config.json", LittleFS, "/config.json");

  //Here as reference, if one of the CDN used on the index.html isn't working anymore, below static files can be used
  //But CDN is preferable to avoid any memory/speed issue with ESP8266
  server->serveStatic("/css/jquery-ui.min.css", LittleFS, "/css/jquery-ui.min.css");
  server->serveStatic("/css/jquery-ui.theme.min.css", LittleFS, "/css/jquery-ui.theme.min.css");
  server->serveStatic("/css/img/ui-4.png", LittleFS, "/css/img/ui-4.png");
  server->serveStatic("/css/img/ui-5.png", LittleFS, "/css/img/ui-5.png");
  server->serveStatic("/css/img/ui-71.png", LittleFS, "/css/img/ui-71.png");
  server->serveStatic("/css/img/ui-72.png", LittleFS, "/css/img/ui-72.png");
  server->serveStatic("/css/img/ui-c.png", LittleFS, "/css/img/ui-c.png");
  server->serveStatic("/css/img/ui-f.png", LittleFS, "/css/img/ui-f.png");
  server->serveStatic("/css/load1.css", LittleFS, "/css/load1.css");
  server->serveStatic("/js/jquery.min.js", LittleFS, "/js/jquery.min.js");
  server->serveStatic("/js/jquery-ui.min.js", LittleFS, "/js/jquery-ui.min.js");
  server->serveStatic("/js/jqColorPicker.min.js", LittleFS, "/js/jqColorPicker.min.js");
  server->serveStatic("/js/jqColorPicker.js.map", LittleFS, "/js/jqColorPicker.js.map");
  
  server->begin();
  Serial.println("Web server started ==> " + WiFi.localIP().toString() + " / " + WiFi.hostname());
}

//Main function, ran on each cycle (with a 100ms delay at the end, so 10 times/sec)
void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("main loop");
  
  //Manages request sent to webserver
  server->handleClient();

  //Manages ntp time sync
  timeClient.update();

  //Update leds luminosity level
  updateLightLevel();

  //If hour/min/sec has changed, we update it on display
  if(time_hours != timeClient.getHours() || time_minutes != timeClient.getMinutes() || time_seconds != timeClient.getSeconds()){
    time_hours   = timeClient.getHours();
    time_minutes = timeClient.getMinutes();
    time_seconds = timeClient.getSeconds();

    //Display updated time on led matrix
    showTime();

    //Showing free heap memory (debugging purposes, to monitor if there's any memory leak)
    Serial.print("Free heap = ");
    Serial.println(ESP.getFreeHeap());
    Serial.println();
  }

  //Check if restart is required
  if(timeClient.getHours() == docConfig["restart"]["hour"].as<int>() && timeClient.getMinutes() == 0 && timeClient.getSeconds() == 0
    && (String(timeClient.getDay()) == docConfig["restart"]["day"].as<String>() || docConfig["restart"]["day"].as<String>() == "all")){
    Serial.println("Auto restart initiated");
    ESP.restart();
  }
  
  delay(100);
}

//Entering captive portal, display orange rings to inform user
void captivePortalCallback (WiFiManager *myWiFiManager) {
  //Unable to connect on a wifi network, portal started
  Serial.println("Unable to connect on wifi - Entered wifi config mode");
  
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());

  Serial.println("Display orange ring while wifi configuration");
  showRing(255,127,0);
}

//Wifi settings have been updated, we restart ESP to refresh hostname on router
void wifiSavedCallback () {
  Serial.println("Wifi settings correctly updated, restarting ESP to refresh hostname on router");
  ESP.restart();
}
