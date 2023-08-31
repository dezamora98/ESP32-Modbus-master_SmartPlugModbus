#ifndef TBREMOTEINTERFACE_H
#define TBREMOTEINTERFACE_H

#pragma once

#include <Arduino.h>
#include <RemoteInterface.h>
#include "charge_station_config.h"
#include <ArduinoJson.h>
#include <ThingsBoard.h>
#include <WifiClientController.h>
#include <WifiController.h>
// typedef void (*callbackFunction)(void);

//* Interfaz para trabajo con Thingsboard //
class TBRemoteInterface : public RemoteInterface
{
public:
    friend class WifiController;

    TBRemoteInterface(void (*callback)());
    ~TBRemoteInterface();

    WifiController _wifi_controller;
    // ThingsBoard _tb_handler;
    WifiClientController _client;

    bool init() override;

    //* TELEMETRY
    // void sendPortInfo(int port, JSON &pinfo);
    // void sendAlarm();
    // void sendError();
    // void sendPortStateUpdate(int port, String state);

    // bool sendTelemetryToPlatform();

    //* RPC
    /* ---------------------------- PORT INFO REQUEST --------------------------- */
    void (*func_callback)();
    void set_callback_on_getPortInfoRequest(void (*callback)()); //! callback funtion not working
    void handle_getPortInfoRequest();

    // void set_callback_on_reservePortRequest(void(*callback)) override;
    // void set_callback_on_abortPortRequest(void(*callback)) override;
    // bool isConnected() override;
    // void set_callback_on_disconnect(void(*callback)) override;

    void startPublishTelemetryTimer();
    void stopPublishTelemetryTimer();
    uint32_t publishTelemetryPeriod() const { return _publish_telemetry_period; }
    void setPublishTelemetryPeriod(const uint32_t &publish_telemetry_period) { _publish_telemetry_period = publish_telemetry_period; }
    void _on_connect();
    void _on_disconnect();

private:
    Client *_network_client = nullptr;

    // static callbackFunction _voidCallback;

    // Methods
    // bool _suscribeSharedAttributes();
    /* ---------------------------------- TARK ---------------------------------- */
    static void _publishTelemtryTask(void *args);
    static TaskHandle_t _publishTelemetry_TH;

    TimerHandle_t _publishTelemetry_TimerH;
    static void _publishTelemetryTimerCallback(TimerHandle_t timer_h);

    TaskHandle_t _checkServerConnection_TH;

    uint32_t _publish_telemetry_period;

    BaseType_t _send_telem_timeout;
    BaseType_t _publish_telem_timer_on;
    BaseType_t _server_connected; // pdTRUE when connected to server
    int _server_reconnection_try_count = 0;
};

#endif
