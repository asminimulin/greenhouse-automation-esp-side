#ifndef GREENHOUSE_CACHE_HPP
#define GREENHOUSE_CACHE_HPP

#include <cstdint>


struct Measures {
    int8_t yellowTemperature;
    int8_t greenTemperature;
    int8_t outsideTemperature;
};


struct GreenhouseCache {
    GreenhouseCache() {
        first = {-125, -125, -125};
        second = {-125, -125, -125};
    }
    Measures first, second;
};

#endif