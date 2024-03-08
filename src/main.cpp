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


//define for debug
#define DEBUG

//Defines for pins
#define DHTPIN 2
#define DHT2PIN 14
#define DSTTEMP 12
#define HEATER_PIN 5
#define LIGHT_PIN 4

//defines for easy of programing
#define ON false
#define OFF true

//Defines for sensors types
#define DHTTYPE DHT22

//defines for time
#define SIXAMSECONDS  (6*60*60) 
#define NINEPMSECONDS (21*60*60)

//const 
const char* ssid = "1776 2.4G";;
const char* password = "BennettEmma1920";
const char* world_time_url = "http://worldtimeapi.org/api/ip";
const char* sunriseSunsetUrl = "http://api.sunrise-sunset.org/json?lat=41.730676&lng=-111.834894";
const int smaple_time = 6000;
const float hen_house_on_temp= 10;
const float hen_house_off_temp = 20;
const unsigned long half_hour_millis = 1800000;
const float water_on_temp = 33.00;
const float water_off_temp = 35.00;


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
ExtendedDallasTemperature waterDallas(onewire);
Light light(LIGHT_PIN);

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
  
  Serial.print("Connecting to WiFi...");
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

  pinMode(HEATER_PIN,OUTPUT);

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

  server.begin();
  TimeReset(); // rest time to next reset
  // rest daily highs and lows
  resetHighLow();

  #ifdef DEBUG
  Serial.println("End of Setup");
  #endif
}

void loop() 
{
  time_now = millis();
  //sample the sensors
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

        light.setFlag(SUNRISEFLAG); 
    
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

        light.clearFlag(SUNRISEFLAG);
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

  if (time_now > millis_at_midnight)
  {
    // reset variables and do whatever else you need to do
    TimeReset(); // rest time to next reset

    // rest daily highs and lows
    resetHighLow();
  }
  time_now = millis();
  if( time_now >= millis_at_sunrise  && time_now < millis_at_sunrise)
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
  else
  {
    #ifdef DEBUG
    uint8_t temp = light.getflags();
    #endif

    light.clearFlag(SUNRISEFLAG);
    #ifdef DEBUG
    if(temp != light.getflags())
     Serial.println("Light OFF");
    #endif 
  }

  if(time_now >= millis_at_sunset && time_now < millis_at_9PM)
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
    
    #ifdef DEBUG
    if(temp != light.getflags())
     Serial.println("Light Off");
    #endif 
  }

  delay(500);
}

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
    sunsetTime = (sunsetHour * 3600) + (sunsetMinute * 60);
    
    doc.clear();
    #ifdef DEBUG
    Serial.print("Sunrise Time");
    Serial.println(sunrise);
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
    int rawOffset = doc["raw_offset"]; // offset in second from utc time based on time zone of ip
    unixTime = unixTime + rawOffset; //currnet time where time is from ip
    // calculate milliseconds until midnight
    //24*60*60 (hours in day * minutes * seconds)
    unsigned long secondsSinceMidnight = unixTime % 86400;  //last time it was midnight 
    unsigned long secondsUntilMidnight = 86400 - secondsSinceMidnight; // next time in seconds  until it is midnight 
    unsigned long millistime = millis(); 
    millis_at_midnight = millistime + (secondsUntilMidnight * 1000); // how many millis the millis should be at mid night 

    //6*60*60 (6 am * Minutes * seeconds)
    millis_at_6AM= millistime + ((SIXAMSECONDS-secondsSinceMidnight) * 1000);  // millis() at 6am to turn on light 
    millis_at_9PM = millistime + ((NINEPMSECONDS-secondsSinceMidnight) * 1000); //mills at 9 PM  to trun off light 
    
    //callcuate what millis will be at sun rise + 30 minutes to turn off light  
    millis_at_sunrise = millistime + ((sunriseTime + rawOffset- secondsSinceMidnight)*1000) + half_hour_millis;
    //calculat what millis will be at sun set - 30 minutes to turn on light
    millis_at_sunset =  millistime + ((sunsetTime + rawOffset - secondsSinceMidnight)*1000) - half_hour_millis;

    // output for debugging
    #ifdef DEBUG
    Serial.println("Current Unix Time: " + String(unixTime));
    Serial.println("Milliseconds until midnight: " + String(secondsUntilMidnight * 1000));
    Serial.println("Millis at midnight: " + String(millis_at_midnight));
    #endif
    doc.clear();
  }

  http.end();
}

bool getSensors()
{

  if(!DHTindoor.sampleData(indoor))
    return false;

  if(!DHToutdoor.sampleData(outdoor))
    return false;
  
  if(!waterDallas.sampleData(water))
    return false;

  return true;
}

String handleJson()
{
  String json = "{";
  json += "\"temperature\":" + String(indoor.temperature) + ",";
  json += "\"humidity\":" + String(indoor.humidity) + ",";
  json += "\"indoorHighTemp\":" + String(indoor.highTemperature) + ",";
  json += "\"indoorLowTemp\":" + String(indoor.lowTemperature) + ",";
  json += "\"indoorHighHum\":" + String(indoor.highHumidity) + ",";
  json += "\"indoorLowHum\":" + String(indoor.lowHumidity) + ",";
  json += "\"outdoorTemperature\":" + String(outdoor.temperature) + ",";
  json += "\"outdoorHumidity\":" + String(outdoor.humidity) + ",";
  json += "\"outdoorHighTemp\":" + String(outdoor.highTemperature) + ",";
  json += "\"outdoorLowTemp\":" + String(outdoor.lowTemperature) + ",";
  json += "\"outdoorHighHum\":" + String(outdoor.highHumidity) + ",";
  json += "\"outdoorLowHum\":" + String(outdoor.lowHumidity) + ",";
  json += "\"outdoorLowHum\":" + String(water.temperature) + ",";
  json += "\"outdoorLowHum\":" + String(water.highTemperature) + ",";
  json += "\"outdoorLowHum\":" + String(outdoor.lowTemperature) + ",";
  json += "}";
  return json;
}

//reset low and high 
void resetHighLow()
{
  indoor = { 0, 0, 0, 0, 0, 0 };
  outdoor = {0, 0, 0, 0, 0, 0 };
  water = {0,0,0};
}