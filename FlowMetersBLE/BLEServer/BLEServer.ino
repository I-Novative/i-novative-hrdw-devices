#include <FlowMeter.h>

#include "BLEDevice.h"
#include "BLE2902.h"

using namespace std;

#define serviceUUID "8a28fe32-8652-46e4-895e-fae4096a03a0"
#define litersCharacteristicUUID "9287c499-9e4a-4f57-97f4-5871319a13ca"
#define stateCharacteristicUUID "3916eb21-7305-4d7d-935f-e1d60f0aadc5"
#define DEVICE_NAME "ESP32_FL1"

#define STATE_OFF 0
#define STATE_ON 1

uint16_t LITERS_VALUE = 0;
uint16_t WEIGHT_VALUE = 0;
uint8_t STATE_VALUE = 0;
bool clientConnected = false;

static const char* TAG = "BLEClient";

BLECharacteristic *litersCharacteristic;

//------------------------------------------------------------------------------------
#define SENSOR 2
#define calibrationFactor 4.5
#define intervalTimeMs 1000

// Define an FlowMeter object
FlowMeter* flowMeter;

void IRAM_ATTR pulseCounter(){
  flowMeter->increasePulseCount();
}
//------------------------------------------------------------------------------------

class stateCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* readCharacteristic){
    //ESP_LOGD(TAG, "Client read characteristic value");
    Serial.println("\nClient read characteristic value");
  }
  void onWrite(BLECharacteristic* writtenCharacteristic){
    //ESP_LOGD(TAG, "Clinet wrote a %d into state characteristic", writtenCharacteristic->getValue());
    Serial.print("\nClinet wrote a ");
    Serial.print(*writtenCharacteristic->getData());
    Serial.print(" into state characteristic");
  }
};

class serverCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* server){
    //ESP_LOGD(TAG, "Client connected");
    Serial.println("\nClient connected");
    clientConnected = true;
    LITERS_VALUE = 0;
  }
  void onDisconnect(BLEServer* server){
    //ESP_LOGD(TAG, "Client disconnected");
    Serial.println("\nClient disconnected");
    clientConnected = false;
    LITERS_VALUE = 0;
  }
};


void setup() {

  Serial.begin(115200);
  Serial.println("Initializing BLE SERVER");
  
  BLEDevice::init(DEVICE_NAME);
  
  BLEServer *FLServer = BLEDevice::createServer();

  //ESP_LOGD(TAG, "Server address: %s", BLEDevice::getAddress().toString().c_str());
  Serial.print("Server address: ");
  Serial.print(BLEDevice::getAddress().toString().c_str());
  
  FLServer->setCallbacks(new serverCallbacks());
  
  BLEService *FLService = FLServer->createService(serviceUUID);
  
  // Create 3 Characteristics for the FLService
  litersCharacteristic = FLService->createCharacteristic(litersCharacteristicUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  litersCharacteristic->setValue(LITERS_VALUE);
  // Descriptor with UUID 0x2902 is the descriptor for notifications
  // The client can set to 1 or 0 if it wants to receive or not notifications
  litersCharacteristic->addDescriptor(new BLE2902());
  
  BLECharacteristic *stateCharacteristic = FLService->createCharacteristic(stateCharacteristicUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  stateCharacteristic->setValue(&STATE_VALUE, 1);
  
  // Set callbacks for client that WRITES the state value
  // Only this characteristic needs this because has the one value that can be written
  stateCharacteristic->setCallbacks(new stateCharacteristicCallbacks());
  
  FLService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  BLEDevice::startAdvertising();
  
  //ESP_LOGD(TAG, "Server advertising it's service...");
  Serial.println("\nServer advertising it's service...");

  //------------------------------------------------------------------------------
  // Initial setup to use the flow meter object
  flowMeter = new FlowMeter(calibrationFactor, intervalTimeMs);
  pinMode(SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
  //-----------------------------------------------------------------------------
}

void loop() {

  //delay(2000);
  
  /*if(clientConnected){
    //ESP_LOGD(TAG, "Notifying LITERS_VALUE to connected BLEClients");
    Serial.println("\nNotifying LITERS_VALUE to connected BLEClients. Value = ");
    Serial.print(LITERS_VALUE);
    litersCharacteristic->setValue(LITERS_VALUE);
    litersCharacteristic->notify();
    LITERS_VALUE++;
  }*/

  flowMeter->listenToWaterFlow();
  uint8_t totalMillilters = flowMeter->getCurrentTotalMilliLiters();
  if(clientConnected){
    if(flowMeter->isflowDetected() && int(flowMeter->getCurrentFlowRate()) == 0){
      flowMeter->switchFlowDetectedFlag();
      Serial.println("\nNotifying LITERS_VALUE to connected BLEClients. Value = ");
      Serial.println(totalMillilters);
      litersCharacteristic->setValue(&totalMillilters, sizeof(totalMillilters));
      litersCharacteristic->notify();
      flowMeter->setCurrentTotalMilliLiters(0);
    }
  }
    
}
