#ifndef SPISENSORMONITOR
#define SPISENSORMONITOR

#include <Arduino.h>
#include "SensorMonitorInterface.h"

class SPISensorMonitor : public SensorMonitorInterface
{
public:
    bool begin() override ;
    float read() override ;
};

#endif