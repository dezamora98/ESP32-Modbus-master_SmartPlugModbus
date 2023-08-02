#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// constexpr char CURRENT_FIRMWARE_TITLE[] = "";
// constexpr char CURRENT_FIRMWARE_VERSION[] = "";

//* CHARGE STATION
#define CHARGING_SOCKETS 4 // NUMERO DE PUERTOS DE CARGA
#define VOLTAGE 110
#define MEASUREMENT_TIME 1000 // 1 second resolution
#define MAX_CURRENT 1000      //! PONER UN VALOR POR DEFECTO DEL CONSUMO DE CORRIENTE MAXIMO

//* ESP32 GPIO PIN DEFINITION
// Salidas Analógicas
#define DAC_1 25
#define DAC_2 26

// Botones de control de la pantalla
#define Button1 35
#define Button2 32

// Entradas Analógicas
// Configurables por Hardware para 5V o 10V
#define AnalogIn_1 36 // (SENSOR VP)
#define AnalogIn_2 39 // (SENSOR VN)
#define AnalogIn_3 34
#define AnalogIn_4 33

#define Relay_1 12
#define Relay_2 14
#define Relay_3 27
#define Relay_4 13

#define DigitalOut_1 15
#define DigitalOut_2 2
#define DigitalOut_3 4

#define DigitalIn_1 16
#define DigitalIn_2 17

// SPI, expuesto para expansiones
#define SPI_SS 5
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

// Bus I2C Compartido tanto para el control de la OLED, como para expansión
#define I2C_SCL 22
#define I2C_SDA 21

//* UART Configuration

// #define SerialMon Serial2
#define SerialMon Serial
#define SERIAL_DEBUG_BAUD 115200

// #define SERIAL_MON_TX 18
// #define SERIAL_MON_RX 19
// #define SERIAL_MON_TX 32
// #define SERIAL_MON_RX 33

constexpr char CONNECTION_MODE[] = "WIFI";

//* WiFi Configuration
// Roger
//  constexpr char DEFAULT_WIFI_SSID[] = "Redmi 9";
//  constexpr char DEFAULT_WIFI_PASS[] = "theclockwork94";

// Giselle
//  constexpr char DEFAULT_WIFI_SSID[] = "giselle98";
//  constexpr char DEFAULT_WIFI_PASS[] = "giselle123";

//* Wifi HOST Configuration
constexpr char DEFAULT_WIFI_AP_SSID[] = "Velkys-PLC";
constexpr char DEFAULT_WIFI_AP_PASS[] = "12345678";
const byte DNSSERVER_PORT = 53; // DNS

//* SERVER Configuration
constexpr char THINGSBOARD_SERVER[] = "152.206.85.71";
constexpr uint16_t THINGSBOARD_PORT = 1883U;
constexpr char TOKEN[] = "vW9NYuIisNflCuGz92HA"; // plc-iot

//* DEFAULTS

constexpr uint16_t PUBLISH_TELEMETRY_PERIOD = 1000U;

#endif // PLCIOT_CONFIG_H
