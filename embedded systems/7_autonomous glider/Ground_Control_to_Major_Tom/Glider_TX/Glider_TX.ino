//Author: Ian Riemer
//Date: 3/20/2018
//Description: Code for auto-leveling an unpowered glider which transmits its data to a base station
//    using 915 Mhz RF transceivers. This code also enables auto-trimming for stable flight, and 
//    reacts to commands sent from the base station. "Plane" may be used instead of "glider" throughout
//    the code and/or comments, but this is to refer to any fixed-wing aircraft. 

#include <Wire.h>
#include <SPI.h>
#include <RH_RF69.h>
#include <SparkFunLSM9DS1.h>
#include <RHReliableDatagram.h>
#include <ServoTimer2.h>
#include <PWMServo.h>

#define LSM9DS1_M  0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

/************ Radio Setup ***************/
#define RF69_FREQ 915.0

// Where to send packets to
#define DEST_ADDRESS   1
// change addresses for each client board, any number
#define MY_ADDRESS     2

#define RFM69_CS      13
#define RFM69_INT     2
#define RFM69_RST     3
#define LED           5
#define LEFTSERVO     6
#define RIGHTSERVO    9

// Comment out to disable printing
#define DEBUG

// Singleton instance of the radio driver
RH_RF69 rf69;

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

LSM9DS1 imu;
ServoTimer2 leftS; //This left servo is using an alternate library since PWMServo only supports pins 9 & 10,
PWMServo rightS;   //But 10 is taken for SPI. However, ServoTimer2 twiches while sending, so PWMServo is better.
                   //I hope to eliminate this issue in the future. Also, the regular servo library is incompatible
                   //with the SparkFunLSM9DS1 RF chip library since they both use Timer1.
const float alpha = 0.5; //Alpha value for IMU filtering
float axf = 0;  //These are filtered IMU values
float ayf = 0;
float azf = 0;
float gxf = 0;
float gyf = 0;
float rAdjust = 0; //Roll adjustment, mainly for left, right, center commands
float pAdjust = 0; //Pitch adjustment
float rCorrect = 0; //Roll correction for auto trimming
float pCorrect = 0; //pitch correction for auto trimming
int16_t packetnum = 0;  // packet counter, we increment per xmission
static unsigned long lastPrint = 0; // Keep track of print time
static unsigned long tempTime = 0;  // Used for counting interval between transmissions
char state = 'C';  //Char value indicating roll state, not really used at the moment
int failAcks = 0;  //Count for the number of times an 
int tModifier = 0;

void setup() 
{
  Serial.begin(115200);

  leftS.attach(LEFTSERVO);
  rightS.attach(RIGHTSERVO);
  leftS.write(servo2Val(90)); //Value has to be converted to usec for ServoTimer2
  rightS.write(90);

  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  if (!imu.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                  "work for an out of the box LSM9DS1 " \
                  "Breakout, but may need to be modified " \
                  "if the board jumpers are.");
    while (1);
  }
  Serial.println("Initialized IMU");
  delay(1000);
  imu.calibrate();

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println(F("Feather Addressed RFM69 TX Test!"));
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69_manager.init()) {
    Serial.println(F("RFM69 radio init failed"));
    while (1);
  }
  Serial.println(F("RFM69 radio init OK!"));
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println(F("setFrequency failed"));
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(14, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the base station. Both are using a defaut key atm.
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  pinMode(LED, OUTPUT);

  Serial.print(F("RFM69 radio @"));  Serial.print((int)RF69_FREQ);  Serial.println(F(" MHz"));
}


// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t data[] = "  OK";

void loop() {
  updateAccelValues();
  adjustElevons();
  
  //Send data every ~100ms
  if (millis() >= lastPrint + 998 + tModifier) {
    tempTime = millis();
    #ifdef DEBUG
      Serial.print(F("Time since last print: "));
      Serial.println(tempTime - lastPrint);
    #endif
    lastPrint = tempTime;
    
    char radiopacket[50] = "";
    strcat(radiopacket, getPitchChars());
    strcat(radiopacket, ", ");
    strcat(radiopacket, getRollChars());
    strcat(radiopacket, ", ");
    strcat(radiopacket, getMillisChars());

  #ifdef DEBUG
    printGyro();
    Serial.print(F("Sending ")); Serial.println(radiopacket);
    Serial.print(F("Adjusted Roll: ")); Serial.print(getRoll() + rAdjust + rCorrect);
    Serial.print(F(", Roll Correct: ")); Serial.println(rCorrect);
    Serial.print(F("Adjusted Pitch: ")); Serial.print(getPitch() - pCorrect);
    Serial.print(F(", Vertical Correct: ")); Serial.println(pCorrect); 
  #endif
    
    // Send a message to the DESTINATION!
    if (rf69_manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), DEST_ADDRESS)) {
      // Now wait for a reply from the server
      uint8_t len = sizeof(buf);
      uint8_t from;   
      //Wait for ACK
      if (rf69_manager.recvfromAckTimeout(buf, &len, 70, &from)) { //Elevons will not be adjusted while waiting
        buf[len] = 0; // zero out remaining string
        tModifier = 0;
        failAcks = 0;
        #ifdef DEBUG
          Serial.print(F("Got reply from #")); Serial.print(from);
          Serial.print(F(" [RSSI :"));
          Serial.print(rf69.lastRssi());
          Serial.print(F("] : "));
          Serial.println((char*)buf);
        #endif
        if( (char*)buf[0] != state){
          setState((char*)buf); //Update plane's control state
        }
        //Blink(LED, 3, 2); //blink LED 3 times, 40ms between blinks //My LED died.
      } else {
        #ifdef DEBUG
          Serial.println(F("No reply, is anyone listening?"));
        #endif
      }
    } else {
      #ifdef DEBUG
        Serial.println(F("Sending failed (no ack)"));
      #endif
      failAcks++;
      if (failAcks > 4) { //If the plane misses more than 4 acks, assume lost connection and transmit
        tModifier = 4000; //less frequently to increase time spent in control over sending & listening.
      }
    }
  }
}

//Adjust elevon positions for delta-wing aircraft (elevon = elevator + aileron)
void adjustElevons() {
  float pitch = getPitch();
  float roll = getRoll() + rAdjust; //Apply rAdjust offset to make plane roll left or right
  if (pitch > 75 || pitch < -75) { roll = 0;}

  //Adjust roll "trim" to ensure plane maintains correct roll.
  //This could theoretically compensate for slight in-flight damage 
  //that imbalances the plane. Defintely needs testing (without damage).
  /*if (roll < -8 && gxf > .01 && rCorrect < 50){
    rCorrect -= .002 * roll;
  }
  else if (roll > 8 && gxf < -.01 && rCorrect > -50){
    rCorrect -= .002 * roll;
  }
  roll -= rCorrect;*/
  roll = map(roll, 80, -80, 0, 180);

  //Adjust pitch "trim" to ensure plane actually reaches level flight
  /*if (pitch < -11 && gxf < 0 && pCorrect > -50){
    pCorrect += .003 * pitch;
  }
  else if (pitch > 5 && gxf >= 0 && pCorrect < 50){
    pCorrect += .004 * pitch;
  }
  pitch += pCorrect;*/
  pitch += gxf * 10; //respond to rotational forces
  
  //Map pitch to servos, exagerating values for stronger responses
  float pitchL = map(pitch, -90, 90, -90, 270);
  float pitchR = map(pitch, 90, -90, -90, 270);

  //Mix roll and pitch for two servo system
  float posL = (roll + pitchL) / 2;
  float posR = (roll + pitchR) / 2;
  if (posL > 180) { posL = 180; }
  if (posL < 0) { posL = 0; } 
  if (posR > 180) { posR = 180; }
  if (posR < 0) { posR = 0; } 

  leftS.write(servo2Val(posL));
  rightS.write(posR);
}

void updateAccelValues() {
  //Update imu values and perform filtering
  if ( imu.gyroAvailable() ) { imu.readGyro(); }
  if ( imu.accelAvailable() ) { imu.readAccel(); }
  gxf = imu.calcAccel(imu.gx) * alpha + (gxf * (1.0 - alpha));
  gyf = imu.calcAccel(imu.gy) * alpha + (gyf * (1.0 - alpha));
  ayf = imu.calcAccel(imu.ax) * alpha + (ayf * (1.0 - alpha));//x and y are switched since imu is rotated 90 degrees
  axf = imu.calcAccel(imu.ay) * alpha + (axf * (1.0 - alpha));//in relation to the plane
  azf = imu.calcAccel(imu.az) * alpha + (azf * (1.0 - alpha));
}

int servo2Val(int deg){  //Value map for ServoTimer2 library
  return map(deg, 0, 180, 600, 2300); //Probably need more tuning for these values
}

float getPitch(){
  float pitch = atan2(-axf, sqrt(ayf * ayf + azf * azf));
  pitch *= 180.0 / PI;
  return pitch;
}

float getRoll(){
  float roll = -atan2(ayf, azf);
  roll *= 180.0 / PI;
  return roll;
}

char * getPitchChars(){
  char pitch[10];
  dtostrf(getPitch(), 4, 2, pitch);
  return pitch;
}

char * getRollChars(){
  char roll[10];
  dtostrf(getRoll(), 4, 2, roll);
  return roll;
}

char * getMillisChars(){
  char mils[32];
  dtostrf(millis(), 4, 0, mils);
  return mils;
}

void printGyro(){
  Serial.print(F("G: "));
  Serial.print(gxf, 2);
  Serial.print(F(", "));
  Serial.print(gyf, 2);
  Serial.print(F(", "));
  Serial.print(imu.calcGyro(imu.gz), 2);
  Serial.println(" deg/s");
}

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}

//Function to set the control state of the plane
void setState(char * buf){
  char cmd = buf[0];
  switch (cmd) {
      case 'L':
        state = 'L';
        rAdjust = 33.0;
        pAdjust = 0;
        break;
      case 'C':
        state = 'C';
        rAdjust = 0;
        pAdjust = 0;
        break;
      case 'R':
        state = 'R';
        rAdjust = -33.0;
        pAdjust = 0;
        break;
      case 'V':
        state = 'V';
        rAdjust = 0;
        pAdjust = 0;
        break;
      default:
        break;
  }
}
