#ifndef ECO_CLIENT
#define ECO_CLIENT

#include "Arduino.h"
#include "BLEDevice.h"
#include <string>

class EcoClientBLE {

    private:
        BLEUUID remoteServiceUUID;
        BLEUUID litersCharacteristicUUID;
        BLEUUID stateCharacteristicUUID;

    public:
        EcoClientBLE(BLEUUID remoteServiceUUID, BLEUUID litersCharacteristicUUID, BLEUUID stateCharacteristicUUID);
        EcoClientBLE();
        BLERemoteCharacteristic* litersRemoteCharacteristic;
        BLERemoteCharacteristic* stateRemoteCharacteristic;
        BLEAdvertisedDevice* myAdvDevice;
        BLEClient* MyClient;
        bool connectionStablished = false;
        bool remoteServiceDataExtracted = false;
        bool advertisingDeviceFound = false;
        void extractServerInfo(BLERemoteService* RemoteService);
        void connectDeviceToServer();
        void setEcoClientCallbacks(BLEClientCallbacks* pClientCallbacks);
        void readServerCharacteristicState();
        

};

#endif