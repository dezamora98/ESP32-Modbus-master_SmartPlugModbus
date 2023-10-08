#include "SmartPlugModbus.h"
#include <freertos/semphr.h>
#include <esp_system.h>

esp_err_t SmartPlugModbus_init(SmartPlugModbus_t *slave, uint8_t *CID_count, const uint8_t ID)
{
    slave->ID = ID;
    slave->Coil_descriptor = (mb_parameter_descriptor_t){
        .cid = (*CID_count)++,
        .param_key = "COILS",
        .param_units = "ON/OFF",
        .mb_slave_addr = slave->ID,
        .mb_param_type = MB_PARAM_COIL,
        .mb_reg_start = InitAddr_Coil,
        .mb_size = SIZE_Coil,
        .param_offset = InitAddr_Coil,
        .param_type = PARAM_TYPE_U8,
        .param_size = PARAM_SIZE_U8,
        .param_opts =
            {
                .max = 0b01111111,
                .min = 0,
                .step = 0,
            },
        .access = PAR_PERMS_READ_WRITE_TRIGGER,
    };

    slave->Holding_descriptor = (mb_parameter_descriptor_t){
        .cid = (*CID_count)++,
        .param_key = "HOLDING_REG",
        .param_units = "UINT16",
        .mb_slave_addr = slave->ID,
        .mb_param_type = MB_PARAM_HOLDING,
        .mb_reg_start = InitAddr_HoldingReg,
        .mb_size = SIZE_HoldingReg,
        .param_offset = InitAddr_HoldingReg,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts =
            {
                .max = 0xffff,
                .min = 0,
                .step = 0,
            },
        .access = PAR_PERMS_READ_WRITE_TRIGGER,
    };

    slave->Input_descriptor = (mb_parameter_descriptor_t){
        .cid = (*CID_count)++,
        .param_key = "INPUT_REG",
        .param_units = "UINT16",
        .mb_slave_addr = slave->ID,
        .mb_param_type = MB_PARAM_INPUT,
        .mb_reg_start = InitAddr_InputReg,
        .mb_size = SIZE_InputReg,
        .param_offset = InitAddr_InputReg,
        .param_type = PARAM_TYPE_U16,
        .param_size = PARAM_SIZE_U16,
        .param_opts = {
            .max = 0xffff,
            .min = 0,
            .step = 0,
        },
        .access = PAR_PERMS_READ,
    };

    slave->sem = xSemaphoreCreateMutex();

    for (uint8_t i = ADDR_Plug_0; i < SIZE_Plug; ++i)
    {
        slave->Plugs[i].Addr = i;
        slave->Plugs[i].SPM = slave;
    }
    return mbc_master_set_descriptor(slave->mb_descriptor, 3);
}

#define LOG_MOD false
esp_err_t SmartPlugModbus_GetAll(SmartPlugModbus_t *slave)
{
    esp_err_t err = ESP_OK;
    uint8_t type = 0;

    err = mbc_master_get_parameter(slave->Coil_descriptor.cid, (char *)(slave->Coil_descriptor.param_key), &(slave->Coil.Array), &type);
    if (err == ESP_OK)
    {
#if LOG_MOD == true
        for (uint8_t i = ADDR_Plug_0; i <= ADDR_Plug_5; ++i)
        {
            ESP_LOGE("MODBUS", "SPM-%d -> Plug_0 = %d", slave->ID, (((slave->Coil.Array & ~(1 << i)) != 0) ? 1 : 0));
        }
        ESP_LOGE("MODBUS", "SPM-%d -> Reset = %d", slave->ID, slave->Coil.Reset);
#endif
    }

    type = 0;
    err = mbc_master_get_parameter(slave->Holding_descriptor.cid, (char *)(slave->Holding_descriptor.param_key), (uint8_t *)(slave->HoldingReg.Array), &type);
    if (err == ESP_OK)
    {
#if LOG_MOD == true
        for (uint8_t i = 0; i != SIZE_HoldingReg; ++i)
        {

            ESP_LOGE("MODBUS", "SPM-%d -> HOLING_%d = %d", slave->ID, i, slave->HoldingReg.Array[i]);
        }
#endif
    }

    type = 0;
    err = mbc_master_get_parameter(slave->Input_descriptor.cid, (char *)(slave->Input_descriptor.param_key), (uint8_t *)(slave->InputReg.Array), &type);
    if (err == ESP_OK)
    {
#if LOG_MOD == true
        for (uint8_t i = 0; i != SIZE_InputReg; ++i)
        {
            ESP_LOGE("MODBUS", "SPM-%d -> INPUT_%d = %d", slave->ID, i, slave->InputReg.Array[i]);
        }
#endif
    }
    return err;
}

esp_err_t SmartPlugModbus_update(SmartPlugModbus_t *slave)
{
    esp_err_t err = ESP_OK;
    uint8_t type = 0;
    SmartPlugModbus_t temp_slave = *slave;

    ESP_LOGE("MODBUS", " -> GET_ALL_SLAVE:");
    err = SmartPlugModbus_GetAll(&temp_slave);
    ESP_LOGE("MODBUS", " -> END_GET_ALL_SLAVE");

    if ((slave->Coil.Array & 0x3f) != (temp_slave.Coil.Array & 0x3f))
    {
        slave->Coil.Array &= 0x3f;
        ESP_LOGE("MODBUS", " -> SET_COIL_SLAVE:");
        err = mbc_master_set_parameter(slave->Coil_descriptor.cid, (char *)slave->Coil_descriptor.param_key, &slave->Coil.Array, &type);
        if (err != ESP_OK)
        {
            return err;
        }
        ESP_LOGE("MODBUS", " -> END_SET_COIL_SLAVE");
    }

    if (memcmp(&slave->HoldingReg, &temp_slave.HoldingReg, SIZE_HoldingReg) != 0)
    {
        ESP_LOGE("MODBUS", "====================SET_HOLDING_REG_SLAVE=========================");
        err = mbc_master_set_parameter(slave->Holding_descriptor.cid, (char *)slave->Holding_descriptor.param_key, (uint8_t *)slave->HoldingReg.Array, &type);
        if (err != ESP_OK)
        {
            return err;
        }
        ESP_LOGE("MODBUS", "====================END_SET_HOLDING_REG_SLAVE====================");

        ESP_LOGE("MODBUS", "===========================RESET_SLAVE===========================");
        slave->Coil.Array = 0x40; // reset;
        err = mbc_master_set_parameter(slave->Coil_descriptor.cid, (char *)slave->Coil_descriptor.param_key, &slave->Coil.Array, &type);
        if (err != ESP_OK)
        {
            return err;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    err = SmartPlugModbus_GetAll(slave);
    return err;
}

void SmartPlugModbus_Task(void *SMP_ARRAY)
{
#define SPM_A_ptr ((SmartPlugModbus_Array_t *)SMP_ARRAY)
    while (true)
    {
        for (uint8_t i = 0; i < SPM_A_ptr->size; ++i)
        {
            if (xSemaphoreTake(SPM_A_ptr->SPM[i].sem, portMAX_DELAY) == pdTRUE)
            {
                for (uint j = 0; j < 6; ++j)
                {
                    ESP_LOGE("MODBUS", " -> Check_plug_%d", j);

                    if (SPM_A_ptr->SPM[i].InputReg.PlugState[j] == st_On &&
                        (SPM_A_ptr->SPM[i].Plugs[j].Power >= SPM_A_ptr->SPM[i].Plugs[j].PowerLimit))
                    {
                        ESP_LOGE("MODBUS", "PLUG %d OFF -> POWER", j);
                        SPM_A_ptr->SPM[i].Coil.Array &= ~(1 << j);
                    }
                    #if false
                    else if(SPM_A_ptr->SPM[i].InputReg.PlugState[j] > st_On)
                    {
                        ESP_LOGE("MODBUS", "PLUG %d OFF -> ERROR", j);
                        SPM_A_ptr->SPM[i].Coil.Array &= ~(1 << j);
                    }
                    #endif
                }
                SmartPlugModbus_update(&(SPM_A_ptr->SPM[i]));
                xSemaphoreGive(SPM_A_ptr->SPM->sem);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

/**
 * @brief this hidden function is used as a callback to stop the connectors due to timeout
 *
 * @param arg Plug_t*
 */
void _SmartPLugModbus_Plug_TimeOut(void *arg)
{
    Plug_t *plug = (Plug_t *)arg;

    ESP_LOGE("MODBUS", " -> TIMEOUT_PLUG");
    if (xSemaphoreTake(plug->SPM->sem, portMAX_DELAY) == pdTRUE) // sync
    {
        plug->SPM->InputReg.PlugState[plug->Addr] = st_Off; // state off
        esp_timer_stop(plug->_Timer_handle);                // stop timer
        esp_timer_delete(plug->_Timer_handle);              // delete the timer handler
        plug->SPM->Coil.Array &= ~(1 << plug->Addr);        // disconnect plug

        xSemaphoreGive(plug->SPM->sem);                     // end sync

        ESP_LOGE("MODBUS", " -> TIMEOUT_PLUG_EXECUTE");
    }
}

/**
 * @brief this function is used to set the power-on status of a plug together with its maximum time and power paramet
 * 
 * @param slave SmartPlugModbus slave ptr
 * @param addr Plug address
 * @param power Power limit for plug 
 * @param timeout Timeout for plug 
 */
void SmartPlugModbus_PlugOn(SmartPlugModbus_t *slave, uint8_t addr, float power, uint64_t timeout)
{
    Plug_t *plug = &slave->Plugs[addr];

    ESP_LOGE("MODBUS", " -> plug %d on for %ds ", addr, (int)timeout);
    if (xSemaphoreTake(slave->sem, portMAX_DELAY) == pdTRUE)                   // sync
    {

        slave->Plugs[addr]._Timer_args = (esp_timer_create_args_t){
            .callback = &_SmartPLugModbus_Plug_TimeOut,
            .arg = (void *)(plug),
        };                                                                     // set arguments for plug timeout
        esp_timer_create(&plug->_Timer_args, &plug->_Timer_handle);            // create timer for plug
        esp_timer_start_once(plug->_Timer_handle, timeout * 1000000);          // start timer

        slave->Plugs[addr].Power = 0;                                          // reset power counter
        slave->Plugs[addr].PowerLimit = power;                                 // set limit power
        slave->Coil.Array |= (1 << addr);                                      // connect plug 

        xSemaphoreGive(slave->sem);                                            // end sync
    }
}

void SmartPlugModbus_PlugOff(SmartPlugModbus_t *slave, uint8_t addr)
{
    if (xSemaphoreTake(slave->sem, portMAX_DELAY) == pdTRUE) // sync
    {
        slave->InputReg.PlugState[addr] = st_Off;   // set off state in plug
        slave->Coil.Array &= ~(1 << addr);          // disconnect plug
        xSemaphoreGive(slave->sem);                 // end sync
    }
}

void SmartPlugModbus_set_Holding(SmartPlugModbus_t *slave, uint8_t addr, uint16_t val)
{
    if (xSemaphoreTake(slave->sem, portMAX_DELAY) == pdTRUE)
    {
        slave->HoldingReg.Array[addr] = val;
        xSemaphoreGive(slave->sem);
    }
}