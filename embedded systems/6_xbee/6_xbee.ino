// By Brad, Broc, Jack
#include <MsTimer2.h>

#define LIGHT_SENSOR A1
#define SAMPLE_RATE 100
#define SYNC 'S'
#define START 'R'
#define STOP 'T'
#define ID 3
#define BUFFER_SIZE 3

int counter = 0;
int buffer1Index = 0;
int buffer2Index = 0;
int buffer1[BUFFER_SIZE][2];
int buffer2[BUFFER_SIZE][2];
bool bufferSwitch = false;

void setup() {
  Serial.begin(57600);
  Serial.flush();
  pinMode(LIGHT_SENSOR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // set the timer function
  MsTimer2::set(SAMPLE_RATE, lightSensor);
}
void fillBuffer(int b[BUFFER_SIZE][2], int &index){
    // store the sensor and counter
    b[index][1] = analogRead(LIGHT_SENSOR);
    b[index][0] = counter;
    index++;
}


void readBuffer(int b[BUFFER_SIZE][2], int &index){
  int i = 0;
  while(i < index){
    Serial.print(ID);
    Serial.print(", ");
    Serial.print(b[i][0]);
    Serial.print(", ");
    Serial.println(b[i][1]);   
    i++;
  }
  index = 0;
}

void lightSensor(){
  // always fill buffer1 or buffer2 at the sampling rate
  if(bufferSwitch){   
    fillBuffer(buffer1, buffer1Index);
  } else {
    fillBuffer(buffer2, buffer2Index);
  }
  // print the filled buffer
  if (counter % 3 == 0){
    if(bufferSwitch){
      readBuffer(buffer1, buffer1Index);
    } else {
      readBuffer(buffer2, buffer2Index);
    }
    bufferSwitch = !bufferSwitch;
  }
  counter++;
}

void loop() {
  
  if (Serial.available() > 0){
    char cmd = Serial.read();
    Serial.print(cmd);

    // switch on the recieved cmd
    switch(cmd){     
     case SYNC: // reset the counter
       MsTimer2::stop();
       counter = 0;
       MsTimer2::start();
       break;
     case START: // start the timer
       MsTimer2::start();
       break;
     case STOP: // stop the timer
       MsTimer2::stop();
       break;
     default:
       Serial.println("Unrecognized command");
    }
  }
}


