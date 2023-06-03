#ifndef SPISENSORMONITOR
#define SPISENSORMONITOR

#include "SensorMonitorInterface.h"

class SPISensorMonitor : public SensorMonitorInterface
{
public:
    SPISensorMonitor(int analogSensorPin);
    bool begin();
    float read();
};

#endif