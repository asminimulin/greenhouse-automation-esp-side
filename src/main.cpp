#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

#include "greenhouse/greenhouse_cache.hpp"
#include "connector/arduino_connector.hpp"
#include "logging/logging.hpp"
#include "config.hpp"


static char buffer[2048];


ESP8266WebServer server(80);
ArduinoConnector arduinoConnector(&Serial);
GreenhouseCache cache;


/**** Handlers API ****/
void apiGetMeasures();
void apiGetSettings();
// void apiSetSettings();


/**** Handlers view ****/
void sendMeasures();


/**** Tasks ****/
void cacheLoader();


void setup() {
  Serial.begin(57600);
  logging::setup(logging::ALL, &Serial);

  WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/api/measures", HTTP_GET, apiGetMeasures);
  server.on("/measures.html", HTTP_GET, sendMeasures);
  // server.on("api/settings", HTTP_GET, apiGetSettings);
  // server.on("api/settings", HTTP_POST, apiSetSettings);
  server.begin();
  if (!MDNS.begin("automation")) {
    logging::error(F("Failed to start MDNS"));
    return;
  }
  MDNS.addService("greenhouse", "tcp", 80);
}


void loop() {
  server.handleClient();
  cacheLoader();
  MDNS.update();
}


void cacheLoader() {
  static uint32_t nextTimestamp = 5 * 1000lu;
  static constexpr uint32_t timeInterval = 5 * 1000lu;
  int readCount = 0;
  if (millis() < nextTimestamp) {
    return;
  }
  auto data = arduinoConnector.query(ArduinoConnector::COMMAND_GET_MEASURES, readCount);
  if (data.size() == sizeof(cache.measures)) {
    memcpy(&(cache.measures), &data.front(), data.size());
    cache.error = "No errors";
  } else if (readCount == 0) {
    cache.error = "No response";
  } else {
    cache.error = String("Data size mismatch ") + sizeof(cache.measures) + " != " + data.size() + "Read count = " + readCount;
  } 
  nextTimestamp = millis() + timeInterval;
}


void apiGetMeasures()
{
  StaticJsonDocument<2048> doc;
  Measures* m = &cache.measures;
  doc[F("outsideTemperature")] = m->outsideTemperature;
  doc[F("yellowTemperature")] = m->yellowTemperature;
  doc[F("greenTemperature")] = m->greenTemperature;
  doc[F("ventStatus")] = m->ventStatus;
  String result;
  serializeJson(doc, result);
  server.send(200, F("application/json"), result);
}

void sendMeasures()
{
  auto ON = "ON";
  auto OFF = "OFF";
  auto format = "<html>"
    "<head>"
    "<meta charset='UTF-8' />"
    "</head>"
    "<body>"
    "<p>Наружняя температура: %d</p>"
    
    "<h3>Первая теплица</h3>"
    "<p>Жёлтая температура: %d</p>"
    "<p>Зелёная температура: %d</p>"
    "<p>Вентилятор: %s</p>"
    "<p>Открытие жёлтого окна: %d%%</p>"
    "<p>Открытие зелёного окна: %d%%</p>"
    "<p>Влажность синяя: %d%%</p>"
    "<p>Синий полив: %s</p>"
    "<p>Влажность красная: %d%%</p>"
    "<p>Красный полив: %s</p>"
    // "<h1>Error:%s</h1>"
    "</body>"
    "<style>"
    "body {"
      "font-size: 40px;"
    "}"
    "</style>"
    "</html>";
  sprintf(buffer, format
    , int(cache.measures.outsideTemperature)
    , int(cache.measures.yellowTemperature)
    , int(cache.measures.greenTemperature)
    , (cache.measures.ventStatus ? ON : OFF)
    , int(cache.measures.yellowWindowPercent)
    , int(cache.measures.greenWindowPercent)
    , int(cache.measures.blueHumidity)
    , (cache.measures.blueWateringStatus ? ON : OFF)
    , int(cache.measures.redHumidity)
    , (cache.measures.redWateringStatus ? ON : OFF)
    // , cache.error.c_str()
  );
  server.send(200, "text/html", buffer);
}

// void apiGetSettings()
// {
//   StaticJsonDocument<1024> doc;
//   doc[F("first")] = cache.firstSettings.openingTemperature;
//   doc[F("first")] = cache.firstSettings.closingTemperature;
//   doc[F("first")] = cache.firstSettings.stepsCount;
//   doc[F("second")] = cache.secondSettings.openingTemperature;
//   doc[F("second")] = cache.secondSettings.closingTemperature;
//   doc[F("second")] = cache.secondSettings.stepsCount;
//   char* result = ::buffer;
//   serializeJson(doc, result);
//   server.send(200, F("application/json"), result);
// }