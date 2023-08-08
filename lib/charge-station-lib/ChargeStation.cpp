#include "ChargeStation.h"

ChargeStation::ChargeStation(/* args */)
{
}

ChargeStation::~ChargeStation()
{
    delete[] _ChargePortControllers;
}

void ChargeStation::init(int port_count, String device_name, int device_location[2])
{
    _port_count = port_count;
    _device_location[0] = device_location[0];
    _device_location[1] = device_location[1];
    _device_name = device_name;
    _ChargePortControllers = new ChargePortController[_port_count];

    /*TODO: seleccionar el tipo de conexion a mantener si es mediante wifi o mediante A9*/

    conn_controller->platformConnectionInit(ConnectionType::WIFI);
    ConnectionState con_state = conn_controller->getConnectionState();
    delay(1000);
    _remote_interface.init(conn_controller->getThingsBoardHandler());

    // // task configuration
    // BaseType_t rc = xTaskCreatePinnedToCore(
    //     ChargeStationTask,
    //     "ChargeStationTask",
    //     2056,
    //     this,
    //     1,
    //     NULL,
    //     ARDUINO_RUNNING_CORE);
    // assert(rc == pdPASS);
}

void ChargeStation::addChargePort(ChargePortController charge_port)
{
    int count = 0;
    if (count < _port_count)
    {
        _ChargePortControllers[count] = charge_port;
        count++;
    }
}

void ChargeStation::removeChargePort(int id_port)
{
    // delete charge_port
}

float ChargeStation::getAcumulateConsumption(int id_port)
{
    for (int i = 0; i < _port_count; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            return _ChargePortControllers[i].getAcumulateConsumption();
        }
    }
    printlnE("El id del puerto proporcionado no existe");
    return -1;
}

ChargePortStates ChargeStation::getState(int id_port)
{
    for (int i = 0; i < _port_count; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            return _ChargePortControllers[i].getChargeState();
        }
    }
    printlnE("El id del puerto proporcionado no existe");
}

void ChargeStation::setState(int id_port, ChargePortStates state)
{
    for (int i = 0; i < _port_count; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            _ChargePortControllers[i].setChargeState(state);
            printI("Al puerto se le asigna el estado: ");
            printlnI(state);
        }
    }
}
void ChargeStation::setChargeTime(float charge_time)
{
}

void ChargeStation::reservedPortCharge(int id_port)
{
    for (int i = 0; i < _port_count; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            _ChargePortControllers[i].setChargeState(RESERVED);
            printlnI("Al puerto se le asigna el estado: RESERVED ");
        }
    }
}

void ChargeStation::abortPortCharge(int id_port)
{
    for (int i = 0; i < _port_count; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            _ChargePortControllers[i].setChargeState(ABORTING);
            printlnI("Al puerto se le asigna el estado: ABORTING ");
        }
    }
}

/*Revisar*/
int *ChargeStation::getAvailablePorts()
{
    int *_available_ports = new int[_port_count];
    int i = 0;
    int j = 0;
    for (i = 0; i < _port_count; i++)
    {
        if (_ChargePortControllers[i].getChargeState() == IDDLE)
        {
            _available_ports[j] = _ChargePortControllers[i].getID();
            j++;
        }
    }

    return _available_ports;
}

//* TASK

// /* ----------------- task function to control the charger ---------------- */
// void ChargeStation::ChargeStationTask(void *args)
// {
//     ChargeStation *obj = static_cast<ChargeStation *>(args);

//     for (;;)
//     {

//     }
// }