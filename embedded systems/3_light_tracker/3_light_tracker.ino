#include <Servo.h>
#define PHOTORESPIN0 0  // The photo resistor is connected to A1
#define PHOTORESPIN1 1  // The photo resistor is connected to A1
#define PHOTORESPIN2 2  // The photo resistor is connected to A3
#define SERVOPIN 9     // The servo is connected to pin 9 

int pos = 90;
Servo myservo;  // create servo object to control a servo

void setup() {
  Serial.begin(9600);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  int light1 = analogRead(PHOTORESPIN0);
  light1 = map(light1, 0, 1023, 0, 180);
  int light3 = analogRead(PHOTORESPIN2);
  light3 = map(light3, 0, 1023, 0, 180);

  int diff = light1 - light3;
  if (diff > 15 || diff < -15) 
  {
    Serial.println(diff);
    if (light1 > light3)
    {    
      if (pos > 2)
        pos -= 2;
      myservo.write(pos); // 0 - 180
      delay(4); 
    }
    else if (light1 < light3)
    {   
      if (pos < 178)
        pos += 2;
      myservo.write(pos); // 0 - 180
      delay(4); 
    }
  }
}
