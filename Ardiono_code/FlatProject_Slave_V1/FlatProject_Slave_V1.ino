#include <EasyTransfer.h>

#define RS485ControlPin 2
EasyTransfer ET;

// Structure for receiving data
struct DataPacket {
  int slaveID;           // Target slave ID
  char command[10];      // Command from master
};
DataPacket dataFromMaster;

// Structure for sending data
struct FeedbackPacket {
  int slaveID;           // Slave ID responding
  float temperature;
  float humidity;
};
FeedbackPacket dataToMaster;

const int slaveID = 1;   // Unique ID for this slave

void setup() {
  Serial.begin(9600);
  pinMode(RS485ControlPin, OUTPUT);
  digitalWrite(RS485ControlPin, LOW); // Set to receive mode
  ET.begin(details(dataFromMaster), &Serial);
}

void respondToMaster() {
  dataToMaster.slaveID = slaveID;
  dataToMaster.temperature = 25.5; // Example temperature
  dataToMaster.humidity = 60.0;    // Example humidity
  digitalWrite(RS485ControlPin, HIGH); // Set to transmit mode
  delay(1); // Ensure proper control line switching
  ET.sendData();
  digitalWrite(RS485ControlPin, LOW); // Back to receive mode
}

void handleCommand() {
  if (dataFromMaster.slaveID == slaveID) { // Check if command is for this slave
    if (strcmp(dataFromMaster.command, "SENSOR") == 0) {
      respondToMaster(); // Send temperature/humidity data
    } else if (strcmp(dataFromMaster.command, "FAN_ON") == 0) {
      // Example: Start fan
      Serial.println("Fan started.");
    } else if (strcmp(dataFromMaster.command, "OPEN_FLAP") == 0) {
      // Example: Open flaps
      Serial.println("Flaps opened.");
    }
  }
}

void loop() {
  if (ET.receiveData()) {
    handleCommand(); // Process the received command
  }
}
