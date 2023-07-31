#include "auxiliars.h"

#include <Arduino.h>

static SemaphoreHandle_t mutexLocker = nullptr;

void initLocker()
{
    if (!mutexLocker)
    {
        mutexLocker = xSemaphoreCreateMutex();
        assert(mutexLocker);
    }
}

/**
 * @brief  utility function to easy lock resources before using print on RTOS
 */
void lock()
{
    BaseType_t rc;
    rc = xSemaphoreTake(mutexLocker, portMAX_DELAY);
    assert(rc == pdPASS);
}

/**
 * @brief  utility function to easy unlock resources previosly locked by lock() function.
 */
void unlock()
{
    BaseType_t rc;
    rc = xSemaphoreGive(mutexLocker);
    assert(rc == pdPASS);
}

void sec_call(void (*func)(), SemaphoreHandle_t mtx)
{
    BaseType_t rc = xSemaphoreTake(mtx, portMAX_DELAY);
    assert(rc == pdPASS);

    func();

    rc = xSemaphoreGive(mtx);
    assert(rc == pdPASS);
}