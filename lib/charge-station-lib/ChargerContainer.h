#ifndef CHARGERCONTAINER_H
#define CHARGERCONTAINER_H

#pragma once

#include <Arduino.h>
#include <ChargeController.h>

class ChargerContainer
{
public:
  // Properties

  ChargerContainer();
  ~ChargerContainer();

  void addChargePort(ChargeController charge_port);
  void removeChargePort(int id_port);
  float getAcumulateConsumption(int id_port);
  ChargePortStates getState(int id_port);

  void setState(int id_port, ChargePortStates state);
  void setChargeTime(float charge_time);

private:
  int _port_id;
  float consumption;
  ChargeController _chargeControllers[CHARGING_SOCKETS];
  int _socketsNumber;
};

#endif