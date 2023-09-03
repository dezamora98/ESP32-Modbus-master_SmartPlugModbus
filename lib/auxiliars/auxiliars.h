#ifndef AUXILIARS_H
#define AUXILIARS_H
#pragma once

#include <charge_station_config.h>

void initLocker();
void lock();
void unlock();

// #if DEBUG_DISABLED
#if true
#define PRINT_LOCK
#define PRINT_UNLOCK
#else
#define PRINT_LOCK lock();
#define PRINT_UNLOCK unlock();
#endif

/**
 * @brief  wraps a function call with a take and give the mutext passed as parameter.
 * @param func: the function that will be call
 * @param mtx: the mutex that lock the function for save call. This mtx needs to be created and initialized by the user before this call.
 * @retval None
 */
void sec_call(void (*func)(), SemaphoreHandle_t mtx);

/**
 * @brief  Take the mutex passed on the constructor and gives it back when the obj created is destroyed (e.g. when goes out of it's scope been no static).
 */
class MutexLocker
{
    SemaphoreHandle_t _mtx;

public:
    /**
     * @brief  Create a locker object that takes a mutex passed as argument and will be give it back whe this object be destroyed.
     * @param  mtx: The mutex that performs the lock. Needs to be created and initialized by the user before this call.
     */
    MutexLocker(SemaphoreHandle_t mtx) : _mtx(mtx)
    {
        BaseType_t rc = xSemaphoreTake(_mtx, portMAX_DELAY);
        assert(rc == pdPASS);
    }
    ~MutexLocker()
    {
        BaseType_t rc = xSemaphoreGive(_mtx);
        assert(rc == pdPASS);
    }
};

#endif // AUXILIARS_H
