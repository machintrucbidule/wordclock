void hideTime(bool day, bool write){
  int luminosity  = docConfig["brightness"]["night"]["back"];
  float percentage = 0;
  if(day) {
    luminosity  = docConfig["brightness"]["day"]["back"];
  }
  percentage = (float)luminosity/255;

  pixels.fill(pixels.gamma32(
      ((int)(docConfig["colors"]["back"]["r"].as<int>()*percentage) << 16) 
      + ((int)(docConfig["colors"]["back"]["g"].as<int>()*percentage) << 8) 
      + (int)(docConfig["colors"]["back"]["b"].as<int>()*percentage)
  ));

  if(write) {
    pixels.show();
    Serial.println("all leds off sent");
  }
}

void showTime(){  //Oh yeah!
  Serial.println("Display new time ==> " +  String(time_hours) + "h" + String(time_minutes) + "m" + String(time_seconds) + "s");

  bool morning = true;
  int time_h = time_hours;
  int time_m = time_minutes;
  int time_s = time_seconds;
  int luminosity_hours = docConfig["brightness"]["day"]["hours"].as<int>();
  int luminosity_seconds = docConfig["brightness"]["day"]["seconds"].as<int>();
  float per_lum_hours=1;
  float per_lum_seconds=1;
  int offset = (docConfig["colors"]["offset"].as<int>() * 65535) / 360;

  bool day = true;
  //Serial.println("Light level = " + String(light_val));
  if(light_val <= docConfig["brightness"]["threshold"].as<int>()){
    day = false;
    luminosity_hours = docConfig["brightness"]["night"]["hours"].as<int>();
    luminosity_seconds = docConfig["brightness"]["night"]["seconds"].as<int>();
  }

  per_lum_hours=(float)luminosity_hours/255;
  per_lum_seconds=(float)luminosity_seconds/255;
  
  hideTime(day, false);
  
  uint32_t color_h = pixels.gamma32(
                   ((int)(docConfig["colors"]["hours_on"]["r"].as<int>()*per_lum_hours) << 16) 
                   + ((int)(docConfig["colors"]["hours_on"]["g"].as<int>()*per_lum_hours) << 8) 
                   + (int)(docConfig["colors"]["hours_on"]["b"].as<int>()*per_lum_hours)
  );
  uint32_t color_m = pixels.gamma32(
                   ((int)(docConfig["colors"]["minutes_on"]["r"].as<int>()*per_lum_hours) << 16) 
                   + ((int)(docConfig["colors"]["minutes_on"]["g"].as<int>()*per_lum_hours) << 8) 
                   + (int)(docConfig["colors"]["minutes_on"]["b"].as<int>()*per_lum_hours)
  );
  uint32_t color_s = pixels.gamma32(
                   ((int)(docConfig["colors"]["seconds_on"]["r"].as<int>()*per_lum_seconds) << 16) 
                   + ((int)(docConfig["colors"]["seconds_ons_on"]["g"].as<int>()*per_lum_seconds) << 8) 
                   + (int)(docConfig["colors"]["seconds_on"]["b"].as<int>()*per_lum_seconds)
  );

  //If dynamic hour/min colors, we calculate them
  if(docConfig["colors"]["hourmin_mode"].as<int>() == 1){
    //Serial.println(String(time_hours) + " .. / .. " + String(time_hours%12));
    uint16_t color_h_temp = (float)(time_h%12 + (float)time_m/60)/12*65536 + offset;
    uint16_t color_m_temp = (float)(time_m%60 + (float)time_s/60)/60*65536 + offset;
    color_h = pixels.gamma32(pixels.ColorHSV(color_h_temp%65536 , docConfig["colors"]["saturation"].as<int>(), luminosity_hours));
    color_m = pixels.gamma32(pixels.ColorHSV(color_m_temp%65536 , docConfig["colors"]["saturation"].as<int>(), luminosity_hours));
 }

  //If seconds color has to be unique, but changing
  if(docConfig["colors"]["seconds_mode"].as<int>() == 1){
    uint16_t color_s_temp = ((float)(time_s-1)/59)*65536 + offset;
    color_s = pixels.gamma32(pixels.ColorHSV(color_s_temp%65536, docConfig["colors"]["saturation"].as<int>(), luminosity_seconds));
 }

  //We display "il est"
  addWord(ledsarray_start["il"], color_h);
  addWord(ledsarray_start["est"], color_h);

  //If not morning
  if(time_h == 0) morning = false;
  if(time_h >  12){
    time_h -= 12;
    morning = false;
  }

  /// MINUTES ///
  //Case when to display "moins"
  if(time_m > 30 && time_m%5 == 0){
    time_h += 1;
    if(time_h == 13) time_h = 1;

    addWord(ledsarray_minutes["moins"], color_m);
    
    if(time_m == 45){
      addWord(ledsarray_minutes["le"], color_m);
      addWord(ledsarray_minutes["quart"], color_m);
    }else if(time_m == 35){
      addWord(ledsarray_minutes["20"], color_m);
      addWord(ledsarray_minutes["5"], color_m);
    }else if(time_m == 40){
      addWord(ledsarray_minutes["20"], color_m);
    }else if(time_m == 50){
      addWord(ledsarray_minutes["10"], color_m);
    }else{  //55
      addWord(ledsarray_minutes["5"], color_m);
    }
  }else if(time_m == 30){
    addWord(ledsarray_minutes["et"], color_m);
    addWord(ledsarray_minutes["demie"], color_m);
  }else if(time_m == 15){
    addWord(ledsarray_minutes["et"], color_m);
    addWord(ledsarray_minutes["quart"], color_m);
  }else if(time_m > 0){
    if(time_m%10 == 0 || time_m <= 16){
      addWord(ledsarray_minutes[String(time_m)], color_m);
    }else{
      int minutes_unit = time_m%10;
      int minutes_tens = time_m - minutes_unit;

      addWord(ledsarray_minutes[String(minutes_unit)], color_m);
      addWord(ledsarray_minutes[String(minutes_tens)], color_m);

      if(minutes_unit == 1){
        addWord(ledsarray_minutes["et_minutes"], color_m);
      }
    }
  }

  /// HOURS ///
  if(time_h > 0 && time_h != 12){
    addWord(ledsarray_hours[String(time_h)], color_h);
    addWord(ledsarray_hours["heure"], color_h);
    if(time_h > 1){
      addWord(ledsarray_hours["s"], color_h);
    }
  }else if(morning == false){
    addWord(ledsarray_hours["minuit"], color_h);
  }else{  //12
    addWord(ledsarray_hours["midi"], color_h);
  }

  /// SECONDS ///
  if(docConfig["colors"]["seconds_display"] == "one"){
    //As there's no led for second 30, we display the 29th one
    if(time_s == 30) time_s = 29;
    
    //If seconds color has to be unique depending of its position
    if(docConfig["colors"]["seconds_mode"].as<int>() == 2){
      uint16_t color_s_temp = ((float)(time_s-1)/59)*65536 + offset;
      color_s = pixels.gamma32(pixels.ColorHSV( color_s_temp%65536, docConfig["colors"]["saturation"].as<int>(), luminosity_seconds));
    }
    addWord(ledsarray_seconds[String(time_s)], color_s);

    //Serial.println("show second " + String(time_s));
    
  }else if(docConfig["colors"]["seconds_display"] == "all"){
    //As there's no led for second 30, we display the 29th one
    if(time_s == 30) time_s = 29;
    
    for(int i = 1; i <= 59; i++){
      //If seconds color has to be unique depending of its position
      if(docConfig["colors"]["seconds_mode"].as<int>() == 2){
        uint16_t color_s_temp = ((float)(i-1)/59)*65536 + offset;
        color_s = pixels.gamma32(pixels.ColorHSV(color_s_temp%65536, docConfig["colors"]["saturation"].as<int>(), luminosity_seconds));
      }

      if(i != time_s){
        addWord(ledsarray_seconds[String(i)], color_s);

        //Serial.println("show second " + String(i));
      }
    }
  }else{  //docConfig["colors"]["seconds_display"] == "past"
    for(int i = 1; i <= time_s; i++){
      //If seconds color has to be unique depending of its position
      if(docConfig["colors"]["seconds_mode"].as<int>() == 2){
        uint16_t color_s_temp = ((float)(i-1)/59)*65536 + offset;
        color_s = pixels.gamma32(pixels.ColorHSV(color_s_temp%65536, docConfig["colors"]["saturation"].as<int>(), luminosity_seconds));
      }
      addWord(ledsarray_seconds[String(i)], color_s);
      //Serial.println("show second " + String(i));
    }
  }
  
  //We display the hour
  pixels.show();
  Serial.println("Send buffer with new hours to led");
}

//Add a word on leds buffer
void addWord(std::vector<int> arr, uint32_t color){
  for (const int& i : arr){
    pixels.setPixelColor(i, color);
  }
}

//Display a ring (turn on all leds used to display seconds) of specified color
void showRing(int r, int g, int b){
  uint32_t color = (r << 16) + (g << 8) + b;
  
  pixels.setBrightness(255);
  
  pixels.fill(0);
  
  for(int i = 1; i <= 59; i++){
    addWord(ledsarray_seconds[String(i)], color);
  }
  
  //We display the ring
  pixels.show();
}

//Get a text containing a number, and returns a text with a number between 0 and maxi
String byteLimit(String number, int maxi=255){
  int val = atoi(number.c_str());

  if(val < 0) val = 0;
  if(val > maxi) val = maxi;

  return String(val);
}

//Return substring, with negative last index managed
String substring(String str, int start, int end){
  if(end < 0){
    return str.substring(start, str.length() + end);
  }
  return str.substring(start, end);
}



void updateLightLevel(){
  last_light_val = analogRead(PIN_LUM);    //Read light level
  light_val = adcFilter.filter(last_light_val);
}

String getFile(char* path){
  String dat;
  File dataFile = LittleFS.open(path, "r");   //Ouverture fichier pour le lire
  Serial.println("Lecture du fichier '" + String(path) + "' en cours:" + String(dataFile.size()));
  for(int i=0;i<dataFile.size();i++){
    dat += (char)dataFile.read();
  }
  dataFile.close();
  
  return dat;
}
