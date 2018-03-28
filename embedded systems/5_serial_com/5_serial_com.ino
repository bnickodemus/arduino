// By: Nathan and Broc
#include <MsTimer2.h>

#define PHOTOCELL A0
#define POT A1

int sampleRate;
int incoming = 0;   // for incoming serial data
bool messageSent = false;
bool pot = false;
int sensorValue;

void setup() {
  Serial.begin(9600);
  Serial.flush();
 
}

void runTimer() {
  if (Serial.available() > 0) {
    incoming = Serial.read();
    if (incoming == 'S') {
      MsTimer2::stop();
    }
  }
  
  if (pot) {
    sensorValue = analogRead(POT);
    Serial.println(sensorValue);
  } else {
    sensorValue = analogRead(PHOTOCELL);
    Serial.println(sensorValue);
  }
}

void setTimer(int sampingRate) {
  MsTimer2::set(sampingRate, runTimer);
  MsTimer2::start();
}

void loop() {

  while (!messageSent) {
    if (Serial.available() > 0) {
      incoming = Serial.read();
    }

    switch(incoming) {
      case 'A':
        messageSent = true;
        pot = true;
        setTimer(200);
        break;
      case 'B':
        messageSent = true;
        pot = true;
        setTimer(20);
        break;
      case 'C':
        messageSent = true;
        setTimer(200);
        break;
      case 'D':
        messageSent = true;
        setTimer(200);
        break; 
      default:
        break;
    }
  }
}

