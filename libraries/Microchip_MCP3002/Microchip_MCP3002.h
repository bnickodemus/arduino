#include <Arduino.h>
#include <"SPI.h">

#pragma once

/*  
    CHANNEL_0: single ended mode (input signal on 0, no ground)
    CHANNEL_0_1: differential mode (0 is "ground", signal on 1)
*/


#define CHANNEL_0   B01100000
#define CHANNEL_1   B01110000
#define CHANNEL_0_1 B01010000
#define CHANNEL_1_0 B01000000

class Microchip_MCP3002 {
    public:
        Microchip_MCP3002(byte chip_select_pin);
        void setChannel(byte channel);
        float getSample();
    private:
        byte _channel;
        byte _pin;
        void select();
        void deselect();
};
