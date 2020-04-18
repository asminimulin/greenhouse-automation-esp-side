#include <Arduino.h>

#include "arduino_connector.hpp"


ArduinoConnector::ArduinoConnector(Stream* stream):
    stream_(stream) {}


std::vector<uint8_t> ArduinoConnector::query(Command command) {
    static constexpr uint32_t maxDelay = 0;
    stream_->write(command);
    uint8_t code = readSync(maxDelay);
    if (code != 0) {
        return {};
    }
    uint8_t len = readSync(maxDelay);
    if (len == EOS) {
        return {};
    }
    std::vector<uint8_t> response(len);
    for (std::size_t i = 0; i < len; ++i) {
        auto code = readSync(maxDelay);
        if (code == EOS) {
            return {};
        }
        response[i] = code;
    }
    return response;
}


uint8_t ArduinoConnector::readSync(uint32_t timeout) {
    uint32_t stop = (timeout == 0) ? ~0u : millis() + timeout;
    if (timeout == 0) stop = ~0u;
    else stop = millis() + timeout;
    while (!stream_->available() && millis() < stop) {} 
    if (!stream_->available()) {
        return EOS;
    }
    return static_cast<uint8_t>(stream_->read());
}