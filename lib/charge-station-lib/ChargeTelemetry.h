#ifndef CHARGERTELEMETRY_H
#define CHARGERTELEMETRY_H

#pragma once

#include <Arduino.h>

struct ChargeTelemetry
{
    // Properties
    uint8_t state;

      // Methods
    ChargeTelemetry();
    ~ChargeTelemetry();
};


#endif