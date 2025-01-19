#include <EasyTransfer.h>
#include <SoftwareSerial.h>

#define RS485ControlPin 2
EasyTransfer ETin, ETout;

SoftwareSerial Serial_rs485(10, 11);  //RX,TX
// Structure for receiving data
struct DataPacket {
  int slaveID;           // Target slave ID
  char command[10];      // Command from master
};
DataPacket RXdataFromMaster;

// Structure for sending data
struct FeedbackPacket {
  int slaveID;           // Slave ID responding
  float temperature;
  float humidity;
};
FeedbackPacket TXdataToMaster;

const int slaveID = 1;   // Unique ID for this slave

void setup() {
  Serial.begin(9600);
  Serial_rs485.begin(9600);
  pinMode(RS485ControlPin, OUTPUT);
  digitalWrite(RS485ControlPin, LOW); // Set to receive mode
  ETin.begin(details(RXdataFromMaster), &Serial_rs485);
  ETout.begin(details(TXdataToMaster), &Serial_rs485);

  
}

void respondToMaster() {
  TXdataToMaster.slaveID = slaveID;
  TXdataToMaster.temperature = 25.5; // Example temperature
  TXdataToMaster.humidity = 60.0;    // Example humidity
  digitalWrite(RS485ControlPin, HIGH); // Set to transmit mode
  delay(10); // Ensure proper control line switching
  ETout.sendData();
  delay(10); // Ensure proper control line switching
  digitalWrite(RS485ControlPin, LOW); // Back to receive mode
  Serial.println("Reply");
}

void handleCommand() {
  if (RXdataFromMaster.slaveID == slaveID) { // Check if command is for this slave
  //Serial.println("data received");
  //Serial.println(RXdataFromMaster.command);
//respondToMaster();
    if (strcmp(RXdataFromMaster.command, "SENSOR") == 0 ) {
      respondToMaster(); // Send temperature/humidity data
      Serial.println("Reply to master");
    } else if (strcmp(RXdataFromMaster.command, "FAN_ON") == 0) {
      // Example: Start fan
      Serial.println("Fan started.");
    } else if (strcmp(RXdataFromMaster.command, "OPEN_FLAP") == 0) {
      // Example: Open flaps
      Serial.println("Flaps opened.");
    }
  }
}

void loop() {
  if (ETin.receiveData()) {
    handleCommand(); // Process the received command
  }
  //delay for good measure
  delay(100);
}
