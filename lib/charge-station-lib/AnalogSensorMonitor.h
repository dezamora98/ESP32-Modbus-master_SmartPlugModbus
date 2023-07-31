#ifndef ANALOGSENSORMONITOR
#define ANALOGSENSORMONITOR

#include <ACS712.h> // analog sensor
#include "SensorMonitorInterface.h"

class AnalogSensorMonitor : public SensorMonitorInterface
{
public:
    AnalogSensorMonitor(int analogSensorPin);
    bool begin() override;
    float read() override; // Funcion virtual para la lectura del sensor
private:
    int _pin;
    ACS712 _acs712;
};

#endif