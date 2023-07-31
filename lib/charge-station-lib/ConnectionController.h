#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H
#pragma once

#include <Arduino.h>
#include <WifiClientController.h>
#include <WifiController.h>
#include <ThingsBoard.h>
#include <SerialDebug.h>

/*  -------------------------------------------------------------------------------------- */
//* Controlador centralizado para todas las vias de conexion con el servidor de ThingsBoard
/*  --------------------------------------------------------------------------------------- */

enum class ConnectionType
{
    WIFI,
    SIM800L,
    SIM7000G,
    LORA,
    ETHERNET
};

enum class ConnectionEvents : uint32_t
{
    EVENT_CONNECTED = 0x000001,
    EVENT_DISCONNECTED = 0x000002,
    EVENT_CONNECTION_FAIL = 0x000004,
    EVENT_CONNECTION_SWITCHED = 0x000010,
    EVENT_SERVER_SHARED_ATTRIBUTES_SUSCRIBED = 0x000020
};

enum class ConnectionState : uint32_t
{
    IDDLE,
    DISCONNECTED,
    CONNECTED
};

class ConnectionController
{
public: // Private Methods
    ~ConnectionController();
    static ConnectionController *Instance();

    // Por ahora no seria necesario declararlo como estatico...
    // Tampoco se asignaran funciones de acceso
    static EventGroupHandle_t connectionEventsHandler;

    ThingsBoard *getThingsBoardHandler();

    void platformConnectionInit(ConnectionType conn_type = ConnectionType::WIFI);

    bool isPlatformConnected();

    // Properties Public Access Members
    ConnectionState getConnectionState();

    ConnectionType getCurrentConnectionType() { return _current_connection_type; }

protected: // Protected Methods
    ConnectionController();

private:
    static void _platformKeepConnectionAlive(void *args);

    // Properties Private Access Members
    void _setConnectionState(ConnectionState con_state);

private: // Private Members
    static ConnectionController *_connection_controller;

    WifiClientController *_wifi_client_controller;
    WifiController *_wifi_controller;
    Client *_client;
    ThingsBoard *_tb_handler;

    SemaphoreHandle_t _conn_state_mutex;

    // Properties
    ConnectionState _connection_state;
    ConnectionType _current_connection_type;
};

#endif