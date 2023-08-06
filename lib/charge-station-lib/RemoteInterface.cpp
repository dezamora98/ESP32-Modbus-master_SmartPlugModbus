#include "RemoteInterface.h"
#include "auxiliars.h"

Shared_Attribute_Callback RemoteInterface::_processSharedattributesUpdate_Callback(_processSharedAttributeUpdate);

constexpr const char RPC_START_CHARGE_METHOD[] = "initCharge";
constexpr const char RPC_START_CHARGE[] = "response_start_charge_state";

RemoteInterface::RemoteInterface()
{
    _attributes_subscribed = false;
}

RemoteInterface::~RemoteInterface()
{
}

bool RemoteInterface::init(ThingsBoard *tb_h)
{
    bool success = false;
    _tb_handler = tb_h;

    if (_tb_handler == nullptr)
    {
        return false;
    }

    // subscribe Shared Attributes
    // success = _suscribeSharedAttributes();

    // subscribe RPC

    // subscribe Provisioning

    // subscribe Claiming

    _publishTelemetry_TimerH = xTimerCreate(
        "publishTelemetry_Timer",
        pdMS_TO_TICKS(_publish_telemetry_period),
        pdTRUE,
        this,
        _publishTelemetryTimerCallback);

    BaseType_t rc = xTaskCreatePinnedToCore(
        _publishTelemtryTask,
        "_publishTelemtryTask",
        5000,
        this,
        1,
        &_publishTelemtry_TH,
        ARDUINO_RUNNING_CORE);
    assert(rc == pdPASS);

    return success;
}

void RemoteInterface::sendDbgCounter()
{
    static int dbg_counter = 0;

    if (_tb_handler != nullptr)
    {
        _tb_handler->sendTelemetryInt("ain_1", dbg_counter++);
    }
    else
    {
        PRINT_LOCK
        printlnA("Trying to send Telemetry DbgCounter without TB handler properly assigned!");
        PRINT_UNLOCK
    }
}

// bool RemoteInterface::sendTelemetryToPlatform(ChargeStation &charge_telem)
// {
//     if (_tb_handler != nullptr)
//     {
//         bool success = true;
//         success &= _tb_handler->sendTelemetryFloat("ain_1", charge_telem.getAcumulateConsumption(1));
//         return success;
//     }
//     return false;
// }

bool RemoteInterface::_suscribeSharedAttributes()
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

void RemoteInterface::_processSharedAttributeUpdate(const Shared_Attribute_Data &data)
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

    // if (data.containsKey("heartbeat"))
    // {
    //     _server_connected = true;
    //     PRINT_LOCK
    //     printlnA("ThingsBoard heartbeat");
    //     PRINT_UNLOCK
    // }

    // if (data.containsKey("date_time"))
    // {
    //     _date_time = data["date_time"].as<time_t>();
    //     // _process_DateTime_Attribute(data);
    //     PRINT_LOCK
    //     printlnA("DateTime Updated");
    //     PRINT_UNLOCK
    // }

    // if (data.containsKey("permissions"))
    // {
    //     _process_FlyPermissions_Attribute(data);
    // }
}

void RemoteInterface::startPublishTelemetryTimer()
{
    if (_publish_telem_timer_on == pdTRUE)
        return;
    BaseType_t rc = xTimerStart(_publishTelemetry_TimerH, portMAX_DELAY);
    assert(rc == pdPASS);
    _publish_telem_timer_on = pdTRUE;
}

void RemoteInterface::stopPublishTelemetryTimer()
{
    if (_publish_telem_timer_on == pdFALSE)
        return;
    BaseType_t rc = xTimerStop(_publishTelemetry_TimerH, portMAX_DELAY);
    assert(rc == pdPASS);
    _publish_telem_timer_on = pdFALSE;
}

void RemoteInterface::_publishTelemetryTimerCallback(TimerHandle_t timer_h)
{
    RemoteInterface *obj = static_cast<RemoteInterface *>(pvTimerGetTimerID(timer_h));

    obj->_send_telem_timeout = pdTRUE;
}

void RemoteInterface::_publishTelemtryTask(void *args)
{
    int dbg_counter = 0;

    RemoteInterface *obj = static_cast<RemoteInterface *>(args);
    // ChargeStation *charge_telem = static_cast<ChargeStation *>(args);

    bool rpc_subscribed = false;

    // time markers
    uint64_t tms = millis();
    uint64_t time_snap = tms;
    // uint64_t send_log_time = tms;
    uint64_t retry_reconnect_time = tms;

    SerialMon.println("[REMOTE_INTERFACE] _sendTelemetry task running...");

    for (;;)
    {
        if (obj->conn_controller->isPlatformConnected())
        {
            if (obj->_tb_handler->connected())
            {
                obj->_server_connected = pdTRUE;

                // if (millis() - check_for_fw_update_time >= 180000) // 3 min

                // if (!rpc_subscribed)
                // {
                //     Serial.println("Subscribing for RPC...");
                //     if (!tb.RPC_Subscribe(rpc_callbacks.cbegin(), rpc_callbacks.cend()))
                //     {
                //         Serial.println("Failed to subscribe for RPC");
                //     }
                //     Serial.println("Subscribe done");
                //     rpc_subscribed = true;
                // }

                if (obj->_send_telem_timeout)
                {
                    obj->_send_telem_timeout = pdFALSE;
                    SerialMon.println("[SERV_MNG] Send Telemetry...");

                    // tb.sendTelemetryInt("DBG_COUNTER", dbg_counter++);

                    bool values = true;

                    if (values)
                    {
                        // obj->_tb_handler->sendTelemetryFloat("ain_1", charge_telem->getState(1));
                        printA("[REMOTE_INTERFACE] Socket1:");
                        // printlnA(charge_telem->getState(1));

                        // StaticJsonDocument<128> data;
                        // data["ts"] = rtc.now() * 1000;
                        // JsonObject values = data.createNestedObject("inputs");
                        // values["din_1"] = plc.getDigInput(0);
                        // values["din_2"] = plc.;
                        // values["ain_1"] = plc.;
                        // values["ain_2"] = plc.;
                        // values["ain_3"] = plc.;
                        // values["ain_4"] = plc.;

                        // char payload[128];
                        // serializeJson(data, payload);

                        // if (tb.sendTelemetryJson(payload))
                        // {
                        //     // SerialMon.println("Send telemetry");
                        // }
                        // else
                        // {
                        //     // SerialMon.println("Fail send Telemetry");
                        // }
                    }
                }

                obj->_tb_handler->loop();
            }
            else
            {
                if (obj->_tb_handler->connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
                {
                    obj->_on_connect();
                }
                else
                {
                    obj->_on_disconnect();
                }
            }
        }
    }
}

//*RPC
/// @brief Processes function for RPC call "Start Charge"
/// @param data Data containing the rpc data that was called -> port id
/// @return Response that should be sent to the cloud. Useful for getMethods
RPC_Response startCharge(const RPC_Data &data)
{
    SerialMon.println("[SERV_MNG]-[RPC] Received the set Valve method");

    // Process data
    int port_id = data;
    // TODO process data

    printA("Enable port with the given ID --ID:");
    printlnA(port_id);

    // Just an response example
    return RPC_Response(RPC_START_CHARGE_METHOD, port_id);
}