#include "ChargeController.h"

ChargeController::ChargeController()
{
    _measurement_time = MEASUREMENT_TIME;
    _state = IDDLE;
}

/* -------------------------------------------------------------------------- */
void ChargeController::setChargeState(ChargePortStates state)
{
    _state = state;
}

void ChargeController::setMaxCurrentLimit(float current)
{
    _maxCurrentLimit = current;
}
void ChargeController::setPurchasedChargingTime(int purchasedChargingTime)
{
    _purchasedChargingTime = purchasedChargingTime;
}

/* -------------------------------------------------------------------------- */
void ChargeController::init(SensorMonitorInterface *sensor, int relayPin, int pluggedPin, int id)
{
    _sensor = sensor;
    _relayPin = relayPin;
    _pluggedPin = pluggedPin;
    is_active = false;
    _state = IDDLE;
    _id = id;
    _maxCurrentLimit = MAX_CURRENT;

    pinMode(_relayPin, OUTPUT);
    pinMode(_pluggedPin, INPUT); // TODO input_pullup???

    digitalWrite(_relayPin, LOW); // desactivar relay

    _sensor->begin();

    // task configuration
    BaseType_t rc = xTaskCreatePinnedToCore(
        chargeControllerTask,
        "chargeControllerTask",
        5000,
        this,
        1,
        NULL,
        ARDUINO_RUNNING_CORE);
    assert(rc == pdPASS);

    // timer configuration
    _measurement_timerH = xTimerCreate(
        "PowerMeasurementTimer",
        pdMS_TO_TICKS(_measurement_time),
        pdTRUE,
        this,
        _powerMeasurementTimerCallback);
}

/*Iniciar la carga, es decir dejar pasar la corriente*/
void ChargeController::startCharge()
{
    printlnD("[CHARGE_CONTROLLER] Start charge");
    digitalWrite(_relayPin, HIGH); // activar relay
    // charge_activated = true;       // se activa la carga de la toma
    is_active = true;
    _state = CHARGING;
    _remainingTime = _purchasedChargingTime;                         // tiempo restante es igual al tiempo comprado
    BaseType_t rc = xTimerStart(_measurement_timerH, portMAX_DELAY); // iniciar temporizador
    assert(rc == pdPASS);                                            // Asegurarse de que el temporizador se inició correctamente con una aserción
}

void ChargeController::stopCharge()
{
    printlnD("[CHARGE_CONTROLLER] Stop charge");
    digitalWrite(_relayPin, LOW); // desactivar relay
    is_active = false;
    BaseType_t rc = xTimerStop(_measurement_timerH, portMAX_DELAY); // detener el temporizador
    assert(rc == pdPASS);
}

void ChargeController::reserveSocket(bool reserve)
{
    if (reserve)
    {
        _state = RESERVED;
    }
    else
    {
        _state = IDDLE;
    }
}

ChargePortStates ChargeController::getChargeState()
{
    return _state;
}

float ChargeController::getChargeCurrent()
{
    return _sensor->read();
}

void ChargeController::updateData()
{
    printlnD("[CHARGE_CONTROLLER] Update information to ThingsBoard");

    // TODO actualizar la informacion de la toma de carga (en caso de que se desconecte la )
}

float ChargeController::getAcumulateConsumption() // devuelve la cantidad de kwats comsumidos
{
    printlnD("[CHARGE_CONTROLLER] The watts consumption is: " + String(_energy));
    return _energy;
}

// void ChargeController::setPowerCapacity(float powerCapacity) // Cantidad de kwats a generar
// {
//     _powerCapacity = powerCapacity;
// }

/* ----------------------------- Timer callback ----------------------------- */
void ChargeController::_powerMeasurementTimerCallback(TimerHandle_t timer_h)
{
    ChargeController *obj = static_cast<ChargeController *>(pvTimerGetTimerID(timer_h));
    printD("[CHARGE_CONTROLLER] Timer callback");

    obj->_measurement_timeout = pdTRUE;
}

/* ----------------- task function to control the charger ---------------- */
void ChargeController::chargeControllerTask(void *args)
{
    ChargeController *obj = static_cast<ChargeController *>(args);
    float wattsConsum = 0;

    for (;;)
    {
        if (digitalRead(obj->_pluggedPin)) // se conecto a la toma
        {
            obj->plug_in = true;
        }
        else
        {
            obj->plug_in = false;
        }

        //* MAQUINA DE ESTADO
        if (obj->_state == RESERVED)
        {
            //* MODO RESERVED VIENE DE THIGNSBOARD
            if (obj->plug_in)
                obj->_state = PLUGGED;
        }
        else if (obj->_state == PLUGGED)
        {
            obj->startCharge();
        }
        else if (obj->_state == CHARGING)
        {
            if (obj->_measurement_timeout)
            {
                obj->_measurement_timeout = pdFALSE;
                obj->_powerConsumption = obj->_sensor->read() * VOLTAGE; // Potencia consumida en el instante = corriente * voltaje 110V
                // TODO medir voltaje
                // TODO medir temperatura
                obj->_energy += obj->_powerConsumption; // enegia acumulada

                obj->_remainingTime -= 1; // disminuye un segundo al tiempo de carga
            }
            if (obj->_remainingTime < 0 || !obj->plug_in || obj->_sensor->read() < obj->_maxCurrentLimit)
            { /*
              /! //*Si mandan a desconcetar desde ThingsBoard
               - si se acaba el timepo comprado
               - si se desconecta la toma fisica
               - si existe un alto consumo
              */
                obj->_state = ABORTING;
            }
        }
        else if (obj->_state == CHARGE_COMPLETE)
        {
            obj->stopCharge();
        }
        else if (obj->_state == ABORTING)
        {
            obj->stopCharge();
            // TODO - Alarmas de xq se aborto
        }
    }
}
