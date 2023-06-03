#include <Arduino.h>
#include <SerialDebug.h> //https://github.com/JoaoLopesF/SerialDebug
#include <ChargeController.h>
#include <SensorMonitorInterface.h>
#include <AnalogSensorMonitor.h>
#include <charge_station_config.h>

void setup()
{
  Serial.begin(115200);
  printlnA(F("**** Setup: initializing ..."));

  SensorMonitorInterface *sensor1 = new AnalogSensorMonitor(AnalogIn_1);
  SensorMonitorInterface *sensor2 = new AnalogSensorMonitor(AnalogIn_2);

  ChargeController socket1(sensor1, Relay_1, 0001);
  ChargeController socket2(sensor2, Relay_2, 0002);
}

void loop()
{
}