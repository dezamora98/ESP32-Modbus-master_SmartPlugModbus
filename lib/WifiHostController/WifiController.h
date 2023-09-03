#ifndef WifiController_H
#define WifiController_H

#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WifiClientController.h>
#include <SPIFFS.h>
#include "charge_station_config.h"
#include <ArduinoJson.h>
#include <DNSServer.h>

class WifiController
{
public:
    AsyncWebServer _server;
    AsyncWebSocket _ws;
    WifiClientController _wifiClient;
    DNSServer _dnsServer;

    WifiController() : _server(80), _ws("/ws") {}

    bool initWifi();          // inicia la wifi ya sea en modo ap o station
    void resetWifiSettings(); //

    void setAccessPoint(bool ap);
    const char *get_ssid() { return wifi_ssid; }
    const char *get_pass() { return wifi_passw; }

    void set_credentials(String pass, String ssid)
    {
        strlcpy(wifi_passw, pass.c_str(), sizeof(pass));
        Serial.println(wifi_passw);
        strlcpy(wifi_ssid, ssid.c_str(), sizeof(ssid));
        Serial.println(wifi_ssid);
        initSPIFFS();
        saveWifiSettings();
    }

private:
    // ---------------------------------------------------------
    // Zona Configuración General
    // ---------------------------------------------------------
    char id[30];   // ID del dispositivo
    int bootCount; // Número de reinicios
    // ---------------------------------------------------------
    // Zona WIFI
    // ---------------------------------------------------------
    boolean wifi_staticIP;      // Uso de IP Estática
    char wifi_ssid[30];         // Red WiFi
    char wifi_passw[30];        // Contraseña de la Red WiFi
    char wifi_ip_static[15];    // IP Estático
    char wifi_gateway[15];      // Gateway
    char wifi_subnet[15];       // Subred
    char wifi_primaryDNS[15];   // DNS primario
    char wifi_secondaryDNS[15]; // DNS secundario
    // ---------------------------------------------------------
    // Zona AP
    // ---------------------------------------------------------
    bool ap_accessPoint; // Uso de Modo AP
    char ap_ssid[31];    // SSID AP
    char ap_passw[63];   // Password AP
    int ap_canalap;      // Canal AP
    int ap_hiddenap;     // Es visible o no el AP (0 - Visible 1 - Oculto)
    int ap_connetap;     // Número de conexiones en el AP

    uint8_t ip[4];

    int _wifiMode;

    /*  Retorna IPAddress en formato "n.n.n.n" de IP a String */
    static String ipStr(const IPAddress &ip);

    /*  Create a Unique ID from MAC address */
    static String idUnique();
    String deviceID();
    String hexStr(const unsigned long &h, const byte &L = 8);

    bool saveWifiSettings();
    bool readWifiSettings();

    bool initSPIFFS();
    String readSPIFFS(const char *path);
    static void writeSPIFFS(const char *path, const char *message);

    void initAPModeESP();
    void initServerHandle();

    void InitWebSockets();
    static void processRequest(AsyncWebSocketClient *client, String request);
    void updateGPIOData(String input, float value);

    static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

    // static void _dnsProcess(void *args);

    // static void setOnOff(String id, bool state, PlcVelkys *_plcVelkys);
    // static void setPWM(String id, int pwm, PlcVelkys *_plcVelkys);
    // static void doAction(String actionId, PlcVelkys *_plcVelkys);

    // static void updateGpioDataTask(void *args);
};
#endif