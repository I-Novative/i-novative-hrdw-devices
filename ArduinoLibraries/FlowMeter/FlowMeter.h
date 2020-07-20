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
        unsigned long totalMilliLitres;
        bool flowDetectedFlag;

    public:
        FlowMeter(float, int);
        ~FlowMeter();
        float getCurrentFlowRate(){return flowRate;}
        unsigned int getCurrentFlowMilliLiters(){return flowMilliLitres;}
        unsigned int getCurrentTotalMilliLiters(){return totalMilliLitres;}
        float getCalibFactor(){return calibrationFactor;}
        int getCurrentInterval(){return currentInterval;}
        bool isflowDetected() {return flowDetectedFlag;}
        void increasePulseCount(){pulseCount++;}
        void listenToWaterFlow();
        void printLastFlowRead();

};

#endif