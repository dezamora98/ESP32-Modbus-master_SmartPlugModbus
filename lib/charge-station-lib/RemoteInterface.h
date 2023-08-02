#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#pragma once

#include <Arduino.h>
// #include <TimeLib.h>
#include <WifiClientController.h>
#include <ChargeController.h>
#include <ChargerContainer.h>
#include <ThingsBoard.h>
#include <ConnectionController.h>

#include "charge_station_config.h"

//* Interfaz para trabajo con Thingsboard //

class RemoteInterface
{
public:
    RemoteInterface();
    ~RemoteInterface();

    ChargeController charge_station_controller;
    ConnectionController *conn_controller;
    bool init(ThingsBoard *tb_h);

    // asyncronous request
    // pago realizado por el usuario

    //* TELEMETRY
    bool sendTelemetryToPlatform(ChargerContainer &charge_telem);
    void sendDbgCounter();

    //* Methods
    void startPublishTelemetryTimer();
    void stopPublishTelemetryTimer();
    uint32_t publishTelemetryPeriod() const { return _publish_telemetry_period; }
    void setPublishTelemetryPeriod(const uint32_t &publish_telemetry_period) { _publish_telemetry_period = publish_telemetry_period; }

private:
    // Methods
    bool _suscribeSharedAttributes();
    static void _processSharedAttributeUpdate(const Shared_Attribute_Data &data);

    static void _publishTelemtryTask(void *args);
    static TaskHandle_t _publishTelemtry_TH;

    TimerHandle_t _publishTelemetry_TimerH;
    static void _publishTelemetryTimerCallback(TimerHandle_t timer_h);

    void _on_connect();
    void _on_disconnect();

    ThingsBoard *_tb_handler;

    static Shared_Attribute_Callback _processSharedattributesUpdate_Callback;
    TaskHandle_t _checkServerConnection_TH;

    bool _rpc_subscribed;

    bool _attributes_subscribed;

    static bool _server_connected;

    uint32_t _publish_telemetry_period;

    BaseType_t _send_telem_timeout;
    BaseType_t _publish_telem_timer_on;

    // RPC
    RPC_Response startCharge(const RPC_Data &data);
};

#endif