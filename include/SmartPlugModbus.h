#ifndef SMART_PLUG_MODBUS_H

#include "mbcontroller.h"
#include "ModbusStruct.h"
#include <esp32/rtc.h>
#include <freertos/semphr.h>

// Note: Some pins on target chip cannot be assigned for UART communication.
// See UART documentation for selected board and target to configure pins using Kconfig.

// The number of parameters that intended to be used in the particular control process
#define MASTER_MAX_CIDS num_device_parameters

// Number of reading of parameters from slave
#define MASTER_MAX_RETRY 30

// Timeout to update cid over Modbus
#define UPDATE_CIDS_TIMEOUT_MS (500)
#define UPDATE_CIDS_TIMEOUT_TICS (UPDATE_CIDS_TIMEOUT_MS / portTICK_PERIOD_MS)

// Timeout between polls
#define POLL_TIMEOUT_MS (1)
#define POLL_TIMEOUT_TICS (POLL_TIMEOUT_MS / portTICK_PERIOD_MS)

// The macro to get offset for parameter in the appropriate structure
#define HOLD_OFFSET(field) ((uint16_t)(offsetof(HoldingReg_t, field) + 1))
#define INPUT_OFFSET(field) ((uint16_t)(offsetof(InputReg_t, field) + 1))
#define COIL_OFFSET(field) ((uint16_t)(offsetof(Coil_t, field) + 1))

#define STR(fieldname) ((const char *)(fieldname))

// Options can be used as bit masks or parameter limits
#define OPTS(min_val, max_val, step_val)                   \
    mb_parameter_opt_t                                     \
    {                                                      \
        .opt1 = min_val, .opt2 = max_val, .opt3 = step_val \
    }

static const char *TAG = "MASTER_TEST";

// Enumeration of modbus device addresses accessed by master device
enum
{
    ADDR_SmartPlugModbus_0 = 1 // Only one slave device used for the test (add other slave addresses here)
};

// Enumeration of all supported CIDs for device (used in parameter definition table)
enum
{
    CID_COILS = 0,
    CID_INPUTS = 3,
    CID_HOLDING_REG = 1,
    CID_INPUT_REG = 2,
};

typedef struct 
{
    uint8_t ID;
    Coil_t Coil;
    HoldingReg_t HoldingReg;
    InputReg_t InputReg;
    SemaphoreHandle_t sem;

    union
    {
       mb_parameter_descriptor_t mb_descriptor[3];
       struct
       {
            mb_parameter_descriptor_t Coil_descriptor;
            mb_parameter_descriptor_t Holding_descriptor;
            mb_parameter_descriptor_t Input_descriptor;
       };
       
    };
    
    
}SmartPlugModbus_t;

typedef struct 
{
    SmartPlugModbus_t* SPM;
    uint8_t size;
}SmartPlugModbus_Array_t;



esp_err_t SmartPlugModbus_init(SmartPlugModbus_t* slave, uint8_t* CID_count, const uint8_t ID);
esp_err_t SmartPlugModbus_GetAll(SmartPlugModbus_t* slave);
esp_err_t SmartPlugModbus_update(SmartPlugModbus_t* slave);
void SmartPlugModbus_Task(void *SMP_ARRAY);
#define PlugON

// Calculate number of parameters in the table

#endif // !SMART_PLUG_MODBUS_H