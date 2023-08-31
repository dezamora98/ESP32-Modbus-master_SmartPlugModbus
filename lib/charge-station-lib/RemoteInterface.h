#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#pragma once

#include <Arduino.h>
#include "charge_station_config.h"
#include <ArduinoJson.h>

//* Interfaz para trabajo con la Plataforma //

class RemoteInterface
{
public:
    RemoteInterface();
    ~RemoteInterface();

    //* TELEMETRY
    // virtual void sendPortInfo(int port, json &pinfo) = 0;
    // virtual void sendAlarm() = 0;
    // virtual void sendError() = 0;
    // virtual void sendPortStateUpdate(int port, String state) = 0;

    // virtual bool sendTelemetryToPlatform() = 0;

    //* RPC
    // virtual void set_callback_on_getPortInfoRequest(callbackFunction newFunction) = 0;
    // virtual void set_callback_on_reservePortRequest(void(*callback)) = 0;
    // virtual void set_callback_on_abortPortRequest(void(*callback)) = 0;

    // virtual bool isConnected() = 0;
    // virtual void set_callback_on_disconnect(void(*callback)) = 0;

    virtual bool init() = 0;

    // void startPublishTelemetryTimer();
    // void stopPublishTelemetryTimer();
    // uint32_t publishTelemetryPeriod() const { return _publish_telemetry_period; }
    // void setPublishTelemetryPeriod(const uint32_t &publish_telemetry_period) { _publish_telemetry_period = publish_telemetry_period; }

private:
};

#endif