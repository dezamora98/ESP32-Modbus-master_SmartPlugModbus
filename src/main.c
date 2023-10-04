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
    TaskHandle_t SPM_task;
    uint8_t CID_count = 0;

    mb_communication_info_t mb_info =
        {
            .baudrate = 38400,
            .mode = MB_MODE_RTU,
            .parity = UART_PARITY_DISABLE,
            .port = UART_NUM_2,

        };
    SmartPlugModbus_t slave_0 = {0};
    SmartPlugModbus_Array_t SPM_ARRAY =
        {
            .SPM = &slave_0,
            .size = 1,
        };

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 22, 23, 0, 0));
    ESP_ERROR_CHECK(mbc_master_init(MB_PORT_SERIAL_MASTER, &modbus_h));
    ESP_ERROR_CHECK(mbc_master_setup((void *)&mb_info));
    ESP_LOGE("MAIN", "Master configurado");
    ESP_ERROR_CHECK(SmartPlugModbus_init(&slave_0, &CID_count, 1));
    ESP_LOGE("MAIN", "Descriptor de test iniciado ");
    ESP_ERROR_CHECK(mbc_master_start());
    SmartPlugModbus_GetAll(&slave_0);
    xTaskCreate(SmartPlugModbus_Task, "SPM-Task", 8192, &SPM_ARRAY, tskIDLE_PRIORITY + 1, &SPM_task);
    while (true)
    {
        for (uint8_t i = 0; i <= ADDR_Plug_5; ++i)
        {
            SmartPlugModbus_PlugOn(&slave_0,i,0xFFFF,3);
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    vTaskSuspend(NULL);
}