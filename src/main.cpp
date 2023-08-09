#include <Arduino.h>
#include <charge_station_config.h>
#include <ConnectionController.h>
#include <RemoteInterface.h>
#include <ChargeStation.h>
#include <WifiController.h>
#include <ChargePortController.h>
#include <SensorMonitorInterface.h>
#include <AnalogSensorMonitor.h>
#include <SerialDebug.h>

WifiController wifi_controller;
ConnectionController *conn_controller = ConnectionController::Instance();
ThingsBoard *tb_h;
ChargeStation charge_station;

/*
//* RPC
  # > startCharge (port number)
  # > tiempo de Carga
  # > abortCharge (port number)
  # > listAvailablePorts

//* Telemetry
  # > port_state_charge_ack
  # > consum_report

//* Configuraciones en modo HOST SettingsManager
  - id_dispositivo
  - localizacion
  - temas de conexion (modo WIFI - A9)
*/

/* StateMachine behavior
    # > [x] reserve
    # > [x] plug_in
    # > [x] start_charge_when_plug_in
    # > [ ] cancel_reserve -> por timeout o por una llamada directa a un metodo
    # > [x] start_charge_when_reserve
    # > [x] plug_out
    # > [x] charge_complete
    # > [x] abort_charge
    # > [ ] reset_port


- El modo host se activa de forma manual

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
bool x;

void setup()
{
  SerialMon.begin(SERIAL_DEBUG_BAUD);
  delay(1000);

  Serial.println(F("**** Setup: initializing ..."));

  // wifi_controller.resetWifiSettings();

  // TODO: Inicializar modo Access Point

  //* Guardando las credenciales a la wifi a la que me voy a conectar, normalmante se realiza la config desde la web embebida
  wifi_controller.set_credentials("giselle98", "giselle123");
  wifi_controller.setAccessPoint(false);
  delay(1000);

  // //* Inicializar la conexión a ThingsBoard mediante WiFi
  conn_controller->platformConnectionInit(ConnectionType::WIFI);
  ConnectionState con_state = conn_controller->getConnectionState();
  tb_h = conn_controller->getThingsBoardHandler();

  AnalogSensorMonitor sensor1(AnalogIn_1);
  AnalogSensorMonitor sensor2(AnalogIn_2);

  ChargePortController socket1;
  ChargePortController socket2;

  socket1.init(&sensor1, Relay_1, DigitalIn_1, 1);
  socket2.init(&sensor2, Relay_2, DigitalIn_2, 2);

  int location[2] = {2, 2};
  charge_station.init(2, "STATION 1", location);

  charge_station.addChargePort(socket1);
  charge_station.addChargePort(socket2);

  Serial.println("----------------------------------------------------");
  Serial.println("Informacion de los puertos de carga");
  Serial.print("PORT ID: ");
  Serial.println(socket1.getID());
  Serial.print("PORT ID: ");
  Serial.println(socket2.getID());
  Serial.print("NOMBRE DE LA ESTACION:");
  Serial.println(charge_station.getDeviceName());
  Serial.println("----------------------------------------------------");

  Serial.println(charge_station._ChargePortControllers[1].getID());
  Serial.println(F("*** Setup end"));
}

void loop()
{
  // TODO Si el PIN del modo HOST esta activo levanto access point paro el resto de las comunicaciones - MODO CONFIGURACIONES
  delay(1000);
  charge_station.setState(1, PLUGGED);
  Serial.println("----------------------------------------------------------------");
  // if (conn_controller->isPlatformConnected()) // si se conecto a la plataforma
  // {
  //   // Serial.println("CONNECTED TO THINGSBOARD");
  //   // remote_interface.init(tb_h);
  //   // Serial.println(telemetry_data.consumption1);
  //   // x = remote_interface.sendTelemetryToPlatform(telemetry_data);
  // }
  // else
  // {
  //   Serial.println("THINGSBOARD - DISCONNECT");
  // }
}
