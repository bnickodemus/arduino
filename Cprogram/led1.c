//#ifndef F_CPU
//#define F_CPU 16000000UL // or whatever may be your frequency
//#endif

#include <avr/io.h>
#include <util/delay.h>                // for _delay_ms()
#include <stdio.h>

void delay_ms( int ms )
{
    int i;
    for (i = 0; i < ms; i++)
    {
        _delay_ms(1);
    }
}

int main(void)
{
    //DDRC = 0x01;                       // initialize port C
    uint8_t     leds =1;
    DDRB = 0xFF;
    
    FILE *inputFile;
    inputFile = fopen("file.txt", "r");
    
    //read file into array
    int numberArray[15];
    int i = 0;
    
    if (inputFile == NULL)
    {
        PORTB = 0xff;
        _delay_ms(10000);
    }
    
    for (i = 0; i < 15; i++)
    {
        fscanf(inputFile, "%d,", &numberArray[i] );
    }
    
    for (i = 0; i < 15; i++)
    {
        //printf("Number is: %d\n\n", numberArray[i]);
    }
    fclose(inputFile);

    
    
    int counter = 0; // < 14
    
    while(1)
    {
        /*
        PORTB = 0xff;
        _delay_ms(500);
        PORTB = 0x00;
        _delay_ms(500);
        */
        
        PORTB = 0xff;
        delay_ms(numberArray[counter]);
        PORTB = 0x00;
        delay_ms(numberArray[counter]);
        counter = counter++;
        
        PORTB = 0xff;
        delay_ms(numberArray[counter]);
        PORTB = 0x00;
        delay_ms(numberArray[counter]);
        counter = counter++;

        PORTB = 0xff;
        delay_ms(numberArray[counter]);
        PORTB = 0x00;
        delay_ms(numberArray[counter]);
        counter = counter++;
        
        PORTB = 0xff;
        delay_ms(numberArray[counter]);
        PORTB = 0x00;
        delay_ms(numberArray[counter]);
        counter = counter++;
        
        PORTB = 0xff;
        delay_ms(numberArray[counter]);
        PORTB = 0x00;
        delay_ms(numberArray[counter]);
        counter = counter++;
        
        counter = 0;
        
        // LED on
        //PORTC = 0b00000001;            // PC0 = High = Vcc
        //_delay_ms(500);                // wait 500 milliseconds

        //LED off
        //PORTC = 0b00000000;            // PC0 = Low = 0v
        //_delay_ms(500);                // wait 500 milliseconds
    }
}
