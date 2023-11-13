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
