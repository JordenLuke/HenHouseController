#include "ExtendedDHT.h"
#include <DHT.h>
#include <cmath> // This header already includes the isnan() function


ExtendedDHT::ExtendedDHT(int pin, int type) : dhtSensor(pin, type) {
    // Initialize climateData here if needed
    climateData = {0,0,0,0,0,0};
}

bool ExtendedDHT::sampleData(Climate &climate) {
        // Sample data from the DHT sensor
        float temp = dhtSensor.readTemperature();
        float hum = dhtSensor.readHumidity();

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
