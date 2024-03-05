#ifndef LIGHT_H
#define Light_H
#include <Arduino.h>

#define ON false
#define OFF true

//flag defines
#define TEMPFLAG     0b000001 //flag for setting 
#define SUNRISEFLAG  0b000010
#define SUNSETFLAG   0b000100
#define OVERRIDEON   0b001000  //forces ligt to be on if enaled 
#define ENABLED      0b010000  //
#define STATUS       0b100000  //used to denote if the light is on or off
#define LIGHTONMASK  0b001111

class Light{
    private:
    
    uint8_t pin;
    uint8_t status;

    public:
    Light(uint8_t pin);
    bool isLightOn();
    bool isEnabled();
    void enable();
    void disable();
    uint8_t setFlag(uint8_t);
    uint8_t clearFlag(uint8_t);
    uint8_t getflags();
    void begin();
};
#endif