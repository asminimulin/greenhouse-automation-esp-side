#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <Arduino.h>


namespace logging {

enum LoggingLevel {
    ALL,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    NOTHING,
};


void setup(LoggingLevel level, Print* output);


void debug(const __FlashStringHelper* message);


void error(const __FlashStringHelper* message);


void warning(const __FlashStringHelper* message);


void info(const __FlashStringHelper* message);


void debug(const int& i);

}

#endif