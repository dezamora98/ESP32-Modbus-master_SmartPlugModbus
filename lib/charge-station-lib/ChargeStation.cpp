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
    Serial.println("init");
    _port_count = port_count;
    _device_location[0] = device_location[0];
    _device_location[1] = device_location[1];
    _device_name = device_name;
    _ChargePortControllers = new ChargePortController[_port_count];
    //* Inicializar la conexión a ThingsBoard mediante WiFi

    for (int i = 0; i <= _port_count - 1; i++)
    {
        _ChargePortControllers[i].setEventCallback(handleEvent);
    }
    /*TODO: seleccionar el tipo de conexion a mantener si es mediante wifi o mediante A9*/

    conn_controller->platformConnectionInit(ConnectionType::WIFI);
    ConnectionState con_state = conn_controller->getConnectionState();
    delay(1000);
    // _remote_interface.init(conn_controller->getThingsBoardHandler());

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
    for (int i = 0; i <= _port_count - 1; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            return _ChargePortControllers[i].getAcumulateConsumption();
        }
    }
    Serial.println("El id del puerto proporcionado no existe");
    return -1;
}

ChargePortStates ChargeStation::getState(int id_port)
{
    for (int i = 0; i <= _port_count - 1; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            return _ChargePortControllers[i].getChargeState();
        }
    }
    Serial.println("El id del puerto proporcionado no existe");
}

void ChargeStation::setState(int id_port, ChargePortStates state) // TODO not working the ID
{
    Serial.println("Entro a la funcion ");

    for (int i = 0; i <= _port_count - 1; i++)
    {
        Serial.print("ID>> ");
        Serial.println(_ChargePortControllers[i].getID());

        if (_ChargePortControllers[i].getID() == id_port)
        {
            _ChargePortControllers[i].setChargeState(state);
            Serial.print("Al puerto se le asigna el estado: ");
            Serial.println(state);
        }
    }
}
void ChargeStation::setChargeTime(float charge_time)
{
}

void ChargeStation::reservedPortCharge(int id_port)
{
    for (int i = 0; i <= _port_count - 1; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            _ChargePortControllers[i].setChargeState(RESERVED);
            Serial.println("Al puerto se le asigna el estado: RESERVED ");
        }
    }
}

void ChargeStation::abortPortCharge(int id_port)
{
    for (int i = 0; i <= _port_count - 1; i++)
    {
        if (_ChargePortControllers[i].getID() == id_port)
        {
            _ChargePortControllers[i].setChargeState(ABORTING);
            Serial.println("Al puerto se le asigna el estado: ABORTING ");
        }
    }
}

/*Revisar*/
int *ChargeStation::getAvailablePorts()
{
    int *_available_ports = new int[_port_count];
    int i = 0;
    int j = 0;
    for (int i = 0; i <= _port_count - 1; i++)
    {
        if (_ChargePortControllers[i].getChargeState() == IDDLE)
        {
            _available_ports[j] = _ChargePortControllers[i].getID();
            j++;
        }
    }

    return _available_ports;
}

// Función de callback para manejar el cambio del evento
void ChargeStation::handleEvent(Event event, int id_port)
{
    ChargeStation *obj = new ChargeStation;

    switch (event)
    {
    case CHARGE_STARTED:
        printD("EL ESTADO DEL PUERTO ES:");
        Serial.println(obj->getState(id_port));
        Serial.println("Charging event changed");
        break;
    case CHARGE_FINISHED:
        Serial.println("Plugged event changed");
        break;
    case CHARGE_ABORTED:
        Serial.println("Aborting event changed");
        break;
    case NOTIFY_ALARM:
        Serial.println("Notify event alarm");
        break;
    case PUBLISH_TELEMETRY:
        Serial.println("Publish telemetry event");
    default:
        break;
    }
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