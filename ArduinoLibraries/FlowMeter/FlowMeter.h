#ifndef FLOW_METER
#define FLOW_METER

#include "Arduino.h"

class FlowMeter {

    private:
        long currentMillis;
        long previousMillis;
        int currentInterval;
        float calibrationFactor;
        volatile byte pulseCount;
        byte pulse1Sec;
        float flowRate;
        unsigned int flowMilliLitres;
        uint8_t totalMilliLitres;
        bool flowDetectedFlag;

    public:
        FlowMeter(float, int);
        ~FlowMeter();
        float getCurrentFlowRate(){return flowRate;}
        unsigned int getCurrentFlowMilliLiters(){return flowMilliLitres;}
        uint8_t getCurrentTotalMilliLiters(){return totalMilliLitres;}
        void setCurrentTotalMilliLiters(float value){totalMilliLitres = value;}
        float getCalibFactor(){return calibrationFactor;}
        int getCurrentInterval(){return currentInterval;}
        bool isflowDetected() {return flowDetectedFlag;}
        void switchFlowDetectedFlag(){flowDetectedFlag = !flowDetectedFlag;}
        void increasePulseCount(){pulseCount++;}
        void listenToWaterFlow();
        void printLastFlowRead();

};

#endif