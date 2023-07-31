#include "ChargeController.h"

ChargeController::ChargeController()
{
    // _measurement_time = MEASUREMENT_TIME;
}

/* -------------------------------------------------------------------------- */
void ChargeController::setChargeState(bool state)
{
    is_active = state;
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
void ChargeController::init(SensorMonitorInterface *sensor, int relayPin, int id)
{
    _sensor = sensor;
    _relayPin = relayPin;
    is_active = false;
    _id = id;
    _maxCurrentLimit = 1000; //* PONER UN VALOR POR DEFECTO DEL CONSUMO DE CORRIENTE MAXIMO

    pinMode(_relayPin, OUTPUT);
    digitalWrite(_relayPin, LOW); // desactivar relay
    
    _sensor->begin();
    // // task configuration
    // BaseType_t rc = xTaskCreatePinnedToCore(
    //     chargeControllerTask,
    //     "chargeControllerTask",
    //     5000,
    //     this,
    //     1,
    //     NULL,
    //     ARDUINO_RUNNING_CORE);
    // assert(rc == pdPASS);

    // // timer configuration
    // _measurement_timerH = xTimerCreate(
    //     "PowerMeasurementTimer",
    //     pdMS_TO_TICKS(_measurement_time),
    //     pdTRUE,
    //     this,
    //     _powerMeasurementTimerCallback);
}

void ChargeController::startCharge()
{
    printlnD("[CHARGE_CONTROLLER] Start charge");
    digitalWrite(_relayPin, HIGH); // activar relay
    charge_activated = true;       // se activa la carga de la toma
    is_active = true;
    // _remainingTime = _purchasedChargingTime;                         // tiempo restante es igual al tiempo comprado
    // BaseType_t rc = xTimerStart(_measurement_timerH, portMAX_DELAY); // iniciar temporizador
    // assert(rc == pdPASS);                                            // Asegurarse de que el temporizador se inició correctamente con una aserción
}

void ChargeController::stopCharge()
{
    printlnD("[CHARGE_CONTROLLER] Stop charge");
    digitalWrite(_relayPin, LOW); // desactivar relay
    is_active = false;
    // BaseType_t rc = xTimerStop(_measurement_timerH, portMAX_DELAY); // detener el temporizador
    // assert(rc == pdPASS);
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
    printlnD("[CHARGE_CONTROLLER] Update information to ThingsBoard");

    // TODO actualizar la informacion de la toma de carga (en caso de que se desconecte la )
}

float ChargeController::getPowerConsumption() // devuelve la cantidad de kwats comsumidos
{
    printlnD("[CHARGE_CONTROLLER] The watts consumption is: " + String(_powerConsumption));
    return _powerConsumption;
}

// void ChargeController::setPowerCapacity(float powerCapacity) // Cantidad de kwats a generar
// {
//     _powerCapacity = powerCapacity;
// }

// /* ----------------------------- Timer callback ----------------------------- */
// void ChargeController::_powerMeasurementTimerCallback(TimerHandle_t timer_h)
// {
//     ChargeController *obj = static_cast<ChargeController *>(pvTimerGetTimerID(timer_h));
//     printD("[CHARGE_CONTROLLER] Timer callback");

//     obj->_measurement_timeout = pdTRUE;
// }

// /* ----------------- task function to control the charger ---------------- */
// void ChargeController::chargeControllerTask(void *args)
// {
//     ChargeController *obj = static_cast<ChargeController *>(args);
//     float wattsConsum = 0;

//     for (;;)
//     {
//         if (obj->is_active && obj->charge_activated == false) // desde la plataforma activa la toma
//         {
//             obj->startCharge();
//         }
//         else if (obj->_measurement_timeout) // timer ends
//         {
//             obj->_measurement_timeout = pdFALSE;
//             obj->_powerConsumption = obj->_sensor->read() * VOLTAGE; // Potencia consumida en el instante = corriente * voltaje 110V
//             obj->_energy += obj->_powerConsumption;                  // enegia acumulada

//             obj->_remainingTime -= 1; // disminuye un segundo al tiempo de carga

//             if (!obj->is_active && obj->_remainingTime <= 0 && !obj->is_connected && obj->_sensor->read() < obj->_maxCurrentLimit)
//             { /*
//                 - si esta desactivado el puerto (desde la plataforma mandan a parar)
//                 - si se acaba el timepo comprado
//                 - si se desconecta la toma fisica
//                 - si existe un alto consumo
//             */
//                 obj->stopCharge();
//                 obj->updateData();
//             }
//             else
//             {
//                 obj->updateData(); // call updateData() para actualzar los valores en la plataforma
//             }
//         }
//     }
// }
