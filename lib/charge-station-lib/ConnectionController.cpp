#include "ConnectionController.h"

#include <config.h>
#include <auxiliars.h>

/*
 * NOTAS DE IMPLEMENTACION
 * - La intencion es crear un controlador centralizado para todas las vias de conexion con el servidor de ThingsBoard.
 * - El controlador debe ser capaz de comprobar el estado de conexion en todo momento y actualizar/notificar dicho estado al usuario.
 * - La comprobacion de estado se debe hacer a nivel de servidor, por la biblioteca de ThingsBoard, y a nivel de red segun la conexion establecida.
 * - Se pretende lograr que al fallar un tipo de conexion, el controlador busque establecer la conexion por otra via de las disponibles. Esto conlleva a que se deban establecer prioridades, que pude ser a partir de la velocidad y ancho de banda disponible por cada tipo de conexion.
 * - Existe un posible caso de uso en el cual deban coexistir dos tipos de conexion de forma simultanea. Seria que durante la descarga de logs mediante una conexion rapida (Wi-Fi), se deseen mandar mensajes SMS de notificacion mediante una conexion GSM. Esto se evitaria de antemano, desacomplando el envio de mensajes directamente del dispositivo y delegando esta tarea a la plataforma. Asi ademas de mensajes se puede incluir un correo o una notificacion a Telegram.
 * - La estrategia inicial fue definir el tipo de conexion en el archivo de config.h con lo cual se fija en tiempo de compilacion la configuracion de la conexion. Se intentara hacerlo dinamico entre WiFi y GSM para comprobar el funcionamiento.
 * - Se buscara implementar la clase ConnectionController utilizando el patron Singleton para garantizar una unica instancia del controlador y mantener las bondades de trabajarlo como un objeto.
 * - La implementacion del ConnectionController se realizara utilizando elementos del FreeRTOS para hacerlo thread safe.
 * */

ConnectionController *ConnectionController::_connection_controller = nullptr;
EventGroupHandle_t ConnectionController::connectionEventsHandler = nullptr;

ConnectionController::ConnectionController() : _connection_state(ConnectionState::DISCONNECTED)
{
    _conn_state_mutex = xSemaphoreCreateMutex();

    _wifi_controller = nullptr;

    if (!connectionEventsHandler)
        connectionEventsHandler = xEventGroupCreate();
}

ConnectionController::~ConnectionController()
{
    delete _tb_handler;
    delete _wifi_controller;
    delete _connection_controller;
}

ConnectionController *ConnectionController::Instance()
{
    if (_connection_controller == nullptr)
    {
        _connection_controller = new ConnectionController;
    }

    return _connection_controller;
}

ThingsBoard *ConnectionController::getThingsBoardHandler()
{
    // assert(_tb_handler != nullptr);
    return _tb_handler;
}

void ConnectionController::platformConnectionInit(ConnectionType conn_type)
{
    _current_connection_type = conn_type;
    Serial.println("DENTRO DE PLATAFORM CONNECTION INITIALIZATION");

    switch (_current_connection_type)
    {
    case ConnectionType::WIFI:
        if (_wifi_controller == nullptr)
        {
            _wifi_controller = new WifiController;
            Serial.println("Creando new WifiController");
        }

        if (_wifi_controller != nullptr)
        {
            if (!_wifi_controller->initWifi())
            {
                PRINT_LOCK
                printlnA("Wifi Not Initialized");
                PRINT_UNLOCK
            }
            _client = _wifi_controller->_wifiClient.getClient();
        }
        break;
    // case ConnectionType::SIM7000G:
    //     if (_sim_controller == nullptr)
    //     {
    //         _sim_controller = new SimController;
    //         if (!_sim_controller->init())
    //         {
    //             PRINT_LOCK
    //             printlnA("SIM Not Initiliazed");
    //             PRINT_UNLOCK
    //         }
    //     }
    //     _client = _sim_controller->getClient();
    //     break;
    // case ConnectionType::SIM800L:

    //     break;
    // case ConnectionType::ETHERNET:

    //     break;
    // case ConnectionType::LORA:

    //     break;
    default:
        // wifiControllerInit();
        PRINT_LOCK
        printlnE("UNDEFINED CONNECTION TYPE");
        PRINT_UNLOCK
        break;
    }

    // delay(2000);

    if (_client != nullptr)
    {
        //! Esto da problemas... ver por qué luego
        // if (_tb_handler != nullptr)
        //     delete _tb_handler;

        _tb_handler = new ThingsBoard(*_client);

        PRINT_LOCK
        printlnA("Creating ConnectionController _platformKeepConnectionAlive Task");
        PRINT_UNLOCK

        // TODO: add task handler to check if is null and prevent from repeat the task creation
        if (_tb_handler != nullptr)
        {
            xTaskCreatePinnedToCore(
                _platformKeepConnectionAlive,
                "KeepPlatfConn",
                2000,
                this,
                1,
                NULL,
                CONFIG_ARDUINO_RUNNING_CORE);
        }
    }

    // delay(5000);
}

void ConnectionController::_platformKeepConnectionAlive(void *args)
{
    bool network_connected = false;

    ConnectionController *obj = static_cast<ConnectionController *>(args);

    for (;;)
    {
        // Reconnect to ThingsBoard, if needed
        if (obj->_tb_handler != nullptr)
        {
            switch (obj->getCurrentConnectionType())
            {
            case ConnectionType::WIFI:
                network_connected = obj->_wifi_controller->_wifiClient.isConnected();
                break;
            // case ConnectionType::SIM7000G:
            //     network_connected = obj->_sim_controller->isConnected();
            //     break;
            default:
                network_connected = false;
                break;
            }

            if (!network_connected)
            {
                obj->_setConnectionState(ConnectionState::DISCONNECTED);
                PRINT_LOCK
                printlnA("Network disconnected...");
                PRINT_UNLOCK
                delay(2000);
                continue;
            }

            if (!obj->_tb_handler->connected())
            {
                obj->_setConnectionState(ConnectionState::DISCONNECTED);

                if (obj->_tb_handler->connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
                {
                    PRINT_LOCK
                    printlnA("Connected to ThignsBoard");
                    PRINT_UNLOCK
                    obj->_setConnectionState(ConnectionState::CONNECTED);
                }
                else
                {
                    PRINT_LOCK
                    printlnA("Failed to connect to ThignsBoard");
                    PRINT_UNLOCK

                    delay(2000);
                }
            }

            obj->_tb_handler->loop();
        }
        else
        {
            PRINT_LOCK
            printlnA("ThingsBoard handler no created...");
            PRINT_UNLOCK
            delay(5000);
        }

        delay(500);
        // PRINT_LOCK
        // printA("Keep Platform Connection Task's Stack Bytes Availables: ");
        // UBaseType_t aux = uxTaskGetStackHighWaterMark(nullptr);
        // printlnA((unsigned)aux);
        // PRINT_UNLOCK
    }
}

bool ConnectionController::isPlatformConnected()
{
    if (_tb_handler != nullptr)
    {
        _tb_handler->loop();
        return _tb_handler->connected();
    }
    return false;
}

// bool ConnectionController::fastConnectionAvailable()
// {
//     switch (_current_connection_type)
//     {
//     // case ConnectionType::SIM7000G:
//     //     // TODO: chequear modo (GSM, LTE)
//     //     return true;
//     // case ConnectionType::ETHERNET:
//     case ConnectionType::WIFI:
//         return true;
//     default:
//         PRINT_LOCK
//         printlnE("UNDEFINED CONNECTION TYPE");
//         PRINT_UNLOCK
//         break;
//     }

//     return false;
// }

ConnectionState ConnectionController::getConnectionState()
{
    ConnectionState con_state;

    BaseType_t rc = xSemaphoreTake(_conn_state_mutex, portMAX_DELAY);
    assert(rc == pdPASS);

    con_state = _connection_state;

    rc = xSemaphoreGive(_conn_state_mutex);
    assert(rc == pdPASS);

    return con_state;
}

void ConnectionController::_setConnectionState(ConnectionState con_state)
{
    BaseType_t rc;

    rc = xSemaphoreTake(_conn_state_mutex, portMAX_DELAY);
    assert(rc == pdPASS);
    if (con_state != this->_connection_state)
    {
        this->_connection_state = con_state;
        if (con_state == ConnectionState::CONNECTED)
        {
            // xEventGroupClearBits(connectionEventsHandler, (EventBits_t)ConnectionEvents::EVENT_DISCONNECTED);
            xEventGroupSetBits(connectionEventsHandler, (EventBits_t)ConnectionEvents::EVENT_CONNECTED);
        }
        else if (con_state == ConnectionState::DISCONNECTED)
        {
            // xEventGroupClearBits(connectionEventsHandler, (EventBits_t)ConnectionEvents::EVENT_CONNECTED);
            xEventGroupSetBits(connectionEventsHandler, (EventBits_t)ConnectionEvents::EVENT_DISCONNECTED);
        }
    }
    rc = xSemaphoreGive(_conn_state_mutex);
    assert(rc == pdPASS);
}
