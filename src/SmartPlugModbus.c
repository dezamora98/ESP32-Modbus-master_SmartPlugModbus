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

    for (uint8_t i = 0; i < 6; ++i)
    {
        slave->Plugs[i].State = &slave->InputReg.PlugState[i];
    }
    return mbc_master_set_descriptor(slave->mb_descriptor, 3);
}

esp_err_t SmartPlugModbus_GetAll(SmartPlugModbus_t *slave)
{
    esp_err_t err = ESP_OK;
    uint8_t type = 0;

    err = mbc_master_get_parameter(slave->Coil_descriptor.cid, (char *)(slave->Coil_descriptor.param_key), &(slave->Coil.Array), &type);
    if (err == ESP_OK)
    {
        for (uint8_t i = ADDR_Plug_0; i <= ADDR_Plug_5; ++i)
        {
            ESP_LOGE("MODBUS", "SPM-%d -> Plug_0 = %d", slave->ID, (((slave->Coil.Array & ~(1 << i)) != 0) ? 1 : 0));
        }
        ESP_LOGE("MODBUS", "SPM-%d -> Reset = %d", slave->ID, slave->Coil.Reset);
    }

    type = 0;
    err = mbc_master_get_parameter(slave->Holding_descriptor.cid, (char *)(slave->Holding_descriptor.param_key), (uint8_t *)(slave->HoldingReg.Array), &type);
    if (err == ESP_OK)
    {
        for (uint8_t i = 0; i != SIZE_HoldingReg; ++i)
        {

            ESP_LOGE("MODBUS", "SPM-%d -> HOLING_%d = %d", slave->ID, i, slave->HoldingReg.Array[i]);
        }
    }

    type = 0;
    err = mbc_master_get_parameter(slave->Input_descriptor.cid, (char *)(slave->Input_descriptor.param_key), (uint8_t *)(slave->InputReg.Array), &type);
    if (err == ESP_OK)
    {
        for (uint8_t i = 0; i != SIZE_InputReg; ++i)
        {
            ESP_LOGE("MODBUS", "SPM-%d -> INPUT_%d = %d", slave->ID, i, slave->InputReg.Array[i]);
        }
    }
    return err;
}

esp_err_t SmartPlugModbus_update(SmartPlugModbus_t *slave)
{
    esp_err_t err = ESP_OK;
    uint8_t type = 0;
    SmartPlugModbus_t temp_slave = *slave;

    ESP_LOGE("MODBUS", "=========================GET_ALL_SLAVE=========================");
    err = SmartPlugModbus_GetAll(&temp_slave);
    ESP_LOGE("MODBUS", "=======================END_GET_ALL_SLAVE=======================");

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
        vTaskDelay(pdMS_TO_TICKS(100));
        err = SmartPlugModbus_GetAll(slave);
        return err;
    }

    if (slave->Coil.Array != temp_slave.Coil.Array)
    {
        ESP_LOGE("MODBUS", "=========================SET_COIL_SLAVE=========================");
        err = mbc_master_set_parameter(slave->Coil_descriptor.cid, (char *)slave->Coil_descriptor.param_key, &slave->Coil.Array, &type);
        ESP_LOGE("MODBUS", "=======================END_SET_COIL_SLAVE=======================");
    }

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
                SmartPlugModbus_update(&(SPM_A_ptr->SPM[i]));
                for (uint j = 0; j < 6; ++j)
                {
                    if (SmartPlugModbus_get_PlugState(SPM_A_ptr->SPM[i], j) == st_On)
                    {
                        SPM_A_ptr->SPM[i].Plugs[j].Tic = esp_rtc_get_time_us();
                        SPM_A_ptr->SPM[i].Plugs[j].Power += SmartPlugModbus_get_PlugPower(SPM_A_ptr->SPM[i], j, Voltage);

                        if (SPM_A_ptr->SPM[i].Plugs[j].Power >= SPM_A_ptr->SPM[i].Plugs[j].PowerLimit ||
                            SPM_A_ptr->SPM[i].Plugs[j].Tic >= SPM_A_ptr->SPM[i].Plugs[j].TimeOut)
                        {
                            SPM_A_ptr->SPM[i].Coil.Array &= ~(1 << j);
                        }
                    }
                }
            }

            xSemaphoreGive(SPM_A_ptr->SPM->sem);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void SmartPlugModbus_PlugOn(SmartPlugModbus_t *slave, uint8_t addr, float power, uint64_t timeout)
{
    if (xSemaphoreTake(slave->sem, portMAX_DELAY) == pdTRUE)
    {
        slave->Plugs[addr].Power = 0;
        slave->Plugs[addr].PowerLimit = power;
        slave->Plugs[addr].Tic = esp_rtc_get_time_us();
        slave->Plugs[addr].TimeOut = esp_rtc_get_time_us() + (timeout * 1000);
        slave->Coil.Array |= (1 << addr);
        xSemaphoreGive(slave->sem);
    }
}

void SmartPlugModbus_PlugOff(SmartPlugModbus_t *slave, uint8_t addr)
{
    if (xSemaphoreTake(slave->sem, portMAX_DELAY) == pdTRUE)
    {
        slave->Coil.Array &= ~(1 << addr);
        xSemaphoreGive(slave->sem);
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
