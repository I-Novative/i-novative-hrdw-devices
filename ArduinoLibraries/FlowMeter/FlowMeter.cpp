#include <iostream>
#include "Arduino.h"
#include "FlowMeter.h"

using namespace std;

FlowMeter::FlowMeter(float CALIBRATION_FACTOR, int INTERVAL){

    this->calibrationFactor = CALIBRATION_FACTOR;
    this->currentInterval = INTERVAL;
    this->pulseCount = 0;
    this->flowRate = 0.0;
    this->flowMilliLitres = 0;
    this->totalMilliLitres = 0;
    this->previousMillis = 0;
    this->currentMillis = 0;
    flowDetectedFlag = false;

}

void FlowMeter::printLastFlowRead(){
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

void FlowMeter::listenToWaterFlow(){

    currentMillis = millis();
    if (currentMillis - previousMillis > currentInterval) {
        
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
            flowDetectedFlag = true;
        }
        
        printLastFlowRead();
        
    }

}