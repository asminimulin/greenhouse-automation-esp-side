#ifndef GREENHOUSE_CACHE_HPP
#define GREENHOUSE_CACHE_HPP

#include <cstdint>


struct Measures {
    int8_t outsideTemperature;
    int8_t yellowTemperature;
    int8_t greenTemperature;
    uint8_t ventStatus;
    uint8_t yellowWindowPercent;
    uint8_t greenWindowPercent;
    uint8_t blueHumidity;
    uint8_t blueWateringStatus;
    uint8_t redHumidity;
    uint8_t redWateringStatus;
};


struct Settings
{
    int8_t openingTemperature;
    int8_t closingTemperature;
    uint8_t stepsCount;
};


struct GreenhouseCache {
    Measures measures;
    String error;
};

#endif
