#include "WifiClientController.h"

#include <charge_station_config.h>

const uint32_t WIFI_MAX_TIME_MS_RETRY = 1000;

WifiClientController::WifiClientController() : _wifi_client(nullptr), _keepWifiAlive_TH(nullptr)
{
}

WifiClientController::~WifiClientController()
{
    delete _wifi_client;
}

bool WifiClientController::init(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;
    stopTask = false;

    if (_keepWifiAlive_TH != nullptr)
    {
        if (WiFi.isConnected())
        {
            WiFi.disconnect();
            // TODO: ver si hay que eliminar y volver a crear la instancia de WifiClient
            return true;
        }
        // vTaskDelete(_keepWifiAlive_TH);
        // delay(100);
    }

    BaseType_t rc = xTaskCreatePinnedToCore(
        _keepWifiAlive,
        "_keepWifiAlive",
        3000,
        this,
        1,
        &_keepWifiAlive_TH,
        CONFIG_ARDUINO_RUNNING_CORE);
    assert(rc == pdPASS);

    return true;
}

void WifiClientController::setStopTask(bool stop)
{
    Serial.println("sToping task");
    stopTask = stop;
}

void WifiClientController::suspendKeepWifiAliveTask()
{
    Serial.println("[WIFI_CLIENT] Suspend Keep Alive Task");
}

void WifiClientController::resumeKeepWifiAliveTask()
{
    vTaskResume(_keepWifiAlive_TH);
}

bool WifiClientController::isConnected()
{
    return WiFi.isConnected();
}

bool WifiClientController::disconnect()
{
    return WiFi.disconnect();
}

Client *WifiClientController::getClient()
{
    if (_wifi_client == nullptr)
    {
        _wifi_client = new WiFiClient;
    }
    return _wifi_client;
}

void WifiClientController::_keepWifiAlive(void *args)
{
    WifiClientController *obj = static_cast<WifiClientController *>(args);

    for (;;)
    {
        Serial.println(obj->stopTask);
        if (!obj->stopTask)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                SerialMon.println("[WIFI] still connected");
                SerialMon.print("[WIFI] Status");
                SerialMon.println(WiFi.isConnected());
                delay(WIFI_MAX_TIME_MS_RETRY);
                continue;
            }

            SerialMon.println("[WIFI] Connecting...");

            Serial.println("WIFI CLIENT Controller");
            Serial.println(obj->_ssid.c_str());
            Serial.println(obj->_password.c_str());

            WiFi.begin(obj->_ssid.c_str(), obj->_password.c_str());
            // WiFi.begin("Redmi 9", "theclockwork94");

            unsigned long startAttemptTime = millis();

            while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_MAX_TIME_MS_RETRY)
            {
            }

            if (WiFi.status() != WL_CONNECTED)
            {
                SerialMon.println("[WIFI] FAILDED");
                delay(20000);
                // continue;
            }
        }
        else
        {
            Serial.println("STOP TASK ");
        }
    }
}
