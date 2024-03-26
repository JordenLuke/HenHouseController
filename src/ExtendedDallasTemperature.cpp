
#include<ExtendedDallasTemperature.h>
#include<OneWire.h>
#include<Arduino.h>

ExtendedDallasTemperature::ExtendedDallasTemperature(OneWire *onewire):sensor(onewire)
{
    dallasData= {0.0,0};
}

bool ExtendedDallasTemperature::sampleData(DallasData &data)
{
    sensor.requestTemperaturesByIndex(0);
    float temp = sensor.getTempFByIndex(0);
    dallasData.temperature = temp;
    if(temp <= -127.00)
    {
        dallasData.temperature = NAN;
        return false; //bad read    
    }

    if (temp > dallasData.highTemperature) {
        dallasData.highTemperature = temp;
    }
    
    if (temp < dallasData.lowTemperature || dallasData.lowTemperature == 0) {
        dallasData.lowTemperature = temp;
    }
    
    data = dallasData;
    return true;
}

DallasData ExtendedDallasTemperature::getClimateData() const{
    return dallasData;
}

void ExtendedDallasTemperature::begin()
{
    sensor.begin();
}
//endoffile
