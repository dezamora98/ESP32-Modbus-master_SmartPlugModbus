/*
 * SPDX-FileCopyrightText: 2016-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/uart.h>
#include "SmartPlugModbus.h"

void app_main(void)
{
    uart_set_pin(UART_NUM_2, 17, 16, NULL, NULL);
    mb_communication_info_t mb_info =
    {
        .baudrate = 38400,
        .mode = MB_MODE_RTU,
        .parity = UART_PARITY_DISABLE,
        .port = UART_NUM_2
    };

    mb_parameter_descriptor_t mb_descr[] =
    {
        {
            .cid = 0,
            .mb_slave_addr = 2,
            .mb_param_type = MB_PARAM_HOLDING,
            .mb_size = 1,
            .access = PAR_PERMS_READ_WRITE_TRIGGER,
            .param_key = "test",
            .param_offset = 0,
            .param_size = PARAM_SIZE_U16,
            .param_units = "H",
            .param_type = PARAM_TYPE_U16,
            .param_opts =
            {
                .max = 0xffff,
                .min = 0x0000,
                .step = 1
            }
        }
    };

    void *modbus_h = NULL;

    ESP_ERROR_CHECK(mbc_master_init(MB_PORT_SERIAL_MASTER, &modbus_h));
    ESP_ERROR_CHECK(mbc_master_setup((void *)&mb_info));
    ESP_LOGE(TAG, "Master configurado");
    ESP_ERROR_CHECK(mbc_master_set_descriptor(&mb_descr[0], sizeof(mb_descr) / sizeof(mb_descr[0])));
    ESP_LOGE(TAG, "Descriptor de test iniciado");
    ESP_ERROR_CHECK(mbc_master_start());

    mb_parameter_descriptor_t* temp_descr = NULL;
    uint8_t temp_data[20] = {0};
    uint8_t temp_type=0;

    while (true)
    {
        mbc_master_get_cid_info(mb_descr[0].cid,&temp_descr);

        mbc_master_get_parameter(temp_descr->cid,temp_descr->param_key, (uint8_t*)temp_data, &temp_type);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}