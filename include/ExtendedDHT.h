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
