#include <Arduino.h>

#include "arduino_connector.hpp"
#include <logging/logging.hpp>

ArduinoConnector::ArduinoConnector(Stream* stream):
    stream_(stream) {}


std::vector<uint8_t> ArduinoConnector::query(Command command, size_t argumentLength, const uint8_t* argumentData) {
    static constexpr uint32_t maxDelay = 50;
    stream_->write(command);
    if (argumentLength && argumentData) {
        stream_->write(argumentData, argumentLength);
    }
    uint8_t code = readSync(maxDelay * 5);
    if (code == EOS) {
        return {};
    }
    if (code != 0) return {};
    uint8_t len = readSync(maxDelay);
    if (len == EOS) {
        return {};
    }
    std::vector<uint8_t> response(len);
    for (std::size_t i = 0; i < len; ++i) {
        auto code = readSync(maxDelay);
        if (code == EOS) {
            return response;
        }
        response[i] = code;
    }
    return response;
}


uint8_t ArduinoConnector::readSync(uint32_t timeout) {
    uint32_t stop = (timeout == 0) ? ~0u : millis() + timeout;
    while (!stream_->available() && millis() < stop) {} 
    if (!stream_->available()) {
        return EOS;
    }
    return static_cast<uint8_t>(stream_->read());
}

void ArduinoConnector::loop() {
    if (stream_->available()) {
        Command cmd = Command(stream_->read());
        if (cmd == COMMAND_PING) {
            static constexpr uint8_t responsePong = COMMAND_PING;
            stream_->write(responsePong);
        } else {
            logging::error(F("Bad command from arduino"));
        }
    }
}