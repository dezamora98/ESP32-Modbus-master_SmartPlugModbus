#ifndef WIFICLIENTCONTROLLER_H
#define WIFICLIENTCONTROLLER_H

#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WifiClientController
{
public:
    WifiClientController();
    ~WifiClientController();

    bool init(const char * ssid, const char * password);
    bool isConnected();
    bool disconnect();
    Client * getClient();

    void setStopTask(bool stop);

    void suspendKeepWifiAliveTask();
    void resumeKeepWifiAliveTask();

private:
    WiFiClient * _wifi_client;
    bool stopTask;
    TaskHandle_t _keepWifiAlive_TH;
    static void _keepWifiAlive(void *args);

    String _ssid;
    String _password;

};

#endif