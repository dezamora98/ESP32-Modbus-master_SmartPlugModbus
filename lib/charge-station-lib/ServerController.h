#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H
#pragma once

#include <Arduino.h>
#include <ChargeTelemetry.h>
#include <ThingsBoard.h>
#include <SerialDebug.h>

class ServerController
{
public:
    ServerController();
    ~ServerController();

    bool init(ThingsBoard *tb_h);
    void sendDbgCounter();
    bool sendTelemetryToPlatform(const ChargeTelemetry &p_telem);

    // Properties functions
    static uint32_t getDateTimeRefreshInterval() { return _dt_refresh_interval; }
    static void _processSharedAttributeUpdate(const Shared_Attribute_Data &data);

private:
    // Methods
    bool _suscribeSharedAttributes();

private:
    ThingsBoard *_tb_handler;

    static Shared_Attribute_Callback _processSharedattributesUpdate_Callback;

    TaskHandle_t _checkServerConnection_TH;

    // bool _rpc_subscribed;
    bool _attributes_subscribed;

    static bool _server_connected;

    static uint32_t _dt_refresh_interval;
};

#endif