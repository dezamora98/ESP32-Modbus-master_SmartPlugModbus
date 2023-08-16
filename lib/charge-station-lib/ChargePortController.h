#ifndef CHARGEPORTCONTROLLER
#define CHARGEPORTCONTROLLER

#pragma once

#include <Arduino.h>
#include <SerialDebug.h>
#include <SensorMonitorInterface.h>
#include <AnalogSensorMonitor.h>
#include <charge_station_config.h>

/*Recibo de ThingsBoard
1. tiempo comprado por el usuario (tiempo en segundos)
2. Toma de carga seleccionado (id de la toma)
*/

// TODO. Actualizar y recibir las informaciones

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

enum Event
{   CHARGE_STARTED,
    CHARGE_FINISHED,
    CHARGE_ABORTED,
    NOTIFY_ALARM,
    NOTIFY_ERROR,
    PUBLISH_TELEMETRY
};

class ChargePortController
{
public:
    ChargePortController();

    void init(SensorMonitorInterface *sensor, int relayPin, int pluggedPin, int id); // configuraciones iniciales

    void setMaxCurrentLimit(float current);                   // Establece la corriente de carga maxima permitida (Configuraciones Iniciales)
    void setPurchasedChargingTime(int purchasedChargingTime); // Tiempo de carga comprado
    void setChargeState(ChargePortStates charge_state);       // Establece el estado del puerto de carga

    void startCharge(); // Inicia el proceso de carga
    void stopCharge();  // Detiene el proceso de carga

    void reserveSocket(bool reserved); // conocimiento de que se realizo el pago, ya se puede activar la toma

    float getChargeCurrent();                          // Devuelve la corriente de carga actual
    ChargePortStates getChargeState();                 // Devuelve el estado actual del proceso de carga
    int getRemainingTime() { return _remainingTime; }; // retornar el tiempo restante de carga

    /* ----------------------------------- ID ----------------------------------- */
    void setID(int id) { _id = id; };
    int getID() { return _id; };

    void resetCharge(); // Reinicia el proceso de carga

    // void setPowerCapacity(float powerCapacity); // Cantidad de kwats a generar
    float getAcumulateConsumption(); // devuelve la cantidad de kwats consumidos
    void updateData();               // actualiza los datos de la toma

    /* ------------------------- CONTROL DE LOS EVENTOS ------------------------- */
    void generateEvent(Event event, int _id);
    void setEventCallback(void (*callback)(Event, int));

private:
    SensorMonitorInterface *_sensor; // sensor

    int _id;                    // id de la toma
    int _remainingTime;         // tiempo restante de carga
    int _purchasedChargingTime; // tiempo en segundos que tiene para cargar
    bool reserve;               // 1 - esta activa la toma
    bool plug_in;               // 1 - esta activa la
    // bool start_charge;          // 1 - esta activa la
    ChargePortStates _state; //
    bool is_active;          // 1 - esta lista para cargar la toma
    int _maxCurrentLimit;    // limite de corriente maxima
    float _powerConsumption; // cantidad de watts consumidos en el instante de medicion
    float _energy;           // energia consumida (acomulada)

    char *_alarm_msg;

    /*PINES*/
    int _relayPin;   // pin del relay
    int _pluggedPin; // pin de toma conectado - PLUGGED

    TimerHandle_t _measurement_timerH;
    uint32_t _measurement_time;

    /* --------------------------- TRABAJO CON EVENTOS -------------------------- */
    SemaphoreHandle_t eventSemaphore;
    uint8_t events = 0;
    // For Notify events
    void (*eventCallback)(Event, int) = nullptr; // TODO implemet this part in ChargeStation

    BaseType_t _measurement_timeout;
    static void ChargePortControllerTask(void *args);       // task function to control the charger
    static void _powerMeasurementTimerCallback(void *args); // timer callback
};
#endif