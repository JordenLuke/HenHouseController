/*
 * Author: Jorden Luke
 * Date: 08-Mar-2024
 * Description: This is a defftion of the Light class. This defines the light
 * object which encapslates a pin used to control a light. The prupose of 
 * this object is allow multipal sources to control light with out having one
 * source turn the light off it has anoter source that needs it on. It is to 
 * avoid turn on off the relay mutlipe times through the loop. 
 *
 * This code is released under the Yerba Mate license:
 * ----------------------------------------------------------------------------
 * "THE YERBA MATE LICENSE" (Revision 42):
 * Jorden Luke <<jorden.luke@gmail.com>> wrote this file. As long as you retain
 * this notice, you can do whatever you want with this stuff. If we meet some 
 * day, and you think this stuff is worth it, you can buy me a Yerba Mate in return. 
 * ----------------------------------------------------------------------------
 */

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