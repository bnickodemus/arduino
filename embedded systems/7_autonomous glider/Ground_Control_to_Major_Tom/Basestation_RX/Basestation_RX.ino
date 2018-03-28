//Author: Ian Riemer
//Date: 3/20/2018
//Description: Wireless relay between a RF connection and a bluetooth connection,
//  intended to recieve data from the RF and forward that to a serial connection.
//  Then it will recieve a response from the serial connection containing a command,
//  which will then be send back over RF (to a hobby aircraft).

#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>

/************ Radio Setup ***************/
#define RF69_FREQ 915.0

// who am i? (server address) 
#define MY_ADDRESS     1

#define RFM69_CS      13
#define RFM69_INT     2
#define RFM69_RST     5
#define LED           9

// Comment out to disable printing. DEBUG will screw up bluetooth comms, terminal use only
//#define DEBUG

// Singleton instance of the radio driver
RH_RF69 rf69;

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission
bool isCmd = false;
int aCmd = 0;
char cCmd = 'C';

void setup() 
{
  Serial.begin(9600);

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  commands();

  #ifdef DEBUG
    Serial.println("Feather Addressed RFM69 RX Test!");
    Serial.println();
  #endif

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69_manager.init()) {
    #ifdef DEBUG
      Serial.println("RFM69 radio init failed");
    #endif
    while (1);
  }
  #ifdef DEBUG
    Serial.println("RFM69 radio init OK!");
  #endif
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    #ifdef DEBUG
      Serial.println("setFrequency failed");
    #endif
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  #ifdef DEBUG
    Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
  #endif
}

// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop() {
  if (rf69_manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    delay(20);
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
      buf[len] = 0; // zero out remaining string

      char data[0];
      data[0] = cCmd;
      if (!rf69_manager.sendtoWait(data, 1, from)){ //Respond immediately so plane doesn't have to wait
        #ifdef DEBUG
          Serial.println("Sending failed (no ack)");
        #endif
      }
      
      #ifdef DEBUG
        Serial.print("Got packet from #"); Serial.print(from);
        Serial.print(" [RSSI :");
        Serial.print(rf69.lastRssi());
        Serial.print(cCmd);
        Serial.print(" : ");
      #endif

      //Send data to phone over bluetooth serial connection
      Serial.println((char*)buf);
      
      Blink(LED, 2, 3); //Blink to indicate msg reception
      
      commands(); //Update current command variable for plane (does not send it)
    }
  }
}

void commands(){ //Listen for response from phone and update cCmd to match phone's command
  for(int i = 0; i < 140; i++){ // Wait ~700ms for response
    if(Serial.available() > 0) {
      int data = Serial.read();
      switch (data) {
        case 48:
          cCmd = 'L';
          break;
        case 49:
          cCmd = 'C';
          break;
        case 50:
          cCmd = 'R';
          break;
        default:
          break;
      }
      break;
    }
    delay(5);
  }
}

//LED blinker
void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
