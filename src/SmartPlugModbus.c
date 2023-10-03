#include "SmartPlugModbus.h"
#include <freertos/semphr.h>

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
    while (true)
    {
        if (xSemaphoreTake(((SmartPlugModbus_Array_t *)SMP_ARRAY)->SPM->sem, portMAX_DELAY) == pdTRUE)
        {
            for (uint8_t i = 0; i < ((SmartPlugModbus_Array_t *)SMP_ARRAY)->size; ++i)
            {

                SmartPlugModbus_update(&(((SmartPlugModbus_Array_t *)SMP_ARRAY)->SPM[i]));
            }
            xSemaphoreGive(((SmartPlugModbus_Array_t *)SMP_ARRAY)->SPM->sem);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
