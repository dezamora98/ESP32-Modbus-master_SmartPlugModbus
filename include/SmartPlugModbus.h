#ifndef SMART_PLUG_MODBUS_H

#include "mbcontroller.h"
#include "ModbusStruct.h"

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
#define HOLD_OFFSET(field) ((uint16_t)(offsetof(t_HoldingReg, field) + 1))
#define INPUT_OFFSET(field) ((uint16_t)(offsetof(t_InputReg, field) + 1))
#define COIL_OFFSET(field) ((uint16_t)(offsetof(t_Coil, field) + 1))

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
    CID_Plug_0 = 0,
    CID_Plug_1,
    CID_Plug_2,
    CID_Plug_3,
    CID_Plug_4,
    CID_Plug_5,
    CID_Reset,

    CID_Param_PlugOverCurrent,
    CID_Param_PlugLowCurrent,
    CID_Param_OverVoltage,
    CID_Param_LowVoltage,

    CID_Param_HighTemperature,
    CID_Param_SystemOverCurrent,
    CID_Param_SlaveID,

    CID_Param_TimeoutPlugLowCurrent,
    CID_Param_TimeoutPlugOverCurrent,
    CID_Param_TimeoutLowVoltage,
    CID_Param_TimeoutOverVoltage,
    CID_Param_TimeoutHighTemperature,

    CID_Reg_SystemState,
    CID_Reg_PlugState_0,
    CID_Reg_PlugState_1,
    CID_Reg_PlugState_2,
    CID_Reg_PlugState_3,
    CID_Reg_PlugState_4,
    CID_Reg_PlugState_5,

    // analog registers
    CID_Reg_PlugCurrent_0,
    CID_Reg_PlugCurrent_1,
    CID_Reg_PlugCurrent_2,
    CID_Reg_PlugCurrent_3,
    CID_Reg_PlugCurrent_4,
    CID_Reg_PlugCurrent_5,
    CID_Reg_Voltage,
    CID_Reg_BoardCurrent,
    CID_Reg_TempMCU,
};

// { CID, Param Name, Units, Modbus Slave Addr, Modbus Reg Type, Reg Start, Reg Size, Instance Offset, Data Type, Data Size, Parameter Options, Access Mode}
const mb_parameter_descriptor_t device_parameters[] = {
    {.cid = CID_Param_HighTemperature,
     .param_key = STR("HOLDING"),
     .param_units = STR("N"),
     .mb_slave_addr = ADDR_SmartPlugModbus_0,
     .mb_param_type = MB_PARAM_HOLDING,
     .mb_reg_start = InitAddr_HoldingReg,
     .mb_size = SIZE_HoldingReg,
     .param_offset = InitAddr_HoldingReg,
     .param_type = PARAM_TYPE_U16,
     .param_opts = {.min = 0, .max = 0x3ff, .step = 1},
     .access = PAR_PERMS_READ_WRITE_TRIGGER}};

// Calculate number of parameters in the table
const uint16_t num_device_parameters = (sizeof(device_parameters) / sizeof(device_parameters[0]));

#endif // !SMART_PLUG_MODBUS_H