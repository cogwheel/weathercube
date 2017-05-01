#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <limits.h>
#include <pgmspace.h>

#include "weather_types.hpp"

#include "../config.h"

// Available GPIO pins for the nodemcuv2 dev board
#define D0  16
#define D1   5
#define D2   4
#define D3   0
#define D4   2
#define D5  14
#define D6  12
#define D7  13
#define D8  15

// Helper for setting them all in a loop
uint8_t const pins[] = { D0, D1, D2, D3, D4, D5, D6, D7, D8 };

#define ERROR_LED D2
#define FETCHED_LED D0
#define WIFI_LED D1

#define FOGGER D3
#define PUMP D7
#define FAN D8

#define RED_LED D4
#define GREEN_LED D5
#define BLUE_LED D6

//#define POLL_PERIOD  30000 /* 30s */
#define POLL_PERIOD 300000 /*  5m */

void print_P(char const * pstr) {
  size_t const BUF_SIZE = 128;
  char buf[BUF_SIZE];
  strncpy_P(buf, pstr, BUF_SIZE);
  Serial.print(buf);
}

void println_P(char const * pstr) {
  print_P(pstr);
  Serial.print('\n');
}

void setup() {
  for (auto pin : pins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 1);
  }

  Serial.begin(115200);
  println_P(PSTR("\n\nInitializing"));

  for (auto pin : pins) {
    digitalWrite(pin, 0);
  }
}

void connectWiFi() {
  print_P(PSTR("Connecting to WiFi..."));

  WiFi.begin(CONFIG_SSID, CONFIG_PASSWORD);

  // TODO: handle failure
  bool flashState = true;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(WIFI_LED, flashState);
    delay(50);
    flashState = !flashState;
  }

  digitalWrite(WIFI_LED, 1);

  print_P(PSTR("Connected, IP address: "));
  Serial.println(WiFi.localIP().toString().c_str());
}

void disconnectWiFi() {
  WiFi.disconnect(true);
  digitalWrite(WIFI_LED, 1);
}

String fetchWeatherJson() {
  connectWiFi();

  HTTPClient client;
  client.begin("http://api.wunderground.com/api/"
               CONFIG_APIKEY
               "/conditions/q/"
               CONFIG_LOCATION
               ".json");
  println_P(PSTR("Fetching JSON"));
  int httpCode = client.GET();

  if (httpCode > 0) {
    print_P(PSTR("HTTP GET code: "));
    Serial.println(httpCode);
    if (httpCode == HTTP_CODE_OK) {
      return client.getString();
    }
  } else {
    print_P(PSTR("HTTP GET failure: "));
    Serial.println(client.errorToString(httpCode));
    digitalWrite(ERROR_LED, 1);
  }

  client.end();
  WiFi.disconnect(true);
}

void loop() {
  println_P(PSTR("\nUpdating weather"));
  Serial.flush();

  auto json = fetchWeatherJson();
  DynamicJsonBuffer buffer;
  println_P(PSTR("Parsing JSON"));
  auto const & root = buffer.parseObject(json);
  auto const & current = root["current_observation"].as<JsonObject>();
  auto const & description = current["weather"];
  auto const & icon = current["icon"];
  if (description.is<char const *>() &&
      icon.is<char const *>()) {
        print_P(PSTR("The weather is "));
        Serial.println(description.as<char const *>());
        auto weather = getWeather(icon.as<char const *>());
        print_P(PSTR("Weather enum value: "));
        Serial.printf("%d - ", static_cast<int>(weather));
        println_P(weatherName(weather));
        digitalWrite(ERROR_LED, 0);
        digitalWrite(FETCHED_LED, 1);
  } else {
    println_P(PSTR("Something's wrong:"));
    root.prettyPrintTo(Serial);
    digitalWrite(ERROR_LED, 1);
  }

  //while (1)
  delay(POLL_PERIOD);
}
