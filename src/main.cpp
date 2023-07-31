#include <Arduino.h>
#include <charge_station_config.h>
#include <ConnectionController.h>
#include <WifiController.h>
#include <ChargeController.h>
#include <SensorMonitorInterface.h>
#include <AnalogSensorMonitor.h>
#include <ServerController.h>
#include <SerialDebug.h>

WifiController wifi_controller;
ConnectionController *conn_controller = ConnectionController::Instance();
// ServerController *server_controller;

/*
- Clase Remote Interface abstracta
  - Clase intermedia entre el modulo A9 (Utiliza la libreria de Daniel)
  - Clase mediante ThingsBoard comunicacion mediante WIFI (inicialmente para pruebas)

- ## Clase mediante Thingsboard necesita de un WifiClient
  IDEN A VELKYS PLC -> WifiController + WifiClient + Connection Manager
  Valorar en Connection Manager el uso de otra estructura sin necesidad de cada
  vez que desee modificar lo q voy a mandar tenga que cambiar en ese lugar

- Iniciar el Host - Configuraciones necesarias ?
- Como vuelvo a poner el modo Host una vez hechas las configuraciones necesarias ?

- Las Configuraciones linkearlas a la una clase de SettingsManager

- Senalizaciones LED -> Clase independiente activacion mediante las variables de charge controller *estado


RECIBIR DESDE LA PLATAFORMA
- comando de inicio de carga o parada
- id de la toma
- tiempo maximo de carga

CONFIGURACIONES
- Pico maximo de corriente a parar la carga
- Configuraciones del sensor
-
*/

// ConnectionController *connection_controller;

void setup()
{
  SerialMon.begin(SERIAL_DEBUG_BAUD);
  delay(1000);

  // wifi_controller.resetWifiSettings();
  wifi_controller.set_credentials("giselle98", "giselle123");
  wifi_controller.setAccessPoint(false);
  delay(100);
  // Inicializar la conexión a ThingsBoard mediante WiFi
  conn_controller->platformConnectionInit(ConnectionType::WIFI);
  ConnectionState con_state = conn_controller->getConnectionState();

  // server_controller.init(thingsBoard);

  // SensorMonitorInterface *sensor1;
  // SensorMonitorInterface *sensor2;

  // sensor1 = new AnalogSensorMonitor(AnalogIn_1);
  // sensor2 = new AnalogSensorMonitor(AnalogIn_2);

  AnalogSensorMonitor sensor1(AnalogIn_1);
  AnalogSensorMonitor sensor2(AnalogIn_2);

  ChargeController socket1;
  ChargeController socket2;

  socket1.init(&sensor1, Relay_1, 1);
  socket2.init(&sensor2, Relay_2, 2);

  // bool success = wifi_controller.initWifi();
}

void loop()
{
  if (conn_controller->isPlatformConnected())
  {
    Serial.println("Conection active");
  }
  else
  {
    Serial.println("NO CONECTION");
  }

  if (conn_controller->getConnectionState() == ConnectionState::CONNECTED)
  {
    Serial.println("Conexión a ThingsBoard establecida mediante WiFi");
    Serial.println("EN TALLA BEIBI");
  }
  delay(1000);
  // // Enviar telemetría utilizando ServerController
  // ChargeTelemetry chargeTelemetry;
  // // cargar la telemetría
  // server_controller.sendTelemetryToPlatform(chargeTelemetry);
  // delay(1000);
}