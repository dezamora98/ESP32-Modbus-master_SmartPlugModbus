#include "WifiController.h"

#define SSID_PATH "/WIFI/SSID"
#define PASS_PATH "/WIFI/PASS"

IPAddress apIP(192, 168, 4, 1);
const size_t JSON_CONFIG = JSON_OBJECT_SIZE(5) + 500;

/* -------------------------------------------------------------------------- */
//*                               INIT SECTION                                */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initializes the WiFi connection for the WifiController class.
 * @return True if the WiFi is in access point mode, false otherwise.
 */
bool WifiController::initWifi()
{
    Serial.println("[WIFI_CONTROLLER] INIT WIFI");
    initSPIFFS();
    readWifiSettings();

    if (wifi_ssid == "" || wifi_passw == "")
    {
        ap_accessPoint = 1;
    }
    // ---------------------------------------------------------
    //* WIFI STATION
    // ---------------------------------------------------------
    if (!ap_accessPoint)
    {
        WiFi.disconnect(WIFI_STA);
        _wifiClient.init(wifi_ssid, wifi_passw); // inicia el cliente wifi, stopTask = False
        delay(100);
        Serial.println("[WIFI_CONTROLLER] WIFI station ");
        // _connectionManager.init(_wifiClient);
        // TODO PONER EL MODO DE CONEXION ES MEDIANTE WIFI ESTO ES EN CONNECTION CONTROLLER
        _wifiMode = WIFI_STA;
        return false;
    }
    // ---------------------------------------------------------
    //* ACCESS POINT
    // ---------------------------------------------------------
    else if (ap_accessPoint || _wifiMode == WIFI_AP)
    {
        initAPModeESP();
        //
        SerialMon.println("[WIFI_CONTROLLER] Access Point");
        _wifiClient.setStopTask(true);
        initServerHandle();
        InitWebSockets();
        return true;
    }
}

/**
 * @brief Initialize Server Hazndler
 *
 */
void WifiController::initServerHandle()
{
    SerialMon.println("[WIFI_CONTROLLER] Initializing Server Handle");

    /**********************************************/
    _server.serveStatic("/bootstrap-responsive.min.css", SPIFFS, "/bootstrap-responsive.min.css").setDefaultFile("bootstrap-responsive.min.css");
    _server.serveStatic("/bootstrap.min.css", SPIFFS, "/bootstrap.min.css").setDefaultFile("bootstrap.min.css");
    _server.serveStatic("/styles.css", SPIFFS, "/styles.css").setDefaultFile("styles.css");
    _server.serveStatic("/jquery.easy-pie-chart.css", SPIFFS, "/jquery.easy-pie-chart.css").setDefaultFile("jquery.easy-pie-chart.css");
    _server.serveStatic("/bootstrap.min.js", SPIFFS, "/bootstrap.min.js").setDefaultFile("bootstrap.min.js");
    _server.serveStatic("/jquery-1.9.1.min.js", SPIFFS, "/jquery-1.9.1.min.js").setDefaultFile("jquery-1.9.1.min.js");
    _server.serveStatic("/jquery.easy-pie-chart.js", SPIFFS, "/jquery.easy-pie-chart.js").setDefaultFile("jquery.easy-pie-chart.js");
    _server.serveStatic("/modernizr.min.js", SPIFFS, "/modernizr.min.js").setDefaultFile("modernizr.min.js");
    _server.serveStatic("/sweetalert2.min.css", SPIFFS, "/sweetalert2.min.css").setDefaultFile("sweetalert2.min.css");
    _server.serveStatic("/sweetalert2.min.js", SPIFFS, "/sweetalert2.min.js").setDefaultFile("sweetalert2.min.js");
    _server.serveStatic("/scripts.js", SPIFFS, "/scripts.js").setDefaultFile("scripts.js");
    _server.serveStatic("/glyphicons-halflings.png", SPIFFS, "/glyphicons-halflings.png").setDefaultFile("glyphicons-halflings.png");
    _server.serveStatic("/glyphicons-halflings-white.png", SPIFFS, "/glyphicons-halflings-white.png").setDefaultFile("glyphicons-halflings-white.png");
    _server.serveStatic("/logo.png", SPIFFS, "/logo.png").setDefaultFile("logo.png");

    _server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    _server.serveStatic("/configwifi", SPIFFS, "/configwifi");
    // ---------------------------------------------------------
    //* INDEX PAGE
    // ---------------------------------------------------------
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
        String req;
        if (request->hasArg("switch1"))
            req = request->arg("switch1");
        req.trim();
        if (req = "1")
        {
            // _plcVelkys.setRelay(0, true);
            SerialMon.println("[WIFI_CONTROLLER] Opens the relay 1");
        }
        else
        {
            // _plcVelkys.setRelay(0, false);
            SerialMon.println("[WIFI_CONTROLLER] Closes the relay 1");
        }

        /*-------- RELAY 2 ----------*/
        if (request->hasArg("switch2"))
        {
            // _plcVelkys.setRelay(1, true);
        }
        else
        {
            // _plcVelkys.setRelay(1, false);
        }

        // /*-------- RELAY 3 ----------*/
        // if (request->hasArg("switch3"))
        // {
        //     _plcVelkys.setRelay(2, true);
        // }
        // else
        // {
        //     _plcVelkys.setRelay(2, false);
        // }

        // /*-------- RELAY 4 ----------*/
        // if (request->hasArg("switch4"))
        // {
        //     _plcVelkys.setRelay(3, true);
        // }
        // else
        // {
        //     _plcVelkys.setRelay(3, false);
        // }
        File file = SPIFFS.open(F("/index.html"), "r");
        if (file)
        {
            file.setTimeout(100);
            String s = file.readString();
            file.close();
            /*-------- RELAY 1 ----------*/

            // Actualiza contenido dinamico del html
            s.replace(F("#id#"), id);

            /* Bloque WIFI */
            s.replace(F("#WFEstatus#"), WiFi.status() == WL_CONNECTED ? F("<span class='label label-success'>CONECTADO</span>") : F("<span class='label label-important'>DESCONECTADO</span>"));
            s.replace(F("#WFSSID#"), WiFi.status() == WL_CONNECTED ? F(wifi_ssid) : F("--"));

            s.replace(F("#sysIP#"), ipStr(WiFi.status() == WL_CONNECTED ? WiFi.localIP() : WiFi.softAPIP()));
            s.replace(F("#WFDBM#"), WiFi.status() == WL_CONNECTED ? String(WiFi.RSSI()) : F("0"));
            s.replace(F("#WFPRC#"), WiFi.status() == WL_CONNECTED ? String(round(1.88 * (WiFi.RSSI() + 100)), 0) : F("0"));

            /* Bloque pie chart */
            s.replace(F("#SWFI#"), WiFi.status() == WL_CONNECTED ? String(round(1.88 * (WiFi.RSSI() + 100)), 0) : F("0"));
            s.replace(F("#PMEM#"), String(round(SPIFFS.usedBytes() * 100 / SPIFFS.totalBytes()), 0));
            s.replace(F("#ram#"), String(ESP.getFreeHeap() * 100 / ESP.getHeapSize()));

            // Envia dados al navegador
            request->send(200, "text/html", s);
        }
        else
        {
            request->send(500, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                            "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                            "Swal.fire({title: 'Error!',"
                                            " text: 'Error 500 Internal Server Error',"
                                            " icon: 'error',"
                                            " confirmButtonText: 'Cerrar'}).then((result) => {"
                                            "if (result.isConfirmed){"
                                            "window.location = '/';"
                                            "};"
                                            "})"
                                            "</script><body></html>");
            SerialMon.println(F("\nError: Config - ERROR leyendo el archivo"));
        } });

    // // ---------------------------------------------------------
    // //* CONFIG WIFI PAGE
    // // ---------------------------------------------------------
    _server.on("/configwifi", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
        SerialMon.println("[WIFI_CONTROLLER] Opens the Wifi Configuration page");
        File file = SPIFFS.open(F("/configwifi.html"), "r");
        if (file){
          file.setTimeout(100);
          String s = file.readString();
          file.close();
          // Atualiza el contenido al cargar
          s.replace(F("#id#"), id);
          s.replace(F("#ssid#"), wifi_ssid);
          //sección ap
          s.replace(F("#ap_ssid#"), ap_ssid);
          // Send data
          request->send(200, "text/html", s);
        }
        else{
          request->send(500, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'Error 500 Internal Server Error',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "window.location = '/';"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
          SerialMon.println (F("\nError: Config - ERROR leyendo el archivo"));
        } });

    // // ---------------------------------------------------------
    // //* CONFIG WIFI SAVE
    // // ---------------------------------------------------------
    _server.on("/confwifiSave", HTTP_POST, [this](AsyncWebServerRequest *request)
               {
        SerialMon.println("[WIFI_CONTROLLER] Opens the save wifi config page");
        String response;
        StaticJsonDocument<300> doc;
        // Graba Configuración desde Config
        // Verifica número de campos recebidos
        // ESP32 envia 5 campos
        if (request->params() == 5)
        {
          String s;
          //ID
          if(request->hasArg("id"))
          s = request->arg("id");
          s.trim();
          if (s == ""){
            s = deviceID();
        }
          strlcpy(id, s.c_str(), sizeof(id));

          // SSID
          if(request->hasArg("ssid"))
          s = request->arg("ssid");
          s.trim();
          if (s == ""){
          s = wifi_ssid;
          }
          strlcpy(wifi_ssid, s.c_str(), sizeof(wifi_ssid));
          // PW SSID
          if(request->hasArg("pw"))
          s = request->arg("pw");
          s.trim();
          if (s != ""){
            // Actualiza contraseña
          strlcpy(wifi_passw, s.c_str(), sizeof(wifi_passw));
          }
          // Nombre AP
          if (request->hasArg("ap_ssid"))
          s = request->arg("ap_ssid");
          s.trim();
          if (s != ""){
            // Atualiza ssid ap
          strlcpy(ap_ssid, s.c_str(), sizeof(ap_ssid));
          }
          // Contraseña AP
          if(request->hasArg("passwordap"))
          s = request->arg("passwordap");
          s.trim();
          if (s != ""){
            // Atualiza contraseña ap
            strlcpy(ap_passw, s.c_str(), sizeof(ap_passw));
          }
          // Graba configuracion
          if (saveWifiSettings())
          {
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                            "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Hecho!',"
                                                                " text: 'Configuración WIFI guardada, se requiere reiniciar el Equipo',"
                                                                " icon: 'success',"
                                                                " showCancelButton: true,"
                                                                " confirmButtonColor: '#3085d6',"
                                                                " cancelButtonColor: '#d33',"
                                                                " confirmButtonText: 'Si, reiniciar',"
                                                                " cancelButtonText: 'Cancelar',"
                                                                " reverseButtons: true"
                                                                " }).then((result) => {"
                                                                              "if (result.isConfirmed){"
                                                                                  "window.location = '/reboot';"
                                                                              "}else if ("
                                                                                  "result.dismiss === Swal.DismissReason.cancel"
                                                                                "){"
                                                                                  "history.back();"
                                                                                "}"
                                                                          "})"
                                                  "</script><body></html>");
          }
          else
          {
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'No se pudo guardar, Falló la configuración WIFI',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "history.back();"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
            SerialMon.println(F("\nError: ConfigSave - ERROR salvando Configuración"));
          }
        }
        else
        {
            request->send(200, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                                                  "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                      "Swal.fire({title: 'Error!',"
                                                                " text: 'No se pudo guardar, Error de parámetros WIFI',"
                                                                " icon: 'error',"
                                                                " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                                                                    "if (result.isConfirmed){"
                                                                                                        "history.back();"
                                                                                                    "};"
                                                                                                "})"
                                                  "</script><body></html>");
        } });

    // // ---------------------------------------------------------
    // //* REBOOT
    // // ---------------------------------------------------------
    _server.on("/reboot", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
            SerialMon.println("[WIFI_CONTROLLER] Opens the REBOOT page");
            request->send(200, "text/plain", "Done. ESP will restart");
            delay(2000);
            _wifiMode = WIFI_STA;
            ap_accessPoint = false;
            saveWifiSettings();

            ESP.restart(); });

    // //----------------------------------------------------------
    // //* RESET
    // //----------------------------------------------------------
    _server.on("/reconfig", HTTP_GET, [this](AsyncWebServerRequest *request)
               {
                   Serial.println("[WIFI_CONTROLLER] REBOOT");
                   request->send(200, "text/plain", "Done. ESP will reset the server"); //! RESCTIFICAR THIS SECTION
                   delay(2000);
                   _wifiMode = WIFI_AP;
                   resetWifiSettings(); });

    // // ---------------------------------------------------------
    //* NOT FOUND
    // ---------------------------------------------------------
    _server.onNotFound([this](AsyncWebServerRequest *request)
                       { request->send(404, "text/html", "<html><meta charset='UTF-8'><head><link href='bootstrap.min.css' rel='stylesheet' media='screen'><link rel='stylesheet' href='sweetalert2.min.css'>"
                                                         "<script src='jquery-1.9.1.min.js'><script src='bootstrap.min.js'></script></script><script src='sweetalert2.min.js'></script></head><body><script>"
                                                         "Swal.fire({title: 'Error 404!',"
                                                         " text: 'Página no encontrada',"
                                                         " icon: 'warning',"
                                                         " confirmButtonText: 'Cerrar'}).then((result) => {"
                                                         "if (result.isConfirmed){"
                                                         "history.back();"
                                                         "};"
                                                         "})"
                                                         "</script><body></html>"); });

    _server.begin();
    SerialMon.println("[WIFI_CONTROLLER] Info: HTTP server iniciado");
}

/* -------------------------------------------------------------------------- */
//*                               SPIFF SECTION                               */
/* -------------------------------------------------------------------------- */
/**
 * @brief Initialize the SPIFFS file system.
 * @return {bool} A boolean value indicating whether the SPIFFS file system was successfully mounted.
 */
bool WifiController::initSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("[WIFI_CONTROLLER] SPIFFS el montaje falló!");
        return false;
    }
    else
    {
        Serial.println("[WIFI_CONTROLLER] SPIFFS montado con éxito!");
        return true;
    }
}

/**
 * @brief Read the contents of a file from the SPIFFS file system.
 * This function reads the contents of a file from the SPIFFS file system and returns them as a String. If the file
 * cannot be opened, an error message is printed to the serial monitor and an empty String is returned.
 *
 * @param {const char*} path - The path to the file to be read.
 *
 * @return {String} The contents of the file as a String, or an empty String if the file cannot be opened.
 */
String WifiController::readSPIFFS(const char *path)
{
    File f = SPIFFS.open(path, "r");
    if (!f)
    {
        Serial.println("[WIFI_CONTROLLER] Falla en la apertura de archivo!");
        return "";
    }
    String payload = f.readStringUntil('\n');
    Serial.printf("[WIFI_CONTROLLER] De %s En leer datos %s \n", path, payload);
    return payload;
}

/**
 * @brief Write a message to a file in the SPIFFS file system.
 * @param {const char*} path - The path to the file to be written.
 * @param {const char*} message - The message to be written to the file.
 */
void WifiController::writeSPIFFS(const char *path, const char *message)
{
    File f = SPIFFS.open(path, "w");
    if (!f)
    {
        Serial.println("[WIFI_CONTROLLER] Falla en la apertura de archivo!");
    }
    if (!f.print(message))
    {
        Serial.println("[WIFI_CONTROLLER] No se pudo escribir el mensaje");
    }
    Serial.printf("[WIFI_CONTROLLER] Escribir %s a %s \n", message, path);
}

/* -------------------------------------------------------------------------- */
//*                               WIFI SETTINGS                               */
/* -------------------------------------------------------------------------- */

/**
 * @brief Read the WiFi settings from the "settingsWifi.json" file stored in SPIFFS.
 * @return {bool} A boolean value indicating whether the WiFi settings were successfully read.
 */
bool WifiController::readWifiSettings()
{
    SerialMon.println("WifiController::readWifiSettings ");

    StaticJsonDocument<JSON_CONFIG> jsonConfig;
    File file = SPIFFS.open("/settingsWifi.json", "r");
    if (file)
    {
        if (deserializeJson(jsonConfig, file))
        {
            // Si falla la lectura inicia valores por defecto
            resetWifiSettings();
            SerialMon.println("Error: Falló la lectura de la configuración WiFi, tomando valores por defecto");
            return false;
        }
        else
        {
            /* ------------------- CLIENTE -------------------- */
            strlcpy(id, jsonConfig["id"] | "", sizeof(id));
            strlcpy(wifi_ssid, jsonConfig["ssid"] | "", sizeof(wifi_ssid));
            strlcpy(wifi_passw, jsonConfig["pw"] | "", sizeof(wifi_passw));
            SerialMon.print("WFI SSID : ");
            SerialMon.println(wifi_ssid);
            SerialMon.print("WFI passwd : ");
            SerialMon.println(wifi_passw);

            /* ------------------- AP -------------------- */
            ap_accessPoint = jsonConfig["ap_accessPoint"];
            strlcpy(ap_ssid, jsonConfig["ap_ssid"] | "", sizeof(ap_ssid));
            strlcpy(ap_passw, jsonConfig["passwordap"] | "", sizeof(ap_passw));
            SerialMon.print("AP SSID: ");
            SerialMon.println(ap_ssid);
            SerialMon.print("AP PSSWD: ");
            SerialMon.println(ap_passw);

            SerialMon.println("Info: Lectura configuración WiFi correcta");
            file.close();
            return true;
        }
    }
}

/**
 * @brief Save the WiFi settings to the "settingsWifi.json" file stored in SPIFFS.
 * @return {bool} A boolean value indicating whether the WiFi settings were successfully saved.
 */
bool WifiController::saveWifiSettings()
{
    SerialMon.println("WifiController::saveWifiSettings ");

    // Graba configuración
    StaticJsonDocument<JSON_CONFIG> jsonConfig;
    File file = SPIFFS.open("/settingsWifi.json", "w+");
    if (file)
    {
        /* ------------------- CLIENTE -------------------- */
        jsonConfig["id"] = id;
        jsonConfig["ssid"] = wifi_ssid;
        jsonConfig["pw"] = wifi_passw;
        SerialMon.print("WIfi SSID while WRITE: ");
        SerialMon.println(wifi_ssid);
        SerialMon.print("PSWWD SSID while WRITE: ");
        SerialMon.println(wifi_passw);
        /* ------------------- AP -------------------- */
        jsonConfig["ap_ssid"] = ap_ssid;
        jsonConfig["passwordap"] = ap_passw;
        SerialMon.print("AP SSID while WRITE: ");
        SerialMon.println(ap_ssid);
        SerialMon.print("AP passwd while WRITE: ");
        SerialMon.println(ap_passw);

        jsonConfig["ap_accessPoint"] = ap_accessPoint; //! ESTO ES REALMENTE NECESARIO
        serializeJsonPretty(jsonConfig, file);
        file.close();
        SerialMon.println("\nInfo: Grabando configuración general");
        // serializeJsonPretty(jsonConfig, Serial);
        return true;
    }
    else
    {
        SerialMon.println("\nError: Falló el grabado de la configuración general");
        return false;
    }
}

/**
 * @brief Reset the WiFi settings
 */
void WifiController::resetWifiSettings()
{
    SerialMon.println("WifiController::resetWifiSettings ");

    Serial.println("[WIFI_CONTROLLER] RESET WIFI SETTINGS");

    /*----------------- GENERAL -------------------------*/
    strlcpy(id, "adminesp32", sizeof(id));
    // bootCount = 0;
    /*----------------- CLIENTE -------------------------*/
    // wifi_staticIP = false;
    strlcpy(wifi_ssid, "", sizeof(wifi_ssid));
    strlcpy(wifi_passw, "", sizeof(wifi_passw));
    // strlcpy(wifi_ip_static, "192.168.0.150", sizeof(wifi_ip_static));
    // strlcpy(wifi_gateway, "192.168.0.1", sizeof(wifi_gateway));
    // strlcpy(wifi_subnet, "255.255.255.0", sizeof(wifi_subnet));
    // strlcpy(wifi_primaryDNS, "8.8.8.8", sizeof(wifi_primaryDNS));
    // strlcpy(wifi_secondaryDNS, "8.8.4.4", sizeof(wifi_secondaryDNS));
    /*-------------------- AP ---------------------------*/
    ap_accessPoint = true;
    strlcpy(ap_ssid, "ChargeStation", sizeof(ap_ssid));
    strlcpy(ap_passw, "12345678", sizeof(ap_passw));
    // ap_canalap = 9;
    // ap_hiddenap = false;
    // ap_connetap = 4;
}

/* -------------------------------------------------------------------------- */
//*                             WEBSOCKET SECTION                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize WebSocket connection
 *
 */
void WifiController::InitWebSockets()
{
    _ws.onEvent(onWsEvent);
    _server.addHandler(&_ws);
    Serial.println("WebSocket server started");
}

/**
 * @brief Processes a request received from an AsyncWebSocketClient.
 *
 * This function parses the request JSON string and performs the corresponding action based on the
 * "command" field of the JSON object. The supported commands are "setOnOff", "setPWM", and "doAction".
 * @param client A pointer to the AsyncWebSocketClient that sent the request.
 * @param request The JSON string containing the request data.
 */
void WifiController::processRequest(AsyncWebSocketClient *client, String request)
{
    // PlcVelkys *_plcVelkys = static_cast<PlcVelkys *>(NULL);
    Serial.println("Dentro de ProcessRequest ");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, request);
    if (error)
        return;

    String command = doc["command"];

    if (command == "setOnOff")
    {
        // setOnOff(doc["id"], (bool)doc["status"], _plcVelkys);
    }
    else if (command == "setPWM")
    {
        // setPWM(doc["id"], (int)doc["pwm"], _plcVelkys);
    }
}

void WifiController::updateGPIOData(String input, float value)
{
    String response;
    StaticJsonDocument<300> doc;
    doc["command"] = "updateGPIO";
    doc["id"] = input;

    if (input == "led-1" || input == "led-2")
    {
        doc["status"] = value ? String("ON") : String("OFF");
        Serial.print(input);
        Serial.println(value ? String(" ON") : String(" OFF"));
    }
    else
    {
        doc["status"] = value;
        Serial.print(input);
        Serial.print(" ");
        Serial.println(value);
    }
    serializeJson(doc, response);

    _ws.textAll(response);

    Serial.print(input);
    Serial.println(value ? String(" ON") : String(" OFF"));
}

/**
 * @brief Updates the GPIO data and sends an update message to all WebSocket clients.
 *
 * This function creates a JSON object with the "command", "id", and "status" fields set to "updateGPIO",
 * the input parameter, and the value parameter, respectively. The "status" field is set to "ON" if the
 * value parameter is true, and "OFF" otherwise. The JSON object is then serialized into a JSON string
 * and sent to all WebSocket clients using the textAll() function of the AsyncWebSocket class.
 *
 * Finally, the function prints a message to the serial console with the input and value parameters.
 *
 * @param input The ID of the GPIO pin to update.
 * @param value The new value of the GPIO pin.
 */
void WifiController::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        // Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        // Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        // Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    }
    else if (type == WS_EVT_PONG)
    {
        // Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len)
        {
            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < info->len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }

            if (info->opcode == WS_TEXT)
                processRequest(client, msg);
        }
        else
        {
            // message is comprised of multiple frames or the frame is split into multiple packets
            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());

            if ((info->index + len) == info->len)
            {
                if (info->final)
                {
                    if (info->message_opcode == WS_TEXT)
                        processRequest(client, msg);
                }
            }
        }
    }
}
/* -------------------------------------------------------------------------- */
/*                                API FUNCTIONS                               */
/* -------------------------------------------------------------------------- */

// void WifiController::doAction(String actionId, PlcVelkys *_plcVelkys)
// {
//     Serial.print("Doing action: ");
//     Serial.println(actionId);
// }

// void WifiController::setPWM(String id, int pwm, PlcVelkys *_plcVelkys)
// {
//     Serial.print("Set PWM ");
//     Serial.print(id);
//     Serial.print(": ");
//     Serial.println(pwm);
//     // TODO: Implementar esta parte del codigo tener en cuenta que desde la web me llegan valores float
//     if (id == "slider-pwm-1")
//     {
//     }
//     else if (id == "slider-pwm-2")
//     {
//         /* code */
//     }
// }

// void WifiController::setOnOff(String id, bool state, PlcVelkys *_plcVelkys)
// {
//     Serial.print("Set GPIO ");
//     Serial.print(id);
//     Serial.print(": ");
//     Serial.println(state);

//     if (id == "switch1")
//     {
//         _plcVelkys->setRelay(1, state);
//     }
//     else if (id == "switch2")
//     {
//         _plcVelkys->setRelay(2, state);
//     }
//     else if (id == "switch3")
//     {
//         _plcVelkys->setRelay(3, state);
//     }
//     else if (id == "switch4")
//     {
//         _plcVelkys->setRelay(4, state);
//     }
//     else if (id == "switch5")
//     {
//         _plcVelkys->setDigOut(0, state);
//     }
//     else if (id == "switch6")
//     {
//         _plcVelkys->setDigOut(1, state);
//     }
//     else if (id == "switch7")
//     {
//         _plcVelkys->setDigOut(2, state);
//     }
// }
/* -------------------------------------------------------------------------- */
//*                          ACCESS POINT FUNCTIONS                           */
/* -------------------------------------------------------------------------- */
/**
 * @brief Set the access point mode.
 * @param {bool} ap - A boolean value indicating whether the device should be set to access point mode.
 */
void WifiController::setAccessPoint(bool ap)
{
    SerialMon.println("WifiController setAccessPoint");
    ap_accessPoint = ap;

    // Lee el documento JSON de SPIFFS
    File file = SPIFFS.open("/settingsWifi.json", "r");
    if (!file)
    {
        Serial.println("Error al abrir el archivo JSON");
        return;
    }

    StaticJsonDocument<JSON_CONFIG> jsonConfig;
    DeserializationError error = deserializeJson(jsonConfig, file);
    if (error)
    {
        Serial.println("Error al analizar el archivo JSON");
        return;
    }
    file.close();

    // Modifica un valor específico en el documento JSON
    jsonConfig["ap_accessPoint"] = ap;

    // Escribe el documento JSON modificado de vuelta en SPIFFS
    file = SPIFFS.open("/settingsWifi.json", "w");
    if (!file)
    {
        Serial.println("Error al abrir el archivo JSON para escritura");
        return;
    }

    serializeJson(jsonConfig, file);
    file.close();
}

void WifiController::initAPModeESP()
{
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
    delay(100);
    // WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.setHostname(deviceID().c_str());
    WiFi.softAP(ap_ssid, ap_passw);
    SerialMon.println("Info: Wifi AP" + deviceID() + "-IP" + ipStr(WiFi.softAPIP()));
    _dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
    _dnsServer.start(DNSSERVER_PORT, "*", apIP);
    _wifiMode = WIFI_AP;
}

/* -------------------------------------------------------------------------- */
/*                               OTHER FUNCTIONS                              */
/* -------------------------------------------------------------------------- */
/**
 * @brief
 * Returns IPAdress in "n.n.n" format from IP to String
 * @param {IPAddress} &ip
 * @return String
 */
String WifiController::ipStr(const IPAddress &ip)
{
    String sFn = "";
    for (byte bFn = 0; bFn < 3; bFn++)
    {
        sFn += String((ip >> (8 * bFn)) & 0xFF) + ".";
    }
    sFn += String(((ip >> 8 * 3)) & 0xFF);
    return sFn;
}

/**
 * @brief Convert HEX to String
 * @param h: {const unsigned long} hexadecimal value
 * @param L: {byte}
 * @return String
 */
String WifiController::hexStr(const unsigned long &h, const byte &L)
{
    String s;
    s = String(h, HEX);
    s.toUpperCase();
    s = ("00000000" + s).substring(s.length() + 8 - L);
    return s;
}

/**
 * @brief Create a Unique ID from MAC address
 *
 * @return String: ultimos 4 Bytes del MAC rotados
 */
String WifiController::idUnique()
{
    char idunique[15];
    uint64_t chipid = ESP.getEfuseMac();
    uint16_t chip = (uint16_t)(chipid >> 32);
    snprintf(idunique, 15, "%04X", chip);
    return idunique;
}

/**
 * @brief Returns deviceID in "n.n.n" format from deviceID to String
 * @return String
 */
String WifiController::deviceID()
{
    return String(hexStr(ESP.getEfuseMac()) + String(idUnique()));
}

// void WifiController::_dnsProcess(void *args)
// {
//     vTaskDelay(4000 / portTICK_PERIOD_MS);
//     WifiController *obj = static_cast<WifiController *>(args);
//     SerialMon.println("[WIFI_CONTROLLER] DNS PROCESS TASK RUNNING ... ");

//     for (;;)
//     {
//         if (obj->_wifiMode == WIFI_AP)
//         {
//             obj->_dnsServer.processNextRequest(); // Captive portal DNS re-dierct
//             SerialMon.println("[WIFI_CONTROLLER] Process Request ");
//             delay(100);
//         }
//     }
// }

/* -------------------------------------------------------------------------- */
//*                       TASK section                                        */
/* -------------------------------------------------------------------------- */
// void WifiController::updateGpioDataTask(void *args)
// {
//     vTaskDelay(4000 / portTICK_PERIOD_MS);

//     WifiController *obj = static_cast<WifiController *>(args);
//     // time markers
//     uint64_t tms = millis();
//     uint64_t time_snap = tms;
//     // uint64_t send_log_time = tms;
//     uint64_t retry_reconnect_time = tms;
//     uint64_t check_time = tms;

//     for (;;)
//     {
//         if (obj->ap_accessPoint)
//         {
//             if (millis() - check_time >= 1000) // 3 seconds
//             {
//                 check_time = millis();
//                 SerialMon.println("Update the gpio values in the web");
//                 // obj->updateGPIOData("led-1", obj->_plcVelkys.getDigIn(1));
//                 // obj->updateGPIOData("led-2", obj->_plcVelkys.getDigIn(2));
//                 // obj->updateGPIOData("value-1", obj->_plcVelkys.getAnIn(0) * 3.3 / 4095);
//                 // obj->updateGPIOData("value-2", obj->_plcVelkys.getAnIn(1) * 3.3 / 4095);
//                 // obj->updateGPIOData("value-3", obj->_plcVelkys.getAnIn(2) * 3.3 / 4095);
//                 // obj->updateGPIOData("value-4", obj->_plcVelkys.getAnIn(3) * 3.3 / 4095);
//             }
//         }
//     }
// }
