#include "TBRemoteInterface.h"
#include "auxiliars.h"

// Shared_Attribute_Callback RemoteInterface::_processSharedattributesUpdate_Callback(_processSharedAttributeUpdate);

constexpr uint32_t MAX_MESSAGE_SIZE = 512U;

constexpr const char RPC_VALVE_METHOD[] = "setValveState";
constexpr const char RPC_VALVE_KEY[] = "response_valve_state";

TaskHandle_t TBRemoteInterface::_publishTelemetry_TH = nullptr;
TBRemoteInterface *obj;

TBRemoteInterface::TBRemoteInterface(void (*callback)())
{
    Serial.println("Setting callback on getPortInfoRequest");
    Serial.println("VOIDCALLBACK::INIT!");
    func_callback = callback;
    _server_connected = pdFALSE;
    _send_telem_timeout = pdFALSE;
    _publish_telem_timer_on = pdFALSE;
    _publish_telemetry_period = PUBLISH_TELEMETRY_PERIOD;
}

TBRemoteInterface::~TBRemoteInterface()
{
}

bool TBRemoteInterface::init()
{
    Serial.println("[TB_REMOTE_INTERFACE] init");
    // _wifi_controller->resetWifiSettings();

    // //* Guardando las credenciales a la wifi a la que me voy a conectar, normalmante se realiza la config desde la web embebida
    _wifi_controller.set_credentials("giselle98", "giselle123"); //- Esto hay que quitarlo
    delay(100);
    if (!_wifi_controller.initWifi())
    {
        _network_client = _wifi_controller._wifiClient.getClient();
    }

    bool success = true;

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
        &_publishTelemetry_TH,
        ARDUINO_RUNNING_CORE);
    assert(rc == pdPASS);

    return success;
}

// void TBRemoteInterface::sendAlarm()
// {
//     bool info = true; //! delete this

//     if (_tb_handler != nullptr)
//     {
//         bool success = true;
//         success &= _tb_handler->sendTelemetryBool("ain_1", info); // TODO: cambiar info por la alarma
//         Serial.println("sending telemetry alarm");
//         // return success;
//     }
//     // return false;
// }

// void TBRemoteInterface::sendError()
// {
//     Serial.println("Send error via TBRemoteInterface");
// }

// void TBRemoteInterface::sendPortStateUpdate(int port, String state)
// {
//     _tb_handler->sendTelemetryInt("port", port);
//     _tb_handler->sendTelemetryString("state", state.c_str());
// }

void TBRemoteInterface::set_callback_on_getPortInfoRequest(void (*callback)())
{
    Serial.println("Setting callback on getPortInfoRequest");
    Serial.println("VOIDCALLBACK::INIT!");
    func_callback = callback;
}

void TBRemoteInterface::handle_getPortInfoRequest()
{
    Serial.println("Handle port info request");
    (*func_callback)(); // llamar al puntero a función
}

/// @brief Processes function for RPC call "Valve Control"
/// @param data Data containing the rpc data that was called and its current value
/// @return Response that should be sent to the cloud. Useful for getMethods
RPC_Response processValveChange(const RPC_Data &data)
{
    SerialMon.println("[SERV_MNG]-[RPC] Received the set Valve method");
    obj->handle_getPortInfoRequest();

    return RPC_Response(RPC_VALVE_KEY, 1);
}

const std::array<RPC_Callback, 1U> rpc_callbacks = {
    RPC_Callback{RPC_VALVE_METHOD, processValveChange},
};

void TBRemoteInterface::startPublishTelemetryTimer()
{
    if (_publish_telem_timer_on == pdTRUE)
        return;
    BaseType_t rc = xTimerStart(_publishTelemetry_TimerH, portMAX_DELAY);
    assert(rc == pdPASS);
    _publish_telem_timer_on = pdTRUE;
}

void TBRemoteInterface::_on_connect()
{
    _server_connected = pdTRUE;
    _server_reconnection_try_count = 0;
    startPublishTelemetryTimer();
    SerialMon.println("[SERV_MNG] Server Connected!");
}

void TBRemoteInterface::_on_disconnect()

{
    stopPublishTelemetryTimer();
    _server_connected = pdFALSE;
    SerialMon.println("[SERV_MNG] Server Disconnected!");
}

void TBRemoteInterface::stopPublishTelemetryTimer()
{
    if (_publish_telem_timer_on == pdFALSE)
        return;
    BaseType_t rc = xTimerStop(_publishTelemetry_TimerH, portMAX_DELAY);
    assert(rc == pdPASS);
    _publish_telem_timer_on = pdFALSE;
}

void TBRemoteInterface::_publishTelemetryTimerCallback(TimerHandle_t timer_h)
{
    TBRemoteInterface *obj = static_cast<TBRemoteInterface *>(pvTimerGetTimerID(timer_h));

    obj->_send_telem_timeout = pdTRUE;
}

void TBRemoteInterface::_publishTelemtryTask(void *args)
{
    int dbg_counter = 0;

    TBRemoteInterface *obj = static_cast<TBRemoteInterface *>(args);
    // ChargeStation *charge_telem = static_cast<ChargeStation *>(args);
    ThingsBoard tb(*(obj->_network_client), MAX_MESSAGE_SIZE);

    bool rpc_subscribed = false;

    // time markers
    uint64_t tms = millis();
    uint64_t time_snap = tms;
    // uint64_t send_log_time = tms;
    uint64_t retry_reconnect_time = tms;

    bool should_retry_reconnect = true;
    bool first_reconnect_attempt = true;

    SerialMon.println("[REMOTE_INTERFACE] _sendTelemetry task running...");
    obj->_server_reconnection_try_count = 0;

    for (;;)
    {
        if (obj->_client.isConnected())
        {
            if (tb.connected())
            {
                obj->_server_connected = pdTRUE;

                // if (millis() - check_for_fw_update_time >= 180000) // 3 min
                if ((millis() - time_snap) > 5000)
                {
                    time_snap = millis();
                    SerialMon.println("[SERV_MNG] Server connection alive");
                }

                if (!rpc_subscribed)
                {
                    Serial.println("Subscribing for RPC...");
                    if (!tb.RPC_Subscribe(rpc_callbacks.cbegin(), rpc_callbacks.cend()))
                    {
                        Serial.println("Failed to subscribe for RPC");
                    }
                    Serial.println("Subscribe done");

                    rpc_subscribed = true;
                }

                if (obj->_send_telem_timeout)
                {
                    obj->_send_telem_timeout = pdFALSE;
                    SerialMon.println("[SERV_MNG] Send Telemetry...");

                    // tb.sendTelemetryInt("DBG_COUNTER", dbg_counter++);

                    bool values = true;

                    if (values)
                    {
                        tb.sendTelemetryFloat("ain_1", 5);
                    }
                }

                tb.loop();
            }
            else
            {
                if (tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
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
