#include "logging.hpp"

namespace {  //  module private

Print* writer;
logging::LoggingLevel level;


void print(const __FlashStringHelper* text, bool printNewLine=true) {
    auto* ptr = reinterpret_cast<const char*>(text);
    if (writer) {
        for (int i = 0; ; ++i) {
            char c = pgm_read_byte(ptr + i);
            if (c == '\0') break;
            writer->write(c);
        }
    }
    // add newline
    if (printNewLine)
        writer->write('\n');
}

}

namespace logging {

void setup(LoggingLevel level, Print* output) {
    writer = output;
    ::level = level;
}


void debug(const __FlashStringHelper* message) {
    if (LoggingLevel::DEBUG >= level) {
        print(F("Debug -> "), false);
        print(message);
    }
}


void debug(const int& t) {
    if (level <= LoggingLevel::DEBUG) {
        print(F("Debug -> "), false);
        writer->print(t);
        writer->print('\n');
    }
}


void warning(const __FlashStringHelper* message) {
    if (LoggingLevel::WARNING >= level) {
        print(F("Warning -> "), false);
        print(message);
    }
}

void error(const __FlashStringHelper* message) {
    if (LoggingLevel::ERROR >= level) {
        print(F("Error -> "), false);
        print(message);
    }
}


void info(const __FlashStringHelper* message) {
    if (LoggingLevel::INFO >= level) {
        print(F("Info -> "), false);
        print(message);
    }
}

}