#ifndef ARDUINO_CONNECTOR_HPP
#define ARDUINO_CONNECTOR_HPP

#include <vector>
#include <cstdint>

#include <Stream.h>


class ArduinoConnector {
public:
    enum Command: uint8_t {
        COMMAND_PING = '0',
        COMMAND_GET_MEASURES = '1',
        COMMAND_GET_SETTINGS = '2',
        COMMANG_SET_SETTINGS = '3',
        COMMAND_SET_YELLOW_SENSOR_ADDRESS = 0x20,
        COMMAND_SET_GREEN_SENSOR_ADDRESS = 0x21,
        COMMAND_SET_OUTSIDE_SENSOR_ADDRESS = 0x22,
        COMMAND_SET_YELLOW_WINDOW_ADDRESS = 0x23,
        COMMAND_SET_GREEN_WINDOW_ADDRESS = 0x24,
        COMMAND_SET_VENT_ADDRESS = 0x25,
    };

    ArduinoConnector(Stream* stream);
    std::vector<uint8_t> query(Command command, size_t argumentLength = 0, const uint8_t* argumentData = nullptr);

    void loop();

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