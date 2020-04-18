#ifndef ARDUINO_CONNECTOR_HPP
#define ARDUINO_CONNECTOR_HPP

#include <vector>
#include <cstdint>

#include <Stream.h>


class ArduinoConnector {
public:
    enum Command: uint8_t {
        COMMAND_GET_MEASURES = '1',
    };

    ArduinoConnector(Stream* stream);
    std::vector<uint8_t> query(Command command);

private:
    Stream* stream_;

private:
    enum ErrorCode: uint8_t {
        OK = 0,
        EOS = 255, // End of stream
    };
    uint8_t readSync(uint32_t timeout = 0);
};

#endif