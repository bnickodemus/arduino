int sensorPin = 0;    // The potentiometer is connected to A0                     
int ledPin = 5;       // The LED is connected to digital pin 5
int onBtnPin = 4;
int offBtnPin = 3;

void setup()
{
  //Serial.begin(9600); // testing with Serial.print()
  
  // Set up the led pin to be an output
  pinMode(ledPin, OUTPUT);

  // Set up the pushbutton pins to be an input
  pinMode(onBtnPin, INPUT);
  pinMode(offBtnPin, INPUT);
}

void loop()
{
  int sensorValue = analogRead(sensorPin); 

  if (digitalRead(onBtnPin) == HIGH) { // onBtn was pressed
    analogWrite(ledPin, sensorValue / 4); // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  }
  else if (digitalRead(offBtnPin) == HIGH) { // offBtn was pressed
    digitalWrite(ledPin, LOW); // turn off led
  }
}

