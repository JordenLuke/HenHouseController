#ifndef EXT_DALLAS_H
#define EXT_DALLAS_H

#include<DallasTemperature.h>
#include<OneWire.h>

typedef struct DallasData
{
    float temperature;
    float highTemperature;
    float lowTemperature;
}DallasData;

class ExtendedDallasTemperature{
private:
    DallasTemperature sensor;
    DallasData dallasData;

public:
    ExtendedDallasTemperature(OneWire* onewire);
    bool sampleData(DallasData &data);
    DallasData getClimateData() const;
    void begin();
};
//end of EXT_DALLAS_H
#endif
