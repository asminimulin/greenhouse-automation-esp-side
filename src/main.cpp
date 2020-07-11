
#define ARDUINOJSON_USE_LONG_LONG 1

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <base64.hpp>
#include <logging.hpp>

#include "greenhouse/greenhouse_cache.hpp"
#include "connector/arduino_connector.hpp"
#include "config.hpp"


namespace buf_private {

static char buffer1[2048];
static uint8_t buffer2[2048];

};

ESP8266WebServer server(80);
ArduinoConnector arduinoConnector(&Serial);
GreenhouseCache cache;


/**** API Handlers ****/
void apiGetMeasures();
void apiGetSettings();
void apiSetSettings();
void apiSetYellowSensorAddress();
void apiSetGreenSensorAddress();
void apiSetOutsideSensorAddress();
void apiSetYellowWindowAddress();
void apiSetGreenWindowAddress();
void apiSetVentAddress();


/**** View Handlers ****/
void sendMeasures();


/**** Tasks ****/
void cacheLoader();


void setup() {
  Serial.begin(57600);
  logging::init(logging::NOTHING, &Serial);

  if (CREATE_ACCESS_POINT && USE_EXISTING_NETWORK) {
    WiFi.mode(WIFI_AP_STA);
  } else if (CREATE_ACCESS_POINT) {
    WiFi.mode(WIFI_AP);
  } else if (USE_EXISTING_NETWORK) {
    WiFi.mode(WIFI_STA);
  }

  if (USE_EXISTING_NETWORK) {
    WiFi.begin(NETWORK.WIFI_SSID, NETWORK.WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(50);
    }
  }

  if (CREATE_ACCESS_POINT) {
    WiFi.softAP(AP_SSID, AP_PASSWORD);
  }

  server.on("/api/measures", HTTP_GET, apiGetMeasures);
  server.on("/measures.html", HTTP_GET, sendMeasures);
  server.on("/api/settings", HTTP_GET, apiGetSettings);
  server.on("/api/settings", HTTP_POST, apiSetSettings);
  server.on("/api/yellow-sensor-address", HTTP_POST, apiSetYellowSensorAddress);
  server.on("/api/green-sensor-address", HTTP_POST, apiSetGreenSensorAddress);
  server.on("/api/outside-sensor-address", HTTP_POST, apiSetOutsideSensorAddress);
  server.on("/api/yellow-window-address", HTTP_POST, apiSetYellowWindowAddress);
  server.on("/api/green-window-address", HTTP_POST, apiSetGreenWindowAddress);
  server.on("/api/vent-address", HTTP_POST, apiSetVentAddress);
  server.begin();
  if (!MDNS.begin("Greenhouse")) {
    logging::error() << F("Failed to start MDNS");
    return;
  }
  MDNS.addService("automation", "tcp", 80);
}


void loop() {
  server.handleClient();
  cacheLoader();
  MDNS.update();
}


void cacheLoader() {
  static uint32_t nextTimestamp = 5 * 1000lu;
  static constexpr uint32_t timeInterval = 5 * 1000lu;
  if (millis() < nextTimestamp) {
    return;
  }
  auto data = arduinoConnector.query(ArduinoConnector::COMMAND_GET_MEASURES);
  if (data.size() == sizeof(cache.measures)) {
    memcpy(&(cache.measures), &data.front(), data.size());
  }
  data = arduinoConnector.query(ArduinoConnector::COMMAND_GET_SETTINGS);
  if (data.size() == sizeof(cache.settings)) {
    memcpy(&(cache.settings), &data.front(), data.size());
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
    "</body>"
    "<style>"
    "body {"
      "font-size: 40px;"
    "}"
    "</style>"
    "</html>";
  sprintf(buf_private::buffer1, format
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
  );
  server.send(200, "text/html", buf_private::buffer1);
}

void apiGetSettings()
{
  StaticJsonDocument<2048> doc;
  Settings& settings = cache.settings;
  doc[F("openingTime")] = settings.openingTime;
  doc[F("temperatureInnercyDelay")] = settings.temperatureInnercyDelay;
  doc[F("openingTemperature")] = settings.openingTemperature;
  doc[F("closingTemperature")] = settings.closingTemperature;
  doc[F("ventOnTemperature")] = settings.ventOnTemperature;
  doc[F("stepsCount")] = settings.stepsCount;
  doc[F("summerMode")] = settings.summerMode;
  doc[F("ventMode")] = settings.ventMode;
  String result;
  serializeJson(doc, result);
  server.send(200, F("application/json"), result);
}

void apiSetSettings()
{
  auto json = server.arg("plain");
  StaticJsonDocument<2048> doc;
  deserializeJson(doc, json);
  Settings settings;
  settings.openingTime = doc[F("openingTime")];
  settings.temperatureInnercyDelay = doc[F("temperatureInnercyDelay")];
  settings.openingTemperature = doc[F("openingTemperature")];
  settings.closingTemperature = doc[F("closingTemperature")];
  settings.ventOnTemperature = doc[F("ventOnTemperature")];
  settings.stepsCount = doc[F("stepsCount")];
  settings.summerMode = doc[F("summerMode")];
  settings.ventMode = doc[F("ventMode")];
  char *buffer = new char[sizeof(settings)];
  memcpy(buffer, &settings, sizeof(settings));
  arduinoConnector.query(ArduinoConnector::COMMANG_SET_SETTINGS,
                         sizeof(settings),
                         reinterpret_cast<const uint8_t*>(&settings));
  cache.settings = settings;
  server.send(200, F("application/json"), F("{success:true}"));
}

void sendBadRequest() {
  server.send(400, F("application/json"), F("{\"status\": \"error\", \"error\": \"Bad request\"}"));
}

void sendSuccess() {
  server.send(200, F("application/json"), F("{\"status\": \"success\"}"));
}

void apiSetYellowSensorAddress() {
  unsigned char buf[256];
  constexpr size_t addressLength = 8;
  auto addressKey = F("address_base64");
  auto json = server.arg("plain");
  StaticJsonDocument<2048> document;
  if (deserializeJson(document, json) || !document.containsKey(addressKey)) {
    sendBadRequest();
    return;
  }
  String addressBase64 = document[addressKey];
  if (addressBase64.length() > addressLength * 2) {
    sendBadRequest();
    return;
  }
  memcpy(buf, addressBase64.c_str(), addressBase64.length() + 1);
  size_t bytesCount = decode_base64(buf, buf_private::buffer2);
  if (bytesCount != 8) {
    sendBadRequest();
    return;
  }
  arduinoConnector.query(ArduinoConnector::COMMAND_SET_YELLOW_SENSOR_ADDRESS, addressLength, buf_private::buffer2);
  sendSuccess();
}

void apiSetGreenSensorAddress() {
  unsigned char buf[256];
  constexpr size_t addressLength = 8;
  auto addressKey = F("address_base64");
  auto json = server.arg("plain");
  StaticJsonDocument<2048> document;
  if (deserializeJson(document, json) || !document.containsKey(addressKey)) {
    sendBadRequest();
    return;
  }
  String addressBase64 = document[addressKey];
  if (addressBase64.length() > addressLength * 2) {
    sendBadRequest();
    return;
  }
  memcpy(buf, addressBase64.c_str(), addressBase64.length() + 1);
  size_t bytesCount = decode_base64(buf, buf_private::buffer2);
  if (bytesCount != 8) {
    sendBadRequest();
    return;
  }
  arduinoConnector.query(ArduinoConnector::COMMAND_SET_GREEN_SENSOR_ADDRESS, addressLength, buf_private::buffer2);
  sendSuccess();
}

void apiSetOutsideSensorAddress() {
  unsigned char buf[256];
  constexpr size_t addressLength = 8;
  auto addressKey = F("address_base64");
  auto json = server.arg("plain");
  StaticJsonDocument<2048> document;
  if (deserializeJson(document, json) || !document.containsKey(addressKey)) {
    sendBadRequest();
    return;
  }
  String addressBase64 = document[addressKey];
  if (addressBase64.length() > addressLength * 2) {
    sendBadRequest();
    return;
  }
  memcpy(buf, addressBase64.c_str(), addressBase64.length() + 1);
  size_t bytesCount = decode_base64(buf, buf_private::buffer2);
  if (bytesCount != 8) {
    sendBadRequest();
    return;
  }
  arduinoConnector.query(ArduinoConnector::COMMAND_SET_OUTSIDE_SENSOR_ADDRESS, addressLength, buf_private::buffer2);
  sendSuccess();
}

void apiSetYellowWindowAddress() {
  unsigned char buf[256];
  constexpr size_t addressLength = 8;
  auto addressKey = F("address_base64");
  auto json = server.arg("plain");
  StaticJsonDocument<2048> document;
  if (deserializeJson(document, json) || !document.containsKey(addressKey)) {
    sendBadRequest();
    return;
  }
  String addressBase64 = document[addressKey];
  if (addressBase64.length() > addressLength * 2) {
    sendBadRequest();
    return;
  }
  memcpy(buf, addressBase64.c_str(), addressBase64.length() + 1);
  size_t bytesCount = decode_base64(buf, buf_private::buffer2);
  if (bytesCount != 8) {
    sendBadRequest();
    return;
  }
  arduinoConnector.query(ArduinoConnector::COMMAND_SET_YELLOW_WINDOW_ADDRESS, addressLength, buf_private::buffer2);
  sendSuccess();
}

void apiSetGreenWindowAddress() {
  unsigned char buf[256];
  constexpr size_t addressLength = 8;
  auto addressKey = F("address_base64");
  auto json = server.arg("plain");
  StaticJsonDocument<2048> document;
  if (deserializeJson(document, json) || !document.containsKey(addressKey)) {
    sendBadRequest();
    return;
  }
  String addressBase64 = document[addressKey];
  if (addressBase64.length() > addressLength * 2) {
    sendBadRequest();
    return;
  }
  memcpy(buf, addressBase64.c_str(), addressBase64.length() + 1);
  size_t bytesCount = decode_base64(buf, buf_private::buffer2);
  if (bytesCount != 8) {
    sendBadRequest();
    return;
  }
  arduinoConnector.query(ArduinoConnector::COMMAND_SET_GREEN_WINDOW_ADDRESS, addressLength, buf_private::buffer2);
  sendSuccess();
}

void apiSetVentAddress() {
  unsigned char buf[256];
  constexpr size_t addressLength = 8;
  auto addressKey = F("address_base64");
  auto json = server.arg("plain");
  StaticJsonDocument<2048> document;
  if (deserializeJson(document, json) || !document.containsKey(addressKey)) {
    sendBadRequest();
    return;
  }
  String addressBase64 = document[addressKey];
  if (addressBase64.length() > addressLength * 2) {
    sendBadRequest();
    return;
  }
  memcpy(buf, addressBase64.c_str(), addressBase64.length() + 1);
  size_t bytesCount = decode_base64(buf, buf_private::buffer2);
  if (bytesCount != 8) {
    sendBadRequest();
    return;
  }
  arduinoConnector.query(ArduinoConnector::COMMAND_SET_VENT_ADDRESS, addressLength, buf_private::buffer2);
  sendSuccess();
}