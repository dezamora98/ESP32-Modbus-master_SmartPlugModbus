#include "ChargeController.h"

ChargeController::ChargeController(SensorMonitorInterface *sensor, int relayPin, int id)
{
    _sensor = sensor;
    _relayPin = relayPin;
    is_active = false;
    _id = id;

    initPines(); // setup pins

    BaseType_t rc = xTaskCreatePinnedToCore(
        chargeControllerTask,
        "chargeControllerTask",
        5000,
        this,
        1,
        NULL,
        ARDUINO_RUNNING_CORE);
    assert(rc == pdPASS);
}

void ChargeController::initPines()
{
    pinMode(_relayPin, OUTPUT);
    digitalWrite(_relayPin, LOW); // desactivar relay
}

void ChargeController::startCharge()
{
    digitalWrite(_relayPin, HIGH); // activar relay
    is_active = true;
    // TODO iniciar timer para contar tiempo de carga
}

void ChargeController::stopCharge()
{
    digitalWrite(_relayPin, LOW); // desactivar relay
    is_active = false;
    // TODO parar el timer
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
    printlnD("the watts consumption is: " + String(_wattsConsum));
    return _wattsConsum;
}

void ChargeController::setPowerCapacity(float powerCapacity) // Cantidad de kwats a generar
{
    _powerCapacity = powerCapacity;
}

/* ----------------- task function to control the charger ---------------- */
void ChargeController::chargeControllerTask(void *args)
{
    ChargeController *obj = static_cast<ChargeController *>(args);
    float wattsConsum = 0;

    for (;;)
    {
        wattsConsum = obj->getChargeCurrent() * 110;                                          // kWatts = A * V --- P = I*V
        if (obj->is_active && wattsConsum < obj->getPowerConsumption() && !obj->is_connected) // si la cantidad de watts consumidos es menor que la definida
        {
            obj->startCharge();
            printlnD("Start Charge");
        }
        else
        {
            obj->stopCharge();
            printlnD("Stop Charge");
        }
    }
}
