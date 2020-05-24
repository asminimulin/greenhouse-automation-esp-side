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


/**** Handlers API ****/
void apiGetMeasures();
// void apiGetSettings();
// void apiSetSettings();


/**** Handlers view ****/
void sendMeasures();


/**** Tasks ****/
void cacheLoader();


void setup() {
  Serial.begin(57600);
  logging::setup(logging::NOTHING, &Serial);
  delay(5000);

  Serial.print("AP_SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP_PASWORD: ");
  Serial.println(AP_PASSWORD);
  
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  // WiFi.begin(AP_SSID, AP_PASSWORD);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(50);
  // }

  Serial.print(F("Ip address: "));
  Serial.println(WiFi.localIP());

  server.on("/api/measures", HTTP_GET, apiGetMeasures);
  server.on("/measures.html", HTTP_GET, sendMeasures);
  // server.on("api/settings", HTTP_GET, apiGetSettings);
  // server.on("api/settings", HTTP_POST, apiSetSettings);
  server.begin();
}


void loop() {
  server.handleClient();
  cacheLoader();
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
    cache.outsideTemperature = data[pos++];
    memcpy(&(cache.first), &data[pos], sizeof(cache.first));
    pos += sizeof(cache.first);
    memcpy(&(cache.second), &data[pos], sizeof(cache.second));
  }
  nextTimestamp = millis() + timeInterval;
}


void apiGetMeasures() {
  StaticJsonDocument<2048> doc;
  doc[F("outsideTemperature")] = cache.outsideTemperature;
  Measures* c[] = {&(cache.first), &(cache.second)};
  const __FlashStringHelper* names[] = {F("first"), F("second")};
  for (int i = 0; i < 2; ++i) {
    Measures* m = c[i];
    auto name = names[i];
    doc[name][F("yellowTemperature")] = m->yellowTemperature;
    doc[name][F("greenTemperature")] = m->greenTemperature;
    doc[name][F("ventStatus")] = m->ventStatus;
    doc[name][F("yellowWindowPercent")] = m->yellowWindowPercent;
    doc[name][F("greenWindowPercent")] = m->greenWindowPercent;
    doc[name][F("blueHumidity")] = m->blueHumidity;
    doc[name][F("blueWateringStatus")] = m->blueWateringStatus;
    doc[name][F("redHumidity")] = m->redHumidity;
    doc[name][F("redWateringStatus")] = m->redWateringStatus;
  }
  char result[2048];
  serializeJson(doc, result);
  server.send(200, F("application/json"), result);
}


char buffer[2048];
void sendMeasures() {
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
    
    "<h3>Вторая теплица</h3>"
    "<p>Жёлтая температура: %d</p>"
    "<p>Зелёная температура: %d</p>"
    "<p>Вентилятор: %s</p>"
    "<p>Открытие жёлтого окна: %d%%</p>"
    "<p>Открытие зелёного окна: %d%%</p>"
    "<p>Влажность синяя: %d%%</p>"
    "<p>Синий полив: %s</p>"
    "<p>Влажность красная: %d%%</p>"
    "<p>Красный полив: %s</p>"
    "</body>"
    "<style>"
    "body {"
      "font-size: 40px;"
    "}"
    "</style>"
    "</html>";
  sprintf(buffer, format
    , int(cache.outsideTemperature)

    , int(cache.first.yellowTemperature)
    , int(cache.first.greenTemperature)
    , (cache.first.ventStatus ? ON : OFF)
    , int(cache.first.yellowWindowPercent)
    , int(cache.first.greenWindowPercent)
    , int(cache.first.blueHumidity)
    , (cache.first.blueWateringStatus ? ON : OFF)
    , int(cache.first.redHumidity)
    , (cache.first.redWateringStatus ? ON : OFF)

    , int(cache.second.yellowTemperature)
    , int(cache.second.greenTemperature)
    , (cache.second.ventStatus ? ON : OFF)
    , int(cache.second.yellowWindowPercent)
    , int(cache.second.greenWindowPercent)
    , int(cache.second.blueHumidity)
    , (cache.second.blueWateringStatus ? ON : OFF)
    , int(cache.second.redHumidity)
    , (cache.second.redWateringStatus ? ON : OFF)
  );
  // sprintf(buffer, "Hello %s", "alexey");
  server.send(200, "text/html", buffer);
}