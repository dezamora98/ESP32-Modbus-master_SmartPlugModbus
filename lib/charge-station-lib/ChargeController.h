#ifndef CHARGECONTROLLER
#define CHARGECONTROLLER

#pragma once

#include <Arduino.h>
#include "SensorMonitorInterface.h"

class ChargeController
{
public:
    SensorMonitorInterface *sensor;

    ChargeController(SensorMonitorInterface *sensor, int _relayPin);

    void getpowerMetering();              // Devuelve los kilowats consumidos
    void startCharge();                   // Inicia el proceso de carga
    void stopCharge();                    // Detiene el proceso de carga
    void setChargeCurrent(float current); // Establece la corriente de carga
    float getChargeCurrent();             // Devuelve la corriente de carga actual
    bool getChargeState();                // Devuelve el estado actual del proceso de carga
    float getChargeTime();                // Devuelve el tiempo transcurrido desde que inicio el proceso de carga
    void resetCharge();                   // Reinicia el proceso de carga
    void initPines();                     // Inicializa los pines

private:
    int _chargeState;
    float _chargeTime;
    bool is_active;
    SensorMonitorInterface *_sensor;
    int _relayPin;
};

#endif