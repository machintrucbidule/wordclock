//Load config from json file to docConfig object
bool loadConfig(){
  bool fileLoaded = true;
  //Try to load primary config file
  File file = LittleFS.open(fileconfig, "r");
  if (!file) {
    Serial.println("Unable to load primary config file");
    fileLoaded = false;
  }

  //Try to load backup file
  File fileBackup = LittleFS.open(fileconfigBackup, "r");
  if (!fileBackup) {
    Serial.println("Unable to load backup config file");

    //Neither primary or backup file has been loaded, that's an issue
    if(!fileLoaded){      
      Serial.println("Unable to load any config file - restarting ESP");
      ESP.restart();
      return false;
    }else{
      //First startup, we initiate backup file
      Serial.println("No backup file, writing it");
      fileBackup = LittleFS.open(fileconfigBackup, "w");
      if(fileBackup){
        int data;
        while ((data = file.read()) >= 0) {
          fileBackup.write(data);
        }
        fileBackup.close();
        file.seek(0);
      }
    }
  }
  
  DeserializationError err = deserializeJson(docConfig, file);
  if (err) {
      Serial.print("deserializeJson() of primary config file failed: ");
      Serial.println(err.c_str());

      //Trying to deserialize backup one
      Serial.println("Trying to deserialize backup config file, updates made to config will be lost");
      DeserializationError err = deserializeJson(docConfig, fileBackup);
      if (err) {
        Serial.print("deserializeJson() of backup config file failed: ");
        Serial.println(err.c_str());
        Serial.println("Unable to read any config file - restarting ESP");
        ESP.restart();
        return false;
      }else{
        //Backup file has been restored, we save it to initiate normal config.json
        Serial.println("Restore of config.json based on backup one");
        saveConfig();
      }
  }

  file.close();

  //refresh objects using config values
  refreshConfig();

  return true;
}

//Save config from docConfig object to json file
bool saveConfig(){
  String jsondoc="";
  File  file = LittleFS.open(fileconfig, "w");
  if (!file) {
    Serial.println("Unable to open file to save config");
    return false;
  }
  serializeJson(docConfig,jsondoc);
  file.print(jsondoc);  // sauvegarde de la chaine
  file.close();
  Serial.println(jsondoc);

  //refresh objects using config values
  refreshConfig();

  return true;
}

//Refresh the config based on json
void refreshConfig(){
  Serial.println("Set NTPClient on " + docConfig["ntp_server"].as<String>() + " / with timezone=" + docConfig["timezone"].as<String>() + " / dst = " + docConfig["dst"].as<String>());
  //NTPClient timeClient(ntpUDP, docConfig["ntp_server"].as<const char*>(), (docConfig["timezone"].as<int>() + docConfig["dst"].as<int>())*3600, 60000);
  timeClient.setPoolServerName(docConfig["ntp_server"].as<const char*>());
  timeClient.setTimeOffset((docConfig["timezone"].as<int>() + docConfig["dst"].as<int>())*3600);
  timeClient.setUpdateInterval(NTP_UPDATE_INTERVAL);
}

const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
