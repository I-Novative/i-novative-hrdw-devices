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
#define SENSOR  2

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
uint8_t totalMilliLitres;
bool flag;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
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

void readWaterFlow(){

  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;

    if(int(flowRate) > 0) {
      flag = true;
    }
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
  }
    
}

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
  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  flag = false;

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

  readWaterFlow();
  
  if(clientConnected){
    if(flag == true && int(flowRate) == 0){
      flag = false;
      Serial.println("\nNotifying LITERS_VALUE to connected BLEClients. Value = ");
      Serial.println(totalMilliLitres);
      litersCharacteristic->setValue(&totalMilliLitres, sizeof(totalMilliLitres));
      litersCharacteristic->notify();
      totalMilliLitres = 0;
    }
  }
    
}
