#include <Imagine_SIM7000.h>
#include <avr/pgmspace.h>

Imagine_SIM7000 module;
SoftwareSerial mySerial(PIN_RX, PIN_TX);

const unsigned int ID = 0;
const char URL[] = { "http://imagineapp.azurewebsites.net/api/GPSDataCapture?code=AjMTbNQW7ogutpDmf2qt5vrIW8k48HeuwA02Zswiqbnsg28OABUsOg==" };
const char PIN[] = { "" };
const bool REBOOT = true;

void setup() {
  Serial.begin(19200);
  module.init(mySerial);

  if(REBOOT){
    if(module.restart()) Serial.println(F("Restart SUCCESS"));
    else return          Serial.println(F("Restart FAILED"));
  }
  
  if(module.checkAT()) Serial.println(F("Communication SUCCESS"));
  else return          Serial.println(F("Communication FAILED"));
    
  if(module.checkSIM(PIN)) Serial.println(F("SIM READY"));
  else return              Serial.println(F("SIM FAILURE"));

  if(module.prepareGPS()) Serial.println(F("GPS READY"));
  else return              Serial.println(F("GPS FAILURE"));
    
  if(module.prepareNetwork()) Serial.println("Network READY");
  else return                 Serial.println(F("Network FAILURE"));

  if(module.HTTPconnect(URL)) Serial.println(F("HTTP connect SUCCESS"));
  else return                 Serial.println(F("HTTP connect FAILURE"));
}


void loop() {
  String postData = "{";
  postData += ID;
  postData += ",";
  postData += module.getGNSSinfo();
  postData += "}";
  
  if(module.HTTPpost(postData)) Serial.println(F("HTTP post SUCCESS"));
  else return Serial.println(F("HTTP post FAILURE"));

  delay(500);
}
