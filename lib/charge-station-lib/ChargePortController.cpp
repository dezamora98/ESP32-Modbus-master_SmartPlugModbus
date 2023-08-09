#include "ChargePortController.h"

ChargePortController::ChargePortController()
{
    _measurement_time = MEASUREMENT_TIME;
    _state = IDDLE;
    // Crear un semaforo para sincronizaar el acceso a eventos
    eventSemaphore = xSemaphoreCreateMutex();
}

/* -------------------------------------------------------------------------- */
void ChargePortController::setChargeState(ChargePortStates state)
{
    _state = state;
}

void ChargePortController::setMaxCurrentLimit(float current)
{
    _maxCurrentLimit = current;
}
void ChargePortController::setPurchasedChargingTime(int purchasedChargingTime)
{
    _purchasedChargingTime = purchasedChargingTime;
}

/* -------------------------------------------------------------------------- */
void ChargePortController::init(SensorMonitorInterface *sensor, int relayPin, int pluggedPin, int id)
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
        ChargePortControllerTask,
        "ChargePortControllerTask",
        2056,
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
void ChargePortController::startCharge()
{
    Serial.println("[CHARGE_CONTROLLER] Start charge");
    digitalWrite(_relayPin, HIGH); // activar relay
    // charge_activated = true;       // se activa la carga de la toma
    is_active = true;
    _state = CHARGING;
    _remainingTime = _purchasedChargingTime;                         // tiempo restante es igual al tiempo comprado
    BaseType_t rc = xTimerStart(_measurement_timerH, portMAX_DELAY); // iniciar temporizador
    assert(rc == pdPASS);                                            // Asegurarse de que el temporizador se inició correctamente con una aserción
}

void ChargePortController::stopCharge()
{
    Serial.println("[CHARGE_CONTROLLER] Stop charge");
    digitalWrite(_relayPin, LOW); // desactivar relay
    is_active = false;
    BaseType_t rc = xTimerStop(_measurement_timerH, portMAX_DELAY); // detener el temporizador
    assert(rc == pdPASS);
}

void ChargePortController::reserveSocket(bool reserve)
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

ChargePortStates ChargePortController::getChargeState()
{
    return _state;
}

float ChargePortController::getChargeCurrent()
{
    return _sensor->read();
}

void ChargePortController::updateData()
{
    Serial.println("[CHARGE_CONTROLLER] Update information to ThingsBoard");

    // TODO actualizar la informacion de la toma de carga (en caso de que se desconecte la )
}

float ChargePortController::getAcumulateConsumption() // devuelve la cantidad de kwats comsumidos
{
    Serial.println("[CHARGE_CONTROLLER] The watts consumption is: " + String(_energy));
    return _energy;
}

void ChargePortController::generateEvent(Event event, int _id)
{
    // Tomar el semaforo para acceder a los eventos
    xSemaphoreTake(eventSemaphore, portMAX_DELAY);

    // Generar el evento
    events |= (1 << event);

    // Liberar el semáforo
    xSemaphoreGive(eventSemaphore);

    // Notificar el cambio del evento
    if (eventCallback)
    {
        eventCallback(event, _id);
    }
}

void ChargePortController::setEventCallback(void (*callback)(Event, int))
{
    eventCallback = callback;
}

// void ChargePortController::setPowerCapacity(float powerCapacity) // Cantidad de kwats a generar
// {
//     _powerCapacity = powerCapacity;
// }

/* ----------------------------- Timer callback ----------------------------- */
void ChargePortController::_powerMeasurementTimerCallback(TimerHandle_t timer_h)
{
    ChargePortController *obj = static_cast<ChargePortController *>(pvTimerGetTimerID(timer_h));
    printD("[CHARGE_CONTROLLER] Timer callback");

    obj->_measurement_timeout = pdTRUE;
}

/* ----------------- task function to control the charger ---------------- */
void ChargePortController::ChargePortControllerTask(void *args)
{
    Serial.println("CHARGE_PORT_CONTROLLER task");
    ChargePortController *obj = static_cast<ChargePortController *>(args);
    float wattsConsum = 0;

    for (;;)
    {
        // if (digitalRead(obj->_pluggedPin)) // se conecto a la toma
        // {
        //     obj->plug_in = true;
        // }
        // else
        // {
        //     obj->plug_in = false;
        // }

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
            obj->generateEvent(CHARGE_STARTED, obj->getID());
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
            obj->generateEvent(CHARGE_FINISHED, obj->getID());
        }
        else if (obj->_state == ABORTING)
        {
            obj->stopCharge();
            obj->generateEvent(CHARGE_ABORTED, obj->getID());
            // delay(100);
            // obj->generateEvent(NOTIFY_ALARM);
            // TODO - Alarmas de xq se aborto
        }
    }
}
