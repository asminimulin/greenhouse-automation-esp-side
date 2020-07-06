#ifndef GREENHOUSE_CACHE_HPP
#define GREENHOUSE_CACHE_HPP

#include <cstdint>


struct Measures {
    int8_t yellowTemperature;
    int8_t greenTemperature;
    int8_t outsideTemperature;
    uint8_t ventStatus;
    uint8_t yellowWindowPercent;
    uint8_t greenWindowPercent;
    uint8_t blueHumidity;
    uint8_t blueWateringStatus;
    uint8_t redHumidity;
    uint8_t redWateringStatus;
};


#pragma pack(push, 1)
struct Settings
{
    uint64_t openingTime;
    uint64_t temperatureInnercyDelay;
    int8_t openingTemperature;
    int8_t closingTemperature;
    int8_t ventOnTemperature;
    uint8_t stepsCount;
    uint8_t summerMode;
    uint8_t ventMode;
};
#pragma pack(pop)


struct GreenhouseCache {
    Measures measures;
    Settings settings;
    String error;
};

#endif
