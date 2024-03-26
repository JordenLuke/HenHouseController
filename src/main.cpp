/*
 * Author: Jorden Luke
 * Date: 3-5-2024
 * Description: This is the main file for the Henhouse server. This appication 
 * controls a loght, and a water heater. Sensors are used to determine when
 * the light and heater should be turned on. The light is also controlled by
 * some timers.
 *
 * This code is released under the Yerba Mate license:
 * ----------------------------------------------------------------------------
 * "THE YERBA MATE LICENSE see Beer-Weare" (Revision 42):
 * Jorden Luke <<jorden.luke@gmail.com>> wrote this file. As long as you retain
 * this notice, you can do whatever you want with this stuff. If we meet some 
 * day, and you think this stuff is worth it, you can buy me a Yerba Mate in return. 
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h> 
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <ctime>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "ExtendedDHT.h"
#include "ExtendedDallasTemperature.h"
#include "Light.h"


//defines for debuging and testing
#define DEBUG
//#define FUNCTIONAL_TIMER_TEST
//#define FUNCTIONAL_SENSOR_TEST
//#define DATA_TEST
#define SENSOR_TEST

//Defines for pins
#define DHTPIN 2
#define DHT2PIN 14
#define DSTTEMP 12
#define HEATER_PIN 5
#define LIGHT_PIN 4

//defines for easy of programing
#define ON  true
#define OFF false

//Defines for sensors types
#define DHTTYPE DHT22

//defines for time
#define SIXAMSECONDS  (6*60*60)
#define NINEPMSECONDS (21*60*60)

//const 
const char* ssid = "1776 2.4G";;
const char* password = "BennettEmma1920";
const char* world_time_url = "http://worldtimeapi.org/api/ip";
const char* sunriseSunsetUrl = "http://api.sunrise-sunset.org/json?lat=41.730676&lng=-111.834894&tzid=MST";
const int smaple_time = 6000;
const float hen_house_on_temp= 10;
const float hen_house_off_temp = 20;
const unsigned long half_hour_millis = 1800000;
const float water_on_temp = 33.00;
const float water_off_temp = 35.00;
const size_t buffer_size = JSON_OBJECT_SIZE(12);
const int utc_time_zone_offset_millis = 25200;

//stuff for button HTML
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// golbal variables
unsigned long time_now = 0; //updatd everytime it goes through the loop 
unsigned long time_last = 0; //keeps track of the last time the sensor were scanned 
unsigned long millis_at_midnight = 0; // Keeps track of when to reset clocks for turning ligt on and off and highs and lows
unsigned long millis_at_sunrise= 0; // what time to turn on light before sunrise
unsigned long millis_at_sunset =0; //when to turn it on bfore sunset
unsigned long millis_at_6AM =0;
unsigned long millis_at_9PM=0;

bool heaterOn = OFF; 


/****gobal objects ***/
// Create an instance of the DHT sensor
ExtendedDHT DHTindoor(DHTPIN, DHTTYPE);
ExtendedDHT DHToutdoor(DHT2PIN, DHTTYPE);

//Dallas onewire sensor 
OneWire onewire(DSTTEMP);
ExtendedDallasTemperature waterDallas(&onewire);
Light light(LIGHT_PIN);

//Connectitvity stuff 
AsyncWebServer server(80);
HTTPClient http;
WiFiClient client;

/*******Golbal Sturcts USed to help organize data *****************/
// Initialize indoor sensor varibles the temperature and humidity variables
Climate indoor = { 0, 0, 0, 0, 0, 0 };

// Initialize outdoor sensor varibles the temperature and humidity variables
Climate outdoor = { 0, 0, 0, 0, 0, 0};

//Initialize water sensor varibles
DallasData water = {0,0,0};

//function prototypes:
void TimeReset();
bool getSensors();
String handleJson();
void resetHighLow();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  Serial.print("\n Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoConnect(true);
  WiFi.persistent(true);

  //start sensors
  DHTindoor.begin();
  DHToutdoor.begin();
  waterDallas.begin();

  //setup for light
  light.begin();
  light.enable();

  //set up for heater 
  pinMode(HEATER_PIN,OUTPUT);
  digitalWrite(HEATER_PIN,OFF);

  if(!LittleFS.begin()) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false,NULL);
  });

  server.on("/json",HTTP_GET,[](AsyncWebServerRequest *request){
    request->send(200,"application/json", handleJson());
  });
  
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS,"/script.js", String(), false,NULL);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS,"/style.css", "text/css", false,NULL);
  });

  server.on("/temperature-icon.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS,"/temperature-icon.svg", "image/svg+xml",false,NULL);
  });

  server.on("/humidity-icon.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS,"/humidity-icon.svg", "image/svg+xml",false,NULL);
  });

 server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS,"/chicken.png", "image/x-icon",false,NULL);
  });

 server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
  String messageID;
  String value;
  
  if(request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2))
  {
    
    messageID = request->getParam(PARAM_INPUT_1)->value();
    value = request->getParam(PARAM_INPUT_2)->value();
    if(messageID.equals("light"))
    {
       if(value.toInt())
       {
          #ifdef DEBUG
          Serial.println("Turning Light on Via Web");
          #endif
          light.setFlag(OVERRIDEON);
       }
       else
       {
          #ifdef DEBUG
          Serial.println("Turning Light off Via Web");
          #endif
          light.clearFlag(OVERRIDEON);
       }
    }
  }
  #ifdef DEBUG
  Serial.println("");
  #endif

  request->send(200,"text/plain","OK");
 });

  server.begin();
  TimeReset(); // rest time to next reset
  // rest daily highs and lows
  resetHighLow();

  #ifdef DEBUG
  Serial.println("Light Flags: " + String(light.getflags()));
  Serial.println("End of Setup");
  #endif
  }

void loop() 
{
  time_now = millis();
  //sample the sensors
  #ifdef DEBUG
  
  #endif

  if((time_now - time_last) > smaple_time)
  {
    if(getSensors())
    {
      time_last = millis();

      if(indoor.temperature < hen_house_on_temp)
      {
        #ifdef DEBUG
        uint8_t temp = light.getflags();
        #endif

        light.setFlag(TEMPFLAG); 
    
        #ifdef DEBUG
        if(temp != light.getflags())
          Serial.println("Light ON");
        #endif
      }
      
      if(indoor.temperature > hen_house_off_temp)
      {
        #ifdef DEBUG
        uint8_t temp = light.getflags();
        #endif

        light.clearFlag(TEMPFLAG);
        #ifdef DEBUG
        if(temp != light.getflags())
        Serial.println("Light OFF");
        #endif 
      }

      if(water.temperature < water_on_temp)
      {
        digitalWrite(HEATER_PIN,ON);
        #ifdef DEBUG
        if(heaterOn == OFF)
          Serial.println("Heater ON");
        #endif
        heaterOn = ON;
      }

      if(water.temperature > water_off_temp)
      {
        digitalWrite(HEATER_PIN,OFF);

        #ifdef DEBUG
        if(heaterOn == ON)
          Serial.println("Heater off");
        #endif
        heaterOn = OFF;
      }
    }
  }

  #ifdef FUNCTIONAL_TIMER_TEST
  time_now = millis();
  Serial.println("Time_now: " +String(time_now));
  Serial.println("Millis at millis_at_6AM: " + String(millis_at_6AM));
  Serial.println("Millis at millis_at_sunrise: " + String(millis_at_sunrise));
  Serial.println("Millis at millis_at_sunset: " + String(millis_at_sunset));
  Serial.println("Millis at millis_at_9PM: " + String(millis_at_9PM));
  #endif

  if( time_now >= millis_at_6AM && time_now < millis_at_sunrise)
  {
    #ifdef DEBUG
    uint8_t temp = light.getflags();
        #endif

    light.setFlag(SUNRISEFLAG); 
    #ifdef DEBUG
    if(temp != light.getflags())
     Serial.println("Light ON");
    #endif 
  }
  else if(time_now >= millis_at_sunset && time_now < millis_at_9PM)
  {
    #ifdef DEBUG
    uint8_t temp = light.getflags();
        #endif
    light.setFlag(SUNSETFLAG);
    #ifdef DEBUG
    if(temp != light.getflags())
     Serial.println("Light On");
    #endif 
  }
  else
  {
    #ifdef DEBUG
    uint8_t temp = light.getflags();
    #endif

    light.clearFlag(SUNSETFLAG);
    light.clearFlag(SUNRISEFLAG);

    #ifdef DEBUG
    if(temp != light.getflags())
     Serial.println("Light Off");
    #endif 
  }

  if (time_now > millis_at_midnight)
  {
    // reset variables and do whatever else you need to do
    TimeReset(); // rest time to next reset

    // rest daily highs and lows
    resetHighLow();
  }

  delay(500);
}
/*********************************************************************************
*  The followng code cood gets sunrise and sunset times then convets the times to
*  milliseconds. The next Midnight, 6 Am and 9 PM  is also claculated in 
*  millissconds. All these times are then used for timers to turn on and off the
*  light and reset the high and low temps.
* 
***********************************************************************************/
void TimeReset()
{
  time_t sunriseTime;
  time_t sunsetTime; 
  http.begin(client,sunriseSunsetUrl);
  int http_code = http.GET();
 
  if (http_code > 0) {
    
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    const char* sunrise = doc["results"]["sunrise"];
    const char* sunset = doc["results"]["sunset"];
    
    // Parse sunrise and sunset times
    int sunriseHour, sunriseMinute, sunsetHour, sunsetMinute;
    sscanf(sunrise, "%d:%d", &sunriseHour, &sunriseMinute);
    sscanf(sunset, "%d:%d", &sunsetHour, &sunsetMinute);
    
    // Calculate light on and off times based on sunrise and sunset for seeconds
    sunriseTime = (sunriseHour * 3600) + (sunriseMinute * 60);
    sunsetTime = (sunsetHour * 3600) + (sunsetMinute * 60) + (12 *3600);
    
    doc.clear();
    #ifdef DEBUG
    Serial.print("Sunrise Time: ");
    Serial.println(sunriseTime);
    Serial.print("Sunset: ");
    Serial.println(sunsetTime);
    #endif
  }
  else
  {
    http.end();
    return;
  }
  // get current Unix time from World Time API
  http.begin(client, world_time_url);
  http_code = http.GET();
  
  if (http_code == HTTP_CODE_OK)
  {
    String response = http.getString();

    // parse JSON response
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    unsigned long unixTime = doc["unixtime"];//current utc time
    const char* utc_offset =doc["utc_offset"] ;
    
    //raw offset is negative 
    int rawOffset, offsetHr, offsetMin;   
    sscanf(utc_offset, "%d:%d", &offsetHr, &offsetMin);

    Serial.println("Hr offset: "+ String(offsetHr));
    rawOffset = (offsetHr * 3600); // offset in second from utc time based on time zone of ip
    unixTime = unixTime + rawOffset; //currnet time where time is from ip
    
    // calculate milliseconds until midnight
    //24*60*60 (hours in day * minutes * seconds)
    unsigned long secondsSinceMidnight = unixTime % 86400;  //last time it was midnight 
    unsigned long secondsUntilMidnight = 86400 - secondsSinceMidnight; // next time in seconds  until it is midnight 
    unsigned long millistime = millis(); 
    millis_at_midnight = millistime + (secondsUntilMidnight * 1000); // how many millis the millis should be at mid night 

    //calcuate milliseconds until 6am 
    //6*60*60 (6 am * Minutes * seeconds)
    millis_at_6AM = millistime + ((SIXAMSECONDS-secondsSinceMidnight) * 1000);  // millis() at 6am to turn on light 
    millis_at_9PM = millistime + ((NINEPMSECONDS-secondsSinceMidnight) * 1000); //mills at 9 PM  to trun off light 
        
    //callcuate what millis will be at sun rise + 30 minutes to turn off light  
    millis_at_sunrise = millistime + ((sunriseTime- secondsSinceMidnight)*1000) + half_hour_millis;
    //adjust sunrise time for daylight saving time 
    millis_at_sunrise = millis_at_sunrise +(utc_time_zone_offset_millis + rawOffset);
    //calculat what millis will be at sun set - 30 minutes to turn on light
    millis_at_sunset =  millistime + ((sunsetTime- secondsSinceMidnight)*1000) - half_hour_millis;
    //adjust sunset time for daylight saving time 
    millis_at_sunset = millis_at_sunset +(utc_time_zone_offset_millis + rawOffset);

    // output for debugging
    #ifdef DEBUG
    Serial.println("Current Unix Time: " + String(unixTime));
    Serial.println("Millistime: " + String(millistime));
    Serial.println("Milliseconds until midnight: " + String(millis_at_midnight - millistime));
    Serial.println("Millis at midnight: " + String(millis_at_midnight));
    Serial.println("Millis at Sunrise: " + String(millis_at_sunrise));
    Serial.println("Millis at Sunset: " + String(millis_at_sunset));
    Serial.println("Millis at 6AM: " + String(millis_at_6AM));
    Serial.println("Millis at 9PM: " +String(millis_at_9PM));
    #endif
    
    #ifdef FUNCTIONAL_TIMER_TEST
    millis_at_sunrise = (18 * 1000) + millistime; 
    millis_at_6AM = (12 * 1000) + millistime;
    millis_at_9PM =  (30 * 1000) + millistime;
    millis_at_sunset = (24 * 1000) + millistime;
    millis_at_midnight = (60 * 1000) + millistime;
    Serial.println("Millis at millis_at_6AM: " + String(millis_at_6AM));
    Serial.println("Millis at millis_at_sunrise: " + String(millis_at_sunrise));
    Serial.println("Millis at millis_at_sunset: " + String(millis_at_sunset));
    Serial.println("Millis at millis_at_9PM: " + String(millis_at_9PM));
    #endif
    doc.clear();
  }

  http.end();
}

bool getSensors()
{
  #ifdef DATA_TEST
  
  #endif

  if(!DHTindoor.sampleData(indoor))
  {
    #ifdef SENSOR_TEST
    Serial.println("Failed to get indoor sennsor");
    #endif
    //return false;
  }
  
  if(!DHToutdoor.sampleData(outdoor))
  {
    #ifdef SENSOR_TEST
    Serial.println("Failed to get outddoor sennsor");
    #endif
   //return false;
  }
  
  if(!waterDallas.sampleData(water))
  {
    #ifdef SENSOR_TEST
    Serial.println("Failed to get Water sennsor");
    #endif
    return false;
  }

  return false;
}

String handleJson()
{
  #ifdef DATA_TEST
  indoor.humidity = 24.8;
  indoor.highHumidity = 30.0;
  indoor.lowHumidity =17.100;
  indoor.temperature = 60;
  indoor.highTemperature =61;
  indoor.lowTemperature = 45;
  

  outdoor.humidity = 30.0;
  outdoor.highHumidity = 30.0;
  outdoor.lowHumidity =17.100;
  outdoor.temperature = 60;
  outdoor.highTemperature =61;
  outdoor.lowTemperature = 45;
  
  water.temperature =45.00;
  water.highTemperature = 55.00;
  water.lowTemperature = 60;  
  #endif

StaticJsonDocument<buffer_size> doc;

// Populate sensor data (example values)
doc["indoorTemperature"] = indoor.temperature;
doc["indoorHigh"] = indoor.highTemperature;
doc["indoorLow"] = indoor.lowTemperature;
doc["indoorHumidity"] = indoor.humidity;

doc["outdoorTemperature"] = outdoor.temperature;
doc["outdoorHigh"] = outdoor.highTemperature;
doc["outdoorLow"] = outdoor.lowTemperature;
doc["outdoorHumidity"] = outdoor.humidity;

doc["waterTemperature"] = water.temperature;
doc["waterHigh"] = water.highTemperature;
doc["waterLow"] = water.lowTemperature;

//otherdata
if((light.getflags() & OVERRIDEON) == OVERRIDEON )
{
  doc["light"] = true;
}
else
{
  doc["light"] = false;
}

// Serialize JSON to a String
String jsonString;
serializeJson(doc, jsonString);

    return jsonString;
}

//reset low and high 
void resetHighLow()
{
  indoor = { 0, 0, 0, 0, 0, 0 };
  outdoor = {0, 0, 0, 0, 0, 0 };
  water = {0,0,0};
}