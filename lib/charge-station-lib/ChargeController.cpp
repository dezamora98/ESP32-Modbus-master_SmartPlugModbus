#include "ChargeController.h"

ChargeController::ChargeController()
{
    _measurement_time = MEASUREMENT_TIME;
}

void ChargeController::init(SensorMonitorInterface *sensor, int relayPin, int id)
{
    _sensor = sensor;
    _relayPin = relayPin;
    is_active = false;
    _id = id;

    pinMode(_relayPin, OUTPUT);
    digitalWrite(_relayPin, LOW); // desactivar relay

    BaseType_t rc = xTaskCreatePinnedToCore(
        chargeControllerTask,
        "chargeControllerTask",
        5000,
        this,
        1,
        NULL,
        ARDUINO_RUNNING_CORE);
    assert(rc == pdPASS);

    _measurement_timerH = xTimerCreate(
        "PowerMeasurementTimer",
        pdMS_TO_TICKS(_measurement_time),
        pdTRUE,
        this,
        _powerMeasurementTimerCallback);
}

void ChargeController::startCharge()
{
    digitalWrite(_relayPin, HIGH); // activar relay
    is_active = true;
    BaseType_t rc = xTimerStart(_measurement_timerH, portMAX_DELAY);
    assert(rc == pdPASS);
}

void ChargeController::stopCharge()
{
    digitalWrite(_relayPin, LOW); // desactivar relay
    is_active = false;
    BaseType_t rc = xTimerStop(_measurement_timerH, portMAX_DELAY);
    assert(rc == pdPASS);
}

bool ChargeController::getChargeState()
{
    return is_active;
}

float ChargeController::getChargeCurrent()
{
    return _sensor->read();
}

void ChargeController::updateData()
{
    printlnD("Update information to ThingsBoard");

    // TODO actualizar la informacion de la toma de carga (en caso de que se desconecte la )
}

float ChargeController::getPowerConsumption() // devuelve la cantidad de kwats que quedan
{
    printlnD("the watts consumption is: " + String(_powerConsumption));
    return _powerConsumption;
}

void ChargeController::setPowerCapacity(float powerCapacity) // Cantidad de kwats a generar
{
    _powerCapacity = powerCapacity;
}

/* ----------------------------- Timer callback ----------------------------- */

void ChargeController::_powerMeasurementTimerCallback(TimerHandle_t timer_h)
{
    ChargeController *obj = static_cast<ChargeController *>(pvTimerGetTimerID(timer_h));
    printD("dentro de timer");

    obj->_measurement_timeout = pdTRUE;
}

/* ----------------- task function to control the charger ---------------- */
void ChargeController::chargeControllerTask(void *args)
{
    ChargeController *obj = static_cast<ChargeController *>(args);
    float wattsConsum = 0;

    for (;;)
    {
        if (obj->_measurement_timeout)
        {
            obj->_measurement_timeout = pdFALSE;
            obj->_powerConsumption = obj->_sensor->read() * VOLTAGE; // Potencia consumida en el instante = corriente * voltaje 110V
            obj->_energy += obj->_powerConsumption;                  // enegia acumulada

            if (obj->is_active && obj->_energy < obj->getPowerConsumption() && obj->is_connected)
            { // si esta activo el puerto, si la energia consumida es menor que los watts comprados y esta conectado al toma
                obj->startCharge();
                printlnD("Start charge");
            }
            else
            {
                obj->stopCharge();
                printlnD("Stop charge");
                // call updateData() para actualzar los valores en la plataforma
            }
        }
    }
}
