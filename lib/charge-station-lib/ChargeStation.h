// #ifndef CHARGESTATION_H
// #define CHARGESTATION_H

// #pragma once

// #include <Arduino.h>
// #include <SerialDebug.h>
// #include <RemoteInterface.h>
// #include <ConnectionController.h>
// #include <ChargePortController.h>
// #include <SerialDebug.h>
// #include <charge_station_config.h>

// // TODO: tener un objeto de cada RemoteInterface (A9G o TB normal)

// // todo
// /*[ ] Eventos en la clase portController
//   [ ] Método setEventCallback(void (*callback)(Event))
//   [ ] Como se cual tarea accedio al evento
// */
// class ChargeStation
// {
// public:
//     // Properties

//     ChargeStation();
//     ~ChargeStation();

//     // RemoteInterface _remote_interface;
//     ChargePortController *_ChargePortControllers;
//     ConnectionController *conn_controller = ConnectionController::Instance();

//     void init(int port_count, String device_name, int device_location[2]);
//     void addChargePort(ChargePortController charge_port);
//     void removeChargePort(int id_port);
//     float getAcumulateConsumption(int id_port);
//     ChargePortStates getState(int id_port);

//     void setState(int id_port, ChargePortStates state);
//     void setChargeTime(float charge_time);

//     int *getAvailablePorts();
//     void abortPortCharge(int id_port);
//     void reservedPortCharge(int id_port);

//     String getDeviceName() { return _device_name; };
//     int getPortID(int portNumber) { return _ChargePortControllers[portNumber].getID(); };

// private:
//     int count = 0;
//     int _port_id;
//     float consumption;
//     int _port_count;
//     String _device_name;
//     int _device_location[2]; // latitud - longitud

//     // static void ChargeStationTask(void *args);       // task function to control the charger
//     static void handleEvent(Event event, int id_port);
// };

// #endif