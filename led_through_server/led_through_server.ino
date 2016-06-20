/*
  To run this program start up the arduino with Led on 2 and photo 
  resistor on A0
  Run the Serial Monitor and enter the following command in the 
  terminal: telnet (ip address here you got from serial monitor)
*/

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x04
};
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192,168,1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// telnet defaults to port 23
EthernetServer server(23);
boolean gotAMessage = false; // whether or not you got a message from the client yet

int ledPin = 2;

String commandString;

void setup() {
  pinMode(ledPin, OUTPUT); //sets digital pin as output
  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, subnet);
  //start listening for clients
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Trying to get an IP address using DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // initialize the Ethernet device not using DHCP:
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
  }
  
  Serial.print("Chat server address:");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // wait for a new client:
  EthernetClient client = server.available();
  bool alreadyConnected = false;

  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clear out the input buffer
      client.flush();
      
      commandString = ""; // clear the commandString variable 

      server.println("--> Please type your command and hit Return...");
      alreadyConnected = true;
    }

    while (client.available()) {
     // read the bytes incoming from the client
     char newChar = client.read();

     // ASCII returned
     if (newChar == 0x0D) // if a 0x0D is received, a Carriage Return, then evaluate
     {
      server.print("Received this command: ");
      server.println(commandString);
      processCommand(commandString);
     } else 
     {
      Serial.println(newChar);
        commandString += newChar;
     }
     
    } 
  }
}

void processCommand(String command)
{
  server.println("Processing command");
  server.println(command);

  if (command.indexOf("photo") > -1){
    Serial.println("Photo command received");
    server.print("Reading from photoresistor: ");
    server.println(analogRead(A0)); // print the integer returned by analog
    commandString = "";
    return;
  }

  if (command.indexOf("ledon") > -1){
    server.println("LED On command recieved");
    digitalWrite(ledPin, HIGH); //sets the LED on
    server.println("LED was turned on");
    commandString = "";
    return;
  }

    if (command.indexOf("ledoff") > -1){
    server.println("LED Off command recieved");
    digitalWrite(ledPin, LOW); //sets the LED off
    server.println("LED was turned off");
    commandString = "";
    return;
  }

  commandString = "";
  instructions();
}

void instructions()
{
  server.println("I don't understand");
  server.println("Please use one of these commands:");
  server.println("* photo, to get a reading from the photoresistor");
  server.println("* ledon, to turn on the LED");
  server.println("* ledoff, to turn off the LED");
}



