#ifndef CHARGECONTROLLER
#define CHARGECONTROLLER

#pragma once

#include <Arduino.h>
#include "SerialDebug.h"
#include "SensorMonitorInterface.h"
#include "AnalogSensorMonitor.h"
#include "charge_station_config.h"

/*Recibo de ThingsBoard
1. tiempo comprado por el usuario (tiempo en segundos)
2. Toma de carga seleccionado (id de la toma)

*/

// TODO 2. Actualizar y recibir las informaciones

//** Si el limite de corriente maxima es superado se desconecta la carga y se notifica a la plataforma
enum ChargePortStates
{
    IDDLE,           // Disponible
    RESERVED,        // Reservado
    PLUGGED,         // Toma conectado
    CHARGING,        // Cargando
    CHARGE_COMPLETE, // Carga Completada
    ABORTING         // Cancelando
};

class ChargeController
{
public:
    ChargeController();

    void init(SensorMonitorInterface *sensor, int relayPin, int pluggedPin, int id); // configuraciones iniciales

    void setMaxCurrentLimit(float current);                   // Establece la corriente de carga maxima permitida (Configuraciones Iniciales)
    void setPurchasedChargingTime(int purchasedChargingTime); // Tiempo de carga comprado
    void setChargeState(ChargePortStates charge_state);       // Establece el estado del proceso de carga 1- activo

    void startCharge(); // Inicia el proceso de carga
    void stopCharge();  // Detiene el proceso de carga

    void reserveSocket(bool reserved); // conocimiento de que se realizo el pago, ya se puede activar la toma

    float getChargeCurrent();                          // Devuelve la corriente de carga actual
    ChargePortStates getChargeState();                 // Devuelve el estado actual del proceso de carga
    int getRemainingTime() { return _remainingTime; }; // retornar el tiempo restante de carga
    int getID() { return _id; };

    void resetCharge(); // Reinicia el proceso de carga

    // void setPowerCapacity(float powerCapacity); // Cantidad de kwats a generar
    float getAcumulateConsumption(); // devuelve la cantidad de kwats consumidos
    void updateData();               // actualiza los datos de la toma

private:
    SensorMonitorInterface *_sensor; // sensor

    int _id;                    // id de la toma
    int _remainingTime;         // tiempo restante de carga
    int _purchasedChargingTime; // tiempo en segundos que tiene para cargar
    bool reserve;               // 1 - esta activa la toma
    bool plug_in;               // 1 - esta activa la
    bool start_charge;          // 1 - esta activa la
    ChargePortStates _state;    //
    bool is_active;             // 1 - esta lista para cargar la toma
    int _relayPin;              // pin del relay
    int _pluggedPin;            // pin del de moto conectada PLUGGED
    int _maxCurrentLimit;       // limite de corriente maxima
    float _powerCapacity;       // cantidad de watt a consumir
    float _powerConsumption;    // cantidad de watts consumidos en el instante de medicion
    float _currentConsumption;  // lectura del sensor
    bool is_connected;          // 1 - esta conectado a la toma
    float _energy;              // energia consumida (acomulada)

    TimerHandle_t _measurement_timerH;
    uint32_t _measurement_time;

    BaseType_t _measurement_timeout;
    static void chargeControllerTask(void *args);           // task function to control the charger
    static void _powerMeasurementTimerCallback(void *args); // timer callback
};
#endif