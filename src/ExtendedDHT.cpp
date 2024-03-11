/*
 * Author: Jorden Luke
 * Date: [Date]
 * Description: This extends the DHT.h class to encaplate the data being retrived.
 * This file implentes ExtendedDHT. This class allowed for better code and data
 * organization.
 * 
 * This code is released under the Yerba Mate license:
 * ----------------------------------------------------------------------------
 * "THE YERBA MATE LICENSE" (Revision 42):
 * Jorden Luke <<jorden.luke@gmail.com>> wrote this file. As long as you retain
 * this notice, you can do whatever you want with this stuff. If we meet some 
 * day, and you think this stuff is worth it, you can buy me a Yerba Mate in return. 
 * ----------------------------------------------------------------------------
 */

#include "ExtendedDHT.h"
#include <DHT.h>
#include <cmath> // This header already includes the isnan() function
#include <Arduino.h>


ExtendedDHT::ExtendedDHT(int pin, int type) : dhtSensor(pin, type) {
    // Initialize climateData here if needed
    climateData = {0,0,0,0,0,0};
}

bool ExtendedDHT::sampleData(Climate &climate) {
        // Sample data from the DHT sensor
        float temp = dhtSensor.readTemperature();
        float hum = dhtSensor.readHumidity();

        Serial.println("Temp: " + String(temp));

        // Check if the data is valid (not NaN)
        if (!isnan(temp) && !isnan(hum)) {
            // Update climateData
            climateData.temperature = temp;
            climateData.humidity = hum;

            // Update high and low values if necessary
            if (temp > climateData.highTemperature) {
                climateData.highTemperature = temp;
            }
            if (temp < climateData.lowTemperature || climateData.lowTemperature == 0) {
                climateData.lowTemperature = temp;
            }
            if (hum > climateData.highHumidity) {
                climateData.highHumidity = hum;
            }
            if (hum < climateData.lowHumidity || climateData.lowHumidity ==0 ) {
                climateData.lowHumidity = hum;
            }

            // Return true to indicate successful data sampling
            climate = climateData;
            return true;
        }

        // Return false to indicate invalid data
        return false;
}

Climate ExtendedDHT::getClimateData() const {
        return climateData;
}

void ExtendedDHT::begin()
{
    dhtSensor.begin();
}
//endoffile
