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
    printlnV("Analog sensor begin");
    uint16_t start, stop, midPoint = 0;
    _acs712.autoMidPoint();
    printlnV("Finding midPoint"); // the output voltage of the sensor when there is no current flowing through it.
    //  might be different 1 cycle or 100.
    start = micros();
    midPoint = _acs712.autoMidPointDC(1);
    stop = micros();
    printlnD("ACS.autoMidPointDC()");
    printlnD("us:\t");
    printlnD(stop - start);
    printD("MP 1:\t");
    printlnD(midPoint);

    midPoint = _acs712.autoMidPointDC(100);
    printD("MP 100:\t");
    printlnD(midPoint);

    Serial.print(". Noise mV: ");
    printlnD(_acs712.getNoisemV());

    _acs712.suppressNoise(true);
    printlnD("Suppress Noise = TRUE");
    return 1;
}

float AnalogSensorMonitor::read()
{
    printlnV("Read Analog Sensor");
    return _acs712.mA_AC();
}