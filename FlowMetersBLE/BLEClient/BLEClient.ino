#include "BLEDevice.h"

using namespace std;

// The time this client will scan for other devices
#define SCANNING_TIME 20

#define STATE_OFF 0
#define STATE_ON 1

// The remote service we wish to connect to.
#define remoteServiceUUID "8a28fe32-8652-46e4-895e-fae4096a03a0"
// The characteristic of the remote service that is gonna be notified to this client.
#define litersCharacteristicUUID "9287c499-9e4a-4f57-97f4-5871319a13ca"
// The characteristic of the remote service that we can read and write the state (ON/OFF)
#define stateCharacteristicUUID "3916eb21-7305-4d7d-935f-e1d60f0aadc5"

static BLERemoteCharacteristic* litersRemoteCharacteristic;
static BLERemoteCharacteristic* stateRemoteCharacteristic;
static BLEAdvertisedDevice* myAdvDevice;
static BLEClient* MyClient;

static BLEScan* scanner;

static bool connectionStablished = false;
static bool remoteServiceDataExtracted = false;
static bool advertisingDeviceFound = false;

// Value that will be set as the stateRemoteCharacteristic value when writing to it
uint8_t stateValueToWrite = STATE_OFF;

static const char* TAG = "BLEClient";

static void litersNotifyCallback(BLERemoteCharacteristic *remoteCharacteristic, uint8_t *data, size_t length, bool isNotify){
  Serial.println("\nNOTIFICATION FROM SERVER ");
  Serial.print(remoteCharacteristic->getUUID().toString().c_str());
  Serial.println("\nValue received: ");
  Serial.println(*data);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("\nOnConnect reached");
  }

  void onDisconnect(BLEClient* pclient) {
    Serial.println("OnDisconnect reached");
    connectionStablished = false;
    remoteServiceDataExtracted = false;
    advertisingDeviceFound = false;
  }
};

void extractServerInfo() {

    // Obtain a reference to the service we are looking for
    BLERemoteService* RemoteService = MyClient->getService(remoteServiceUUID);
    if (RemoteService == nullptr) {
      Serial.println("\nFailed to find our service UUID: ");
      Serial.print(remoteServiceUUID);
      MyClient->disconnect();
      return;
    }

    Serial.println("\nService with UUID ");
    Serial.print(remoteServiceUUID);
    Serial.print(" found");

    // Obtain a reference to the liters characteristic in the service we just found
    litersRemoteCharacteristic = RemoteService->getCharacteristic(litersCharacteristicUUID);
    if (litersRemoteCharacteristic == nullptr) {
      Serial.println("\nFailed to find our characteristic UUID: ");
      Serial.print(litersCharacteristicUUID);
      MyClient->disconnect();
      return;
    }

    Serial.println("\nLiters characteristic with UUID");
    Serial.print(litersCharacteristicUUID);
    Serial.print(" found");

    // Obtain a reference to the state characteristic in the service we just found
    stateRemoteCharacteristic = RemoteService->getCharacteristic(stateCharacteristicUUID);
    if (stateRemoteCharacteristic == nullptr) {
      Serial.println("\nFailed to find our characteristic UUID: ");
      Serial.print(stateCharacteristicUUID);
      MyClient->disconnect();
      return;
    }

    Serial.println("\nState characteristic with UUID ");
    Serial.print(litersCharacteristicUUID);

    // Enable notifications from the server on the liters characteristic descriptor
    if(litersRemoteCharacteristic->canNotify())
      litersRemoteCharacteristic->registerForNotify(litersNotifyCallback);

    remoteServiceDataExtracted = true;
}

void connectDeviceToServer(){
    Serial.println("\nRemote server address: ");
    Serial.print(myAdvDevice->getAddress().toString().c_str());
    MyClient = BLEDevice::createClient();
    Serial.println("\nClient created...");
    MyClient->setClientCallbacks(new MyClientCallback());
    Serial.println("\nClient callbacks set...");
    MyClient->connect(myAdvDevice);
    Serial.println("\nConnected to server");
    connectionStablished = true;
}

void readServerCharacteristicState(){
  // Read the value of the state characteristic.(ON/OFF)
    if(stateRemoteCharacteristic->canRead()) {
      string value = stateRemoteCharacteristic->readValue();
      Serial.println("\nThe device state characteristic value is: ");
      Serial.print(value.c_str());
    }
}



// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  //Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    Serial.println("\nBLE Advertised Device found");

    // The remote device is advertising it's service data as well
    if (advertisedDevice.haveServiceUUID()){
      // Get service UUID to know if it is the one we are looking for
      BLEUUID serviceUUID = advertisedDevice.getServiceUUID();
      Serial.println("\nService UUID scanned ");
      Serial.print(serviceUUID.toString().c_str());

      // Check if is the UUID we are looking for
      if(advertisedDevice.isAdvertisingService(BLEUUID(remoteServiceUUID))){
        Serial.println("\nService UUID match");
        scanner->stop();
        Serial.println("\nStop scanning...");
        myAdvDevice = new BLEAdvertisedDevice(advertisedDevice); // Init the device found to extract it's data
        advertisingDeviceFound = true;
      }
      else {
        Serial.println("\nFailed to get remote service of advertising device");
      }
    }
  }
};


void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting Arduino BLE Client...");
  BLEDevice::init("ESP32_CLIENT");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 10 seconds.
  scanner = BLEDevice::getScan();
  scanner->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  scanner->setActiveScan(true);
  scanner->start(10);
}


// This is the Arduino main loop function.
// If the connection succeed update the stateRemoteCharacteristic every 2 seconds
void loop() {

  delay(2000); // Delay 2 seconds between loops.
   
  if (advertisingDeviceFound) { // Check if the BLE Server was found
    connectDeviceToServer();
    advertisingDeviceFound = false;
  }
  
  if(connectionStablished){  // Check if the connection was stablished successfully
    extractServerInfo(); 
    connectionStablished = false;
  }
    
  /*if(remoteServiceDataExtracted){ // Check if the data got extracted from the BLE Server successfully
    
    //ESP_LOGD(TAG, "We are now connected to the BLE Server");
    Serial.println("\nWe are now connected to the BLE Server");
    
    stateValueToWrite = STATE_ON;
    if(stateRemoteCharacteristic->canRead()){
      //ESP_LOGD(TAG, "Reading state value from service characteristic. Value = %d", stateRemoteCharacteristic->readUInt8());
      Serial.println("\nReading state value from service characteristic. Value = ");
      Serial.print(stateRemoteCharacteristic->readUInt8()); 
    }
    
    if(stateRemoteCharacteristic->canWrite()){ 
      //ESP_LOGD(TAG, "Wrting new value to the state characteristic"); 
      Serial.println("\nWrting new value to the state characteristic. Value = ");
      Serial.print(stateValueToWrite); 
      stateRemoteCharacteristic->writeValue(stateValueToWrite, 0); // 0 means I dont need a response from the server
      stateValueToWrite = STATE_OFF;
    } 
    
  }
  else{
    scanner->start(20); // If connection failed try to scan again
  }*/
  
} // End of loop
