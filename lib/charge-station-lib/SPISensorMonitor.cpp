#include "SPISensorMonitor.h"

bool SPISensorMonitor::begin() 
{
    Serial.println("SPISensorMonitor begin");
}

float SPISensorMonitor::read() 
{
    Serial.println("SPISensorMonitor read");
    return 0.2;
}