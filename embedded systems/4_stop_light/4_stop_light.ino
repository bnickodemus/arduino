// By: Brad and Broc
#include <MsTimer2.h>

#define BUTTON 2
#define LED_RED 11
#define LED_YELLOW 12
#define LED_GREEN 13
#define redState 1
#define yellowState 2
#define greenState 3
#define pendingState 4
int counter;
volatile byte buttonState;
int btnInterupt;
int lightState;

void stopLight(){
  Serial.print("counter: ");
  Serial.println(counter);
  Serial.print("button state: ");
  Serial.println(buttonState);

  switch(lightState){
    case redState:
    Serial.println("redstate");
      if (counter < 10){
        counter++;
      }
      else{ // switch to green if counter >= 10
        lightState = greenState;
        counter = 0;
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
      }
      break;
    case greenState:
      Serial.println("greenstate");
      if (buttonState == HIGH && counter >= 10){ // set to yellow if button pressed
         lightState = yellowState;
         digitalWrite(LED_GREEN, LOW);
         digitalWrite(LED_YELLOW, HIGH);
         counter = 0;
         buttonState = LOW;
      }
      else if(buttonState == HIGH && counter < 10){ // set to pending
          lightState = pendingState;
          buttonState = LOW;
      }
      else{
        if (counter < 30){
         counter++; 
        }    
      }
      break;
    case pendingState:
      Serial.println("pendingstate");
      if(counter >= 10){
        lightState = yellowState;
        // turn off green and turn on yellow
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        counter = 0;
      }
      else{
        counter++;
      }
      break;  
    case yellowState:
      Serial.println("yellowstate");
      if(counter >= 5){
        // turn off yellow turn on red
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, HIGH);
        counter = 0;
        lightState = redState;
      }
      else{
        counter++;
      }
      break;
    default:
      break;
  }
}

void buttonPress() {
  buttonState = HIGH;
}

void setup() {
  counter = 0;
  buttonState = LOW;
  btnInterupt = 0;
  lightState = redState;
  Serial.begin(115200);
  digitalWrite(LED_RED, HIGH); // init red light on
  // set pins
  pinMode(BUTTON, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonPress, HIGH);
  MsTimer2::set(1000, stopLight);
  MsTimer2::start();
}
 
void loop() {
}
