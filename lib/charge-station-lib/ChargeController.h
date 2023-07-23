#ifndef CHARGECONTROLLER
#define CHARGECONTROLLER

#pragma once

#include <Arduino.h>
#include "SerialDebug.h"
#include "SensorMonitorInterface.h"

// TODO 1. Como detectar si esta conectada o no la moto
// TODO 2. Actualizar y recibir las informaciones

class ChargeController
{
public:
    SensorMonitorInterface *sensor;

    ChargeController(SensorMonitorInterface *sensor, int relayPin, int id);

    void startCharge(); // Inicia el proceso de carga
    void stopCharge();  // Detiene el proceso de carga
    // void setChargeCurrent(float current); // Establece la corriente de carga
    float getChargeCurrent(); // Devuelve la corriente de carga actual
    bool getChargeState();    // Devuelve el estado actual del proceso de carga
    // float getChargeTime();    // TODO retornar el tiempo de carga // Devuelve el tiempo transcurrido desde que inicio el proceso de carga

    void resetCharge(); // Reinicia el proceso de carga
    void initPines();   // Inicializa los pines

    void setPowerCapacity(float powerCapacity); // Cantidad de kwats a generar
    float getPowerConsumption();                // devuelve la cantidad de kwats consumidos
    void updateData();                          // actualiza los datos de la toma

private:
    SensorMonitorInterface *_sensor; // sensor

    int _id; // id de la toma
    // int _chargeState;  //
    // float _chargeTime; // tiempo que lleva cargando
    bool is_active;       // 1 - esta activa la toma
    int _relayPin;        // pin del relay
    float _powerCapacity; // cantidad de watt a consumir
    float _wattsConsum;   // cantidad de watts consumidos
    bool is_connected;    // 1 - esta conectado a la toma

    static void chargeControllerTask(void *args); // task function to control the charger
};
#endif