/*
 * Author: Jorden Luke
 * Date: [Date]
 * Description: This extends the DHT.h class to encaplate the data being retrived.
 * This file defiines the Climate struct as well as the ExtendedDHT. This allowed
 * for better organaization of the data.
 *
 * This code is released under the Yerba Mate license:
 * ----------------------------------------------------------------------------
 * "THE YERBA MATE LICENSE" (Revision 42):
 * Jorden Luke <<jorden.luke@gmail.com>> wrote this file. As long as you retain
 * this notice, you can do whatever you want with this stuff. If we meet some 
 * day, and you think this stuff is worth it, you can buy me a Yerba Mate in return. 
 * ----------------------------------------------------------------------------
 */


#ifndef EXTENDED_DHT_H
#define EXTENDED_DHT_H

#include <DHT.h>

typedef struct Climate {
    float temperature;
    float humidity;
    float highTemperature;
    float lowTemperature;
    float highHumidity;
    float lowHumidity;
}Climate;

class ExtendedDHT {
private:
    DHT dhtSensor;
    Climate climateData;

public:
    ExtendedDHT(int pin, int type);
    bool sampleData(Climate &climate);
    Climate getClimateData() const;
    void begin();
};

#endif // EXTENDED_DHT_H
