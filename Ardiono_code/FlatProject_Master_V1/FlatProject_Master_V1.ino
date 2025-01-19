#include <EasyTransfer.h>


#define RS485ControlPin 2
EasyTransfer ETin, ETout;

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
const int NUM_SLAVES = 2; // Total number of slaves
const int MAX_RETRIES = 5;    // Maximum number of retries for a slave

int currentSlave = 1;    // Start with Slave 1
bool slaveStates[NUM_SLAVES]; // Array to track operational state of slaves (true = operational, false = not operational)


void setup() {
  Serial.begin(9600); // Debugging
  Serial1.begin(9600); // RS485 connection
  pinMode(RS485ControlPin, OUTPUT);
  digitalWrite(RS485ControlPin, LOW); // Set to receive mode
  ETin.begin(details(dataFromSlave), &Serial1);
  ETout.begin(details(dataToSlave), &Serial1);

  // Initialize all slaves as operational
  for (int i = 0; i < NUM_SLAVES; i++) {
    slaveStates[i] = true;
  }
}

void sendCommand(int slaveID, const char* command) {
  dataToSlave.slaveID = slaveID;
  strncpy(dataToSlave.command, command, sizeof(dataToSlave.command));
  digitalWrite(RS485ControlPin, HIGH); // Set to transmit mode
  delay(1); // Ensure proper control line switching
  ETout.sendData();
  digitalWrite(RS485ControlPin, LOW); // Back to receive mode
  Serial.print("Sent command to Slave ");
  Serial.println(slaveID);
}

bool receiveFeedback(int expectedSlaveID) {
  unsigned long startTime = millis();
  while (millis() - startTime < TIMEOUT) {
    if (ETin.receiveData()) {
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

void updateSlaveState(int slaveID, bool state) {
  slaveStates[slaveID - 1] = state; // Update the state array (convert ID to array index)
  Serial.print("Slave ");
  Serial.print(slaveID);
  if (state) {
    Serial.println(" is operational.");
  } else {
    Serial.println(" is NOT operational.");
  }
}

void loop() {

  int retryCount = 0;
  bool responseReceived = false;

  // Try to communicate with the current slave
  while (retryCount < MAX_RETRIES && !responseReceived) {
    sendCommand(currentSlave, "SENSOR");
    responseReceived = receiveFeedback(currentSlave);

    if (!responseReceived) {
      retryCount++;
      Serial.print("Retry ");
      Serial.print(retryCount);
      Serial.print(" for Slave ");
      Serial.println(currentSlave);
    }
  }

  // Update the slave state based on the outcome
  if (responseReceived) {
    updateSlaveState(currentSlave, true); // Slave is operational
    Serial.println("Data successfully received.");
  } else {
    updateSlaveState(currentSlave, false); // Slave is not operational
    Serial.println("Failed to receive data from Slave.");
  }


  // Move to the next slave in the sequence
  currentSlave++;
  if (currentSlave > NUM_SLAVES) {
    currentSlave = 1; // Loop back to the first slave
  }

  delay(1000); // Wait before polling the next slave
}
