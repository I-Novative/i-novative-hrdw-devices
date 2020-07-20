#include <FlowMeter.h>
#define SENSOR 2
#define calibrationFactor 4.5
#define intervalTimeMs 1000

FlowMeter* flowMeter;

void IRAM_ATTR pulseCounter(){
  flowMeter->increasePulseCount();
}

void setup() {
  flowMeter = new FlowMeter(calibrationFactor, intervalTimeMs);
  pinMode(SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:

}
