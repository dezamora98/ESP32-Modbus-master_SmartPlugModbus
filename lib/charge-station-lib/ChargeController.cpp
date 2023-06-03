#include "ChargeController.h"

ChargeController::ChargeController(SensorMonitorInterface *sensor, int relayPin)
{
    _sensor = sensor;
    _relayPin = relayPin;
    is_active = false;
    // tiempo_muestreo = 1000;

    initPines();
}

void ChargeController::initPines()
{
    pinMode(_relayPin, OUTPUT);
    digitalWrite(_relayPin, LOW); // desactivar relay
}

void ChargeController::startCharge()
{
    digitalWrite(_relayPin, HIGH); // desactivar relay
    is_active = true;
}

void ChargeController::stopCharge()
{
    digitalWrite(_relayPin, LOW); // desactivar relay
    is_active = false;
}

bool ChargeController::getChargeState()
{
    return is_active;
}
