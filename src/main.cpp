#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include <ArduinoJson.h>

#include "greenhouse/greenhouse_cache.hpp"
#include "connector/arduino_connector.hpp"
#include "logging/logging.hpp"
#include "config.hpp"


ESP8266WebServer server(80);
ArduinoConnector arduinoConnector(&Serial);
GreenhouseCache cache;


/**** Handlers ****/
void sendMeasures();


/**** Tasks ****/
void cacheLoader();


void setup() {
  Serial.begin(57600);
  logging::setup(logging::NOTHING, &Serial);
  delay(5000);
  
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  server.on("/measures", sendMeasures);
  server.begin();
}


void loop() {
  server.handleClient();
  cacheLoader();
}


void sendMeasures() {
  StaticJsonDocument<1024> doc;
  doc[F("first")][F("yellowTemperature")] = cache.first.yellowTemperature;
  doc[F("first")][F("greenTemperature")] = cache.first.greenTemperature;
  doc[F("first")][F("outsideTemperature")] = cache.first.outsideTemperature;
  doc[F("second")][F("yellowTemperature")] = cache.second.yellowTemperature;
  doc[F("second")][F("greenTemperature")] = cache.second.greenTemperature;
  doc[F("second")][F("outsideTemperature")] = cache.second.outsideTemperature;
  char result[1024];
  serializeJson(doc, result);
  server.send(200, F("application/json"), result);
}


void cacheLoader() {
  static uint32_t nextTimestamp = 5 * 1000lu;
  static constexpr uint32_t timeInterval = 5 * 1000lu;
  if (millis() < nextTimestamp) {
    return;
  }
  auto data = arduinoConnector.query(ArduinoConnector::COMMAND_GET_MEASURES);
  logging::debug(F("Lenght of data:"));
  logging::debug(int(data.size()));
  if (data.size() == sizeof(cache)) {
    std::size_t pos = 0;
    cache.first.yellowTemperature = data[pos++];
    cache.first.greenTemperature = data[pos++];
    cache.first.outsideTemperature = data[pos++];
    cache.second.yellowTemperature = data[pos++];
    cache.second.greenTemperature = data[pos++];
    cache.second.outsideTemperature = data[pos++];
  }
  nextTimestamp = millis() + timeInterval;
}