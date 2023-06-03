#ifndef SENSORMONITORINTERFACE
#define SENSORMONITORINTERFACE

class SensorMonitorInterface
{
public:
    virtual bool begin();
    virtual float read(); // Funcion virtual para la lectura del sensor
};

#endif