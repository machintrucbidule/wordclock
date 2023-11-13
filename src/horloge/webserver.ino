void handleConfig(){
  bool reboot = false;  // will be set to true if config update requires a reboot
  bool dynamic = false; // will be set to true if request is coming from ajax

  //dynamic
  if(server->hasArg("dynamic") && server->arg("dynamic") == "1"){
    Serial.println("Request coming from ajax, no need to build config page");
    dynamic = true;
  }
  
  //hostname
  if(server->hasArg("hostname") && server->arg("hostname") != NULL){
    if(docConfig["hostname"] != server->arg("hostname")){
      docConfig["hostname"] = server->arg("hostname");
      reboot = true;
    }
  }

  //timezone
  if(server->hasArg("timezone") && server->arg("timezone") != NULL){
    docConfig["timezone"] = server->arg("timezone");
  }

  //restart_day
  if(server->hasArg("restart_day") && server->arg("restart_day") != NULL){
    docConfig["restart"]["day"] = server->arg("restart_day");
  }

  //restart_hour
  if(server->hasArg("restart_hour") && server->arg("restart_hour") != NULL){
    docConfig["restart"]["hour"] = server->arg("restart_hour");
  }

  //dst
  if(server->hasArg("dst") && server->arg("dst") != NULL){
    docConfig["dst"] = server->arg("dst");
  }

  //ntp_server
  if(server->hasArg("ntp_server") && server->arg("ntp_server") != NULL){
    docConfig["ntp_server"] = server->arg("ntp_server");
  }

  //colors_hourmin_mode
  if(server->hasArg("colors_hourmin_mode") && server->arg("colors_hourmin_mode") != NULL){
    docConfig["colors"]["hourmin_mode"] = byteLimit(server->arg("colors_hourmin_mode"),1);
  }

  //colors_seconds_mode
  if(server->hasArg("colors_seconds_mode") && server->arg("colors_seconds_mode") != NULL){
    docConfig["colors"]["seconds_mode"] = byteLimit(server->arg("colors_seconds_mode"),2);
  }

  //colors_seconds_display
  if(server->hasArg("colors_seconds_display") && server->arg("colors_seconds_display") != NULL){
    docConfig["colors"]["seconds_display"] = server->arg("colors_seconds_display");
  }

  //colors_back
  Serial.println("New colors_back : " + server->arg("colors_back"));
  if(server->hasArg("colors_back") && server->arg("colors_back") != NULL){
    StringSplitter *splitter = new StringSplitter(substring(server->arg("colors_back"),4,-1), ',', 3);
    docConfig["colors"]["back"]["r"] = byteLimit(splitter->getItemAtIndex(0));
    docConfig["colors"]["back"]["g"] = byteLimit(splitter->getItemAtIndex(1));
    docConfig["colors"]["back"]["b"] = byteLimit(splitter->getItemAtIndex(2));
    delete splitter;
  }

  //colors_hours_on
  Serial.println("New colors_hours_on : " + server->arg("colors_hours_on"));
  if(server->hasArg("colors_hours_on") && server->arg("colors_hours_on") != NULL){
    StringSplitter *splitter = new StringSplitter(substring(server->arg("colors_hours_on"),4,-1), ',', 3);
    docConfig["colors"]["hours_on"]["r"] = byteLimit(splitter->getItemAtIndex(0));
    docConfig["colors"]["hours_on"]["g"] = byteLimit(splitter->getItemAtIndex(1));
    docConfig["colors"]["hours_on"]["b"] = byteLimit(splitter->getItemAtIndex(2));
    delete splitter;
  }

  //colors_minutes_on
  Serial.println("New colors_minutes_on : " + server->arg("colors_minutes_on"));
  if(server->hasArg("colors_minutes_on") && server->arg("colors_minutes_on") != NULL){
    StringSplitter *splitter = new StringSplitter(substring(server->arg("colors_minutes_on"),4,-1), ',', 3);
    docConfig["colors"]["minutes_on"]["r"] = byteLimit(splitter->getItemAtIndex(0));
    docConfig["colors"]["minutes_on"]["g"] = byteLimit(splitter->getItemAtIndex(1));
    docConfig["colors"]["minutes_on"]["b"] = byteLimit(splitter->getItemAtIndex(2));
    delete splitter;
  }

  //colors_seconds_on
  if(server->hasArg("colors_seconds_on") && server->arg("colors_seconds_on") != NULL){
    StringSplitter *splitter = new StringSplitter(substring(server->arg("colors_seconds_on"),4,-1), ',', 3);
    docConfig["colors"]["seconds_on"]["r"] = byteLimit(splitter->getItemAtIndex(0));
    docConfig["colors"]["seconds_on"]["g"] = byteLimit(splitter->getItemAtIndex(1));
    docConfig["colors"]["seconds_on"]["b"] = byteLimit(splitter->getItemAtIndex(2));
    delete splitter;
  }

  //colors_saturation
  if(server->hasArg("colors_saturation") && server->arg("colors_saturation") != NULL){
    docConfig["colors"]["saturation"] = byteLimit(server->arg("colors_saturation"),255);
  }

  //colors_offset
  if(server->hasArg("colors_offset") && server->arg("colors_offset") != NULL){
    docConfig["colors"]["offset"] = byteLimit(server->arg("colors_offset"),360);
  }


  //brightness_day_back
  if(server->hasArg("brightness_day_back") && server->arg("brightness_day_back") != NULL){
    docConfig["brightness"]["day"]["back"] = byteLimit(server->arg("brightness_day_back"), 255);
  }

  //brightness_day_hours
  if(server->hasArg("brightness_day_hours") && server->arg("brightness_day_hours") != NULL){
    docConfig["brightness"]["day"]["hours"] = byteLimit(server->arg("brightness_day_hours"), 255);
  }

  //brightness_day_seconds
  if(server->hasArg("brightness_day_seconds") && server->arg("brightness_day_seconds") != NULL){
    docConfig["brightness"]["day"]["seconds"] = byteLimit(server->arg("brightness_day_seconds"), 255);
  }

  
  //brightness_night_back
  if(server->hasArg("brightness_night_back") && server->arg("brightness_night_back") != NULL){
    docConfig["brightness"]["night"]["back"] = byteLimit(server->arg("brightness_night_back"), 255);
  }

  //brightness_night_hours
  if(server->hasArg("brightness_night_hours") && server->arg("brightness_night_hours") != NULL){
    docConfig["brightness"]["night"]["hours"] = byteLimit(server->arg("brightness_night_hours"), 255);
  }

  //brightness_night_seconds
  if(server->hasArg("brightness_night_seconds") && server->arg("brightness_night_seconds") != NULL){
    docConfig["brightness"]["night"]["seconds"] = byteLimit(server->arg("brightness_night_seconds"), 255);
  }

  //brightness_threshold
  if(server->hasArg("brightness_threshold") && server->arg("brightness_threshold") != NULL){
    docConfig["brightness"]["threshold"] = byteLimit(server->arg("brightness_threshold"), 255);
  }

  //restart
  if(server->hasArg("restart") && server->arg("restart") == "1"){
    reboot=true;
  }
  
  
  //Save updates to json
  saveConfig();

  //restoreconfig
  if(server->hasArg("restoreconfig") && server->arg("restoreconfig") == "1"){
    Serial.println("Restore default config");
    LittleFS.remove(fileconfig);
    reboot=true;
  }
  
  if(reboot){
    String content = "<html><head>";
    content += "<meta http-equiv='refresh' content='10;url=http://" + docConfig["hostname"].as<String>() + "/'/>";
    content += "<style>";
    content += "body,h1,h2,h3{font-family:Calibri;background-color:#35363a;color:#fdfdfd;}";
    content += "</style>";
    content += "</head><body><center><h3>Red&eacute;marrage de l'horloge, veuillez patienter...</h3></center></body></html>";
    server->send(200, "text/html", content);
    delay(1000);
    ESP.restart();
  }else if(!dynamic){
    server->sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    server->send(303);                         // Send it back to the browser with an HTTP status 303
  }
}

void handleGetLightLevel(){
  Serial.println("Sending light level to web browser");
  String ret = "{\"average\":" + String(light_val) + ",\"last\":" + String(last_light_val) + "}";
  server->send(200, "application/json", ret);
}

void handleGetResetInfo(){
  Serial.println("Sending info about last reboot");
  String ret = "{\"reason\":\"";
  switch(rebootReason){
    case REASON_DEFAULT_RST : ret += "Normal startup by power on"; break;
    case REASON_WDT_RST : ret += "Hardware watch dog reset"; break;
    case REASON_EXCEPTION_RST : ret += "Exception reset"; break;
    case REASON_SOFT_WDT_RST : ret += "Software watch dog reset"; break;
    case REASON_SOFT_RESTART : ret += "Software restart"; break;
    case REASON_DEEP_SLEEP_AWAKE : ret += "Wake up from deep-sleep"; break;
    case REASON_EXT_SYS_RST : ret += "External system reset"; break;
  }
  ret += "\",\"time\":\"";

  long seconds = millis() / 1000;
  long minutes = seconds / 60;
  long hours = minutes / 60;
  long days = hours / 24;
  ret += String(days) + String("j ") + String(hours % 24) + String("h ") + String(minutes % 60) + String("m ") + String(seconds % 60) + String("s"); 
  
  ret += "\"}";
  server->send(200, "application/json", ret);
}
