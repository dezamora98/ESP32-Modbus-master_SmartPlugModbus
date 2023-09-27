#include "SmartPlugModbus.h"

esp_err_t SmartPlugModbus_init(SmartPlugModbus_t *slave, uint8_t *CID_count, const uint8_t ID)
{
    slave->ID = ID;
    slave->mb_descriptor[0] = (mb_parameter_descriptor_t){
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

    slave->mb_descriptor[1] = (mb_parameter_descriptor_t){
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

    slave->mb_descriptor[2] = (mb_parameter_descriptor_t){
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

    return mbc_master_set_descriptor(slave->mb_descriptor, 3);
}

esp_err_t SmartPlugModbus_update(SmartPlugModbus_t *slave)
{
    esp_err_t err = ESP_OK;
    uint8_t type = 0;
    err = mbc_master_get_parameter(slave->mb_descriptor[0].cid, slave->mb_descriptor[0].param_key, &(slave->Coil.Array), &type);
    if (err == ESP_OK)
    {
        ESP_LOGE("MODBUS", "SPM-%d -> Plug_0 = %d", slave->ID, slave->Coil.Plug_0);
        ESP_LOGE("MODBUS", "SPM-%d -> Plug_1 = %d", slave->ID, slave->Coil.Plug_1);
        ESP_LOGE("MODBUS", "SPM-%d -> Plug_2 = %d", slave->ID, slave->Coil.Plug_2);
        ESP_LOGE("MODBUS", "SPM-%d -> Plug_3 = %d", slave->ID, slave->Coil.Plug_3);
        ESP_LOGE("MODBUS", "SPM-%d -> Plug_4 = %d", slave->ID, slave->Coil.Plug_4);
        ESP_LOGE("MODBUS", "SPM-%d -> Plug_5 = %d", slave->ID, slave->Coil.Plug_5);
        ESP_LOGE("MODBUS", "SPM-%d -> Reset = %d", slave->ID, slave->Coil.Reset);
    }

    type = 0;
    err = mbc_master_get_parameter(slave->mb_descriptor[1].cid, slave->mb_descriptor[1].param_key, (uint8_t *)(slave->HoldingReg.Array), &type);
    if (err == ESP_OK)
    {
        for (uint8_t i = 0; i != SIZE_HoldingReg; ++i)
        {
            ESP_LOGE("MODBUS", "SPM-%d -> HOLING_%d = %d", slave->ID, i, slave->HoldingReg.Array[i]);
        }
    }

    type = 0;
    err = mbc_master_get_parameter(slave->mb_descriptor[2].cid, slave->mb_descriptor[2].param_key, (uint8_t *)(slave->InputReg.Array), &type);
    if (err == ESP_OK)
    {
        for (uint8_t i = 0; i != SIZE_InputReg; ++i)
        {
            ESP_LOGE("MODBUS", "SPM-%d -> INPUT_%d = %d", slave->ID, i, slave->InputReg.Array[i]);
        }
    }

    return err;
}
