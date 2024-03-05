
#include<ExtendedDallasTemperature.h>
#include<OneWire.h>

ExtendedDallasTemperature::ExtendedDallasTemperature(OneWire onewire):sensor(&onewire)
{
    dallasData= {0.0,0};
}

bool ExtendedDallasTemperature::sampleData(DallasData &data)
{
    float temp = sensor.getTempCByIndex(0);
    if(temp == -127.00)
        return false; //bad read 
    
    dallasData.temperature = temp;

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
