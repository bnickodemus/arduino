#include "Microchip_MCP3002.h"

Microchip_MCP3002::Microchip_MCP3002(byte chip_select_pin)
{
    //set the chip select pin
    _pin = chip_select_pin;

    //enable pin as digital output 
    pinMode(_pin, OUTPUT);

    //deselect chip
    digitalWrite(_pin, HIGH);

    //initialize SPI
    SPI.begin();
}

//use the #define constants
void Microchip_MCP3002::setChannel(byte channel)
{
    _channel = channel;
}

float Microchip_MCP3002::getSample()
{
    float voltage;
    byte msb, lsb;
    word w;

    select();

    //transfer command to select channel 
    //  get back the MSBs 
    msb = SPI.transfer(_channel);

    //send "don't care" bits to get LSBs
    lsb = SPI.transfer(0xFF);
    w = word(msb, lsb);
    w &= 0x03FF;

    deselect();

    //convert to voltage
    voltage = (w * 5.0) / 1024.0;
    return voltage;
}

void Microchip_MCP3002::select()
{
    //select chip
    digitalWrite(_pin, LOW);
    
    //reset SPI bus
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE1);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
}

void Microchip_MCP3002::deselect()
{
    //deselect chip
    digitalWrite(_pin, HIGH);
}