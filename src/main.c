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
    
    void *modbus_h = NULL;
    uint8_t CID_count = 0;
    mb_communication_info_t mb_info =
        {
            .baudrate = 38400,
            .mode = MB_MODE_RTU,
            .parity = UART_PARITY_DISABLE,
            .port = UART_NUM_2,
        };
    SmartPlugModbus_t slave_0;

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, 0, 0));
    ESP_ERROR_CHECK(mbc_master_init(MB_PORT_SERIAL_MASTER, &modbus_h));
    ESP_ERROR_CHECK(mbc_master_setup((void *)&mb_info));
    ESP_LOGE(TAG, "Master configurado");
    ESP_ERROR_CHECK(SmartPlugModbus_init(&slave_0,&CID_count,1));
    ESP_LOGE(TAG, "Descriptor de test iniciado");
    ESP_ERROR_CHECK(mbc_master_start());

    mb_parameter_descriptor_t *temp_descr = NULL;
    uint8_t temp_data[20] = {0};
    uint8_t temp_type = 0;

    while (true)
    {
        SmartPlugModbus_update(&slave_0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}