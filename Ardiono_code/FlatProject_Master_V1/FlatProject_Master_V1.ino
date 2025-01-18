#include <EasyTransfer.h>

#define RS485ControlPin 2
EasyTransfer ET;

// Structure for sending data
struct DataPacket {
  int slaveID;           // Target slave ID
  char command[10];      // Command from master
};
DataPacket dataToSlave;

// Structure for receiving data
struct FeedbackPacket {
  int slaveID;           // Slave ID responding
  float temperature;
  float humidity;
};
FeedbackPacket dataFromSlave;

const int TIMEOUT = 500; // Timeout for response in milliseconds
const int NUM_SLAVES = 4; // Total number of slaves
int currentSlave = 1;    // Start with Slave 1

void setup() {
  Serial.begin(9600); // Debugging
  Serial1.begin(9600); // RS485 connection
  pinMode(RS485ControlPin, OUTPUT);
  digitalWrite(RS485ControlPin, LOW); // Set to receive mode
  ET.begin(details(dataToSlave), &Serial1);
}

void sendCommand(int slaveID, const char* command) {
  dataToSlave.slaveID = slaveID;
  strncpy(dataToSlave.command, command, sizeof(dataToSlave.command));
  digitalWrite(RS485ControlPin, HIGH); // Set to transmit mode
  delay(1); // Ensure proper control line switching
  ET.sendData();
  digitalWrite(RS485ControlPin, LOW); // Back to receive mode
  Serial.print("Sent command to Slave ");
  Serial.println(slaveID);
}

bool receiveFeedback(int expectedSlaveID) {
  unsigned long startTime = millis();
  while (millis() - startTime < TIMEOUT) {
    if (ET.receiveData()) {
      if (dataFromSlave.slaveID == expectedSlaveID) { // Validate response is from the correct slave
        Serial.print("Response from Slave ");
        Serial.println(dataFromSlave.slaveID);
        Serial.print("Temperature: ");
        Serial.println(dataFromSlave.temperature);
        Serial.print("Humidity: ");
        Serial.println(dataFromSlave.humidity);
        return true;
      } else {
        Serial.println("Response from unexpected slave.");
      }
    }
  }
  Serial.println("No response within timeout.");
  return false;
}

void loop() {
  // Send command to the current slave and wait for a response
  sendCommand(currentSlave, "SENSOR");
  if (receiveFeedback(currentSlave)) {
    Serial.println("Data successfully received.");
  } else {
    Serial.println("Failed to receive data from Slave.");
  }

  // Move to the next slave in the sequence
  currentSlave++;
  if (currentSlave > NUM_SLAVES) {
    currentSlave = 1; // Loop back to the first slave
  }

  delay(1000); // Wait before polling the next slave
}
