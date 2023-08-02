#include "ChargerContainer.h"

ChargerContainer::ChargerContainer(/* args */)
{
}

ChargerContainer::~ChargerContainer()
{
}

void ChargerContainer::addChargePort(ChargeController charge_port)
{
    if (_socketsNumber < CHARGING_SOCKETS)
    {
        _chargeControllers[_socketsNumber] = charge_port;
        _socketsNumber++;
    }
}
void ChargerContainer::removeChargePort(int id_port)
{
    // delete charge_port
}

float ChargerContainer::getAcumulateConsumption(int id_port)
{
    return _chargeControllers[id_port - 1].getAcumulateConsumption();
}

ChargePortStates ChargerContainer::getState(int id_port)
{
    return _chargeControllers[id_port - 1].getChargeState();
}
void ChargerContainer::setState(int id_port, ChargePortStates state)
{
    _chargeControllers[id_port - 1].setChargeState(state);
}
void ChargerContainer::setChargeTime(float charge_time)
{
}