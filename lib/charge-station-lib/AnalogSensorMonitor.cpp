#include "AnalogSensorMonitor.h"

AnalogSensorMonitor::AnalogSensorMonitor(int _pin) : _acs712(_pin, 3.3, 4095, 185)
{
    //  ACS712 5A  uses 185 mV per A
    //  ACS712 20A uses 100 mV per A
    //  ACS712 30A uses  66 mV per A
    // (analog pin, max voltage, DAC resolution, sencibility )
    _pin = _pin;
}

bool AnalogSensorMonitor::begin()
{
    Serial.println("Analog sensor begin");
    uint16_t start, stop, midPoint = 0;
    _acs712.autoMidPoint();
    Serial.println("Finding midPoint"); // the output voltage of the sensor when there is no current flowing through it.
    //  might be different 1 cycle or 100.
    start = micros();
    midPoint = _acs712.autoMidPointDC(1);
    stop = micros();
   Serial.println("ACS.autoMidPointDC()");
   Serial.println("us:\t");
   Serial.println(stop - start);
    printD("MP 1:\t");
   Serial.println(midPoint);

    midPoint = _acs712.autoMidPointDC(100);
    printD("MP 100:\t");
   Serial.println(midPoint);

    Serial.print(". Noise mV: ");
   Serial.println(_acs712.getNoisemV());

    _acs712.suppressNoise(true);
   Serial.println("Suppress Noise = TRUE");
    return 1;
}

float AnalogSensorMonitor::read()
{
    Serial.println("Read Analog Sensor");
    return _acs712.mA_AC();
}