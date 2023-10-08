#ifndef SMART_PLUG_MODBUS_H

#include "mbcontroller.h"
#include "ModbusStruct.h"
#include <esp32/rtc.h>
#include <esp_timer.h>
#include <freertos/semphr.h>

#define Voltage 110

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

// Options can be used as bit masks or parameter limits
#define OPTS(min_val, max_val, step_val)                   \
    mb_parameter_opt_t                                     \
    {                                                      \
        .opt1 = min_val, .opt2 = max_val, .opt3 = step_val \
    }

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
    uint8_t Addr;
    float Power;
    float PowerLimit;
    struct SmartPlugModbus_t *SPM;
    esp_timer_create_args_t _Timer_args;
    esp_timer_handle_t _Timer_handle;
} Plug_t;

#pragma pack(push, 1)
typedef struct SmartPlugModbus_t
{
    uint8_t ID;
    Coil_t Coil;
    HoldingReg_t HoldingReg;
    InputReg_t InputReg;
    SemaphoreHandle_t sem;
    Plug_t Plugs[6];

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

} SmartPlugModbus_t;
#pragma pack(pop)

typedef struct
{
    SmartPlugModbus_t *SPM;
    uint8_t size;
} SmartPlugModbus_Array_t;

esp_err_t SmartPlugModbus_init(SmartPlugModbus_t *slave, uint8_t *CID_count, const uint8_t ID);
esp_err_t SmartPlugModbus_GetAll(SmartPlugModbus_t *slave);
esp_err_t SmartPlugModbus_update(SmartPlugModbus_t *slave);
void SmartPlugModbus_Task(void *SMP_ARRAY);
#define SmartPlugModbus_get_PlugState(slave, addr) slave.InputReg.PlugState[addr]
#define SmartPlugModbus_get_PlugPower(slave, addr, voltage) slave.InputReg.PlugCurrent[addr] * voltage
void SmartPlugModbus_PlugOn(SmartPlugModbus_t *slave, uint8_t addr, float power, uint64_t timeout);
void SmartPlugModbus_PlugOff(SmartPlugModbus_t *slave, uint8_t addr);
void SmartPlugModbus_set_Holding(SmartPlugModbus_t *slave, uint8_t addr, uint16_t val);

// Calculate number of parameters in the table

#endif // !SMART_PLUG_MODBUS_H