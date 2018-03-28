#include <SPI.h>
#include <Microchip_MCP3002.h>

Microchip_MCP3002 adc(4);

void setup()
{
  Serial.begin(115200);
  
  //channel 0 on "bottom" (GND)
  //channel 1 on "top" input signal
  adc.setChannel(CHANNEL_0_1);  
}

void loop() 
{
  //get a voltage and print.
  float f = adc.getSample();
  Serial.println(f);
  
  //wait 100 ms
  delay(100);
}
