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
const char *world_time_url = "http://worldtimeapi.org/api/ip";
const int Smaple_time = 6000;
const float hen_house_on_temp= 10;
const float hen_house_off_temp = 20;
const unsigned long half_hour_millis = 1800000;

const char* sunriseSunsetUrl = "https://api.sunrise-sunset.org/json?lat=41.730676&lng=-111.834894";


// golbal variables
unsigned long time_now = 0; //updatd everytime it goes through the loop 
unsigned long time_last = 0; //keeps track of the last time the sensor were scanned 
unsigned long millis_at_midnight = 0; // Keeps track of when to reset clocks for turning ligt on and off and highs and lows
unsigned long millis_at_sunrise= 0; // what time to turn on light before sunrise
unsigned long millis_at_sunset =0; //when to turn it on bfore sunset
unsigned long millis_at_6AM =0;
unsigned long millis_at_9PM=0;


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
  
}

void loop() 
{
  time_now = millis();
  //sample the sensors
  if((time_now - time_last) > Smaple_time)
  {
    if(getSensors())
    {
      time_last = millis();
      if(indoor.temperature < hen_house_on_temp)
      {
        light.setFlag(TEMPFLAG);
      }
      
      if(indoor.temperature > hen_house_off_temp)
      {
        light.clearFlag(TEMPFLAG);
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

  if( millis() >= (millis_at_sunrise - half_hour_millis) && millis() < (millis_at_sunrise + half_hour_millis))
  {
    light.setFlag(SUNRISEFLAG);
  }
  else
  {
    light.clearFlag(SUNRISEFLAG);
  }

  if(millis() > (millis_at_sunset- half_hour_millis) && millis() < millis_at_9PM)
  {
    light.setFlag(SUNSETFLAG);
  }
  else
  {
    light.clearFlag(SUNSETFLAG);
  }

  delay(500);
}

void TimeReset()
{
  time_t sunriseTime;
  time_t sunsetTime; 
  http.begin(sunriseSunsetUrl);
  int http_code = http.GET();
  if (http_code > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    const char* sunrise = doc["results"]["sunrise"].as<char*>();
    const char* sunset = doc["results"]["sunset"].as<char*>();

    // Parse sunrise and sunset times
    struct tm tmSunrise, tmSunset;
    strptime(sunrise, "%I:%M:%S %p", &tmSunrise);
    strptime(sunset, "%I:%M:%S %p", &tmSunset);

    // Calculate light on and off times based on sunrise and sunset
    sunriseTime = mktime(&tmSunrise);
    sunsetTime = mktime(&tmSunset);
  }
  else
  {
    http.end();
    client.stop();
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
    unsigned long unix_time = doc["unixtime"];
    int raw_offset = doc["raw_offset"]; // offset in second from utc time based on time zone of ip
    unix_time = unix_time + raw_offset;
    // calculate milliseconds until midnight
    //24*60*60 (hours in day * minutes * seconds)
    unsigned long seconds_since_midnight = unix_time % 86400;
    unsigned long seconds_until_midnight = 86400 - seconds_since_midnight;
    unsigned long millistime = millis();
    millis_at_midnight = millistime + (seconds_until_midnight * 1000);

    //6*60*60 (6 am * Minutes * seeconds)
    millis_at_6AM= millistime + ((SIXAMSECONDS-seconds_since_midnight) * 1000);
    millis_at_9PM = millistime + ((NINEPMSECONDS-seconds_since_midnight) * 1000);

    millis_at_sunrise = millistime + ((sunriseTime + raw_offset- seconds_since_midnight)*1000);
    millis_at_sunset =  millistime +((sunsetTime + raw_offset - seconds_since_midnight)*1000);

    // output for debugging
    #ifdef DEBUG
    Serial.println("Current Unix Time: " + String(unix_time));
    Serial.println("Milliseconds until midnight: " + String(seconds_until_midnight * 1000));
    Serial.println("Millis at midnight: " + String(millis_at_midnight));
    #endif
    doc.clear();
  }

  http.end();

  client.stop();
}

bool getSensors()
{

  if(DHTindoor.sampleData(indoor))
    return false;

  if(DHToutdoor.sampleData(outdoor))
    return false;
  
  if(waterDallas.sampleData(water))
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