#include "ServerController.h"

#include <auxiliars.h>
#include <ArduinoJson.h>

// static members initialization
bool ServerController::_server_connected = false;
// ThingsBoard *ServerController::_tb_handler = nullptr;

void _processSharedAttributeUpdate(const Shared_Attribute_Data &data);

Shared_Attribute_Callback ServerController::_processSharedattributesUpdate_Callback(_processSharedAttributeUpdate);

ServerController::ServerController()
{
    // _rpc_subscribed = false;
    _attributes_subscribed = false;
}

ServerController::~ServerController()
{
}

bool ServerController::init(ThingsBoard *tb_h)
{
    bool success = false;

    _tb_handler = tb_h;

    if (_tb_handler == nullptr)
    {
        return false;
    }

    // subscribe Shared Attributes
    success = _suscribeSharedAttributes();

    // subscribe RPC

    // subscribe Provisioning

    // subscribe Claiming

    return success;
}

void ServerController::sendDbgCounter()
{
    static int dbg_counter = 0;

    if (_tb_handler != nullptr)
    {
        _tb_handler->sendTelemetryInt("DBG_COUNTER", dbg_counter++);
    }
    else
    {
        PRINT_LOCK
        printlnA("Trying to send Telemetry DbgCounter without TB handler properly assigned!");
        PRINT_UNLOCK
    }
}

bool ServerController::sendTelemetryToPlatform(const ChargeTelemetry &p_telem)
{
    if (_tb_handler != nullptr)
    {
        // TODO: Evaluar si enviar como JSON
        bool success = true;
        _tb_handler->sendTelemetryFloat("ain_1", p_telem.state); // Send Humedad

        return success;
    }
    return false;
}

bool ServerController::_suscribeSharedAttributes()
{
    PRINT_LOCK
    printlnA("Subscribing for shared attribute updates...");
    PRINT_UNLOCK

    if (_tb_handler->Shared_Attributes_Subscribe(_processSharedattributesUpdate_Callback))
    {
        _attributes_subscribed = true;
        PRINT_LOCK
        printlnA("Attributes Subscribe done");
        PRINT_UNLOCK
        // xEventGroupSetBits(connectionEventsHandler, (EventBits_t)ConnectionEvents::EVENT_SERVER_SHARED_ATTRIBUTES_SUSCRIBED);
        return true;
    }

    _attributes_subscribed = false;
    PRINT_LOCK
    printlnA("Failed to subscribe for Share Attributes updates");
    PRINT_UNLOCK
    return false;
}

void ServerController::_processSharedAttributeUpdate(const Shared_Attribute_Data &data)
{
    // for (JsonObject::iterator it = data.begin(); it != data.end(); ++it)
    // {
    //     printlnA(it->key().c_str());
    //     printlnA(it->value().as<const char *>()); // We have to parse data by it's type in the current example we will show here only char data
    // }

    // int jsonSize = measureJson(data) + 1;
    // char buffer[jsonSize];
    // serializeJson(data, buffer, jsonSize);
    // printlnA(buffer);

    PRINT_LOCK
    printlnA("Atributo Recibido");
    PRINT_UNLOCK

    if (data.containsKey("heartbeat"))
    {
        _server_connected = true;
        PRINT_LOCK
        printlnA("ThingsBoard heartbeat");
        PRINT_UNLOCK
    }
}
