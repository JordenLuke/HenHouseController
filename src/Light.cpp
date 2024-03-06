#include"Light.h"
#include <Arduino.h>

Light::Light(uint8_t pin)
{
    this->pin =pin;
    status = 0x0000;
}

bool Light::isLightOn()
{
    return (status & STATUS) != 0 ? true : false;
}

uint8_t Light::setFlag(uint8_t flag)
{
    status &= ~(STATUS|ENABLED); //Don't allow the Enable bit and Satus bit to be set by flags
    status|=flag;
    
    //check to see if the Light is Enabled
    if((status&ENABLED)!= 0)
    {
        if((status&LIGHTONMASK))
        {
            status = status | STATUS; //set status bit to show light is on
            digitalWrite(pin,ON); //turn light on 
        }
    }
    return status;
}
uint8_t Light::clearFlag(uint8_t flag)
{
    flag &= ~(STATUS|ENABLED);
    status &= ~flag;
    if((status&ENABLED))
    {
         if((status&LIGHTONMASK )== 0)
         {
            status &= ~STATUS;
            digitalWrite(pin,OFF);
         }
    }
    return status;
}
bool Light::isEnabled()
{
    return (status & ENABLED) != 0? true : false;
}
void Light::enable()
{
    status |= ENABLED;
    if((status&LIGHTONMASK))
    {
            status = status | STATUS; //set status bit to show light is on
            digitalWrite(pin,ON); //turn light on 
    }
}
   
void Light::disable()
{
    status &= ~STATUS;
    digitalWrite(pin,OFF);
}
void Light::begin()
{
    pinMode(pin,OUTPUT);
}
