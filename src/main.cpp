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

void print_P(char const * pstr) {
    size_t const BUF_SIZE = 128;
    char buf[BUF_SIZE];
    strncpy_P(buf, pstr, BUF_SIZE);
    Serial.print(buf);
}

void println_P(char const * pstr) {
    print_P(pstr);
    Serial.println();
}

#define PRINT_P(str) print_P(PSTR(str))
#define PRINTLN_P(str) PRINT_P(str "\n")

void setup() {
    for (auto pin : pins) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, 1);
    }

    Serial.begin(115200);
    PRINTLN_P("\n\nInitializing");

    for (auto pin : pins) {
        digitalWrite(pin, 0);
    }
}

bool connectWiFi() {
    PRINT_P("Connecting to WiFi");

    WiFi.begin(CONFIG_SSID, CONFIG_PASSWORD);

    digitalWrite(WIFI_LED, 1);

    while (true) {
        switch (WiFi.status()) {
            case WL_IDLE_STATUS:
            case WL_DISCONNECTED:
                Serial.print('.');
                break;
            case WL_CONNECTED:
                PRINT_P("Connected\nIP address: ");
                Serial.println(WiFi.localIP().toString().c_str());
                return true;
            case WL_CONNECT_FAILED:
                PRINTLN_P("Error\nConnect failed");
                return false;
            case WL_CONNECTION_LOST:
                PRINTLN_P("Connection lost");
                return false;
            default:
                PRINT_P("Other WiFi connection error: ");
                Serial.printf("0x%08Xd\n", WiFi.status());
                return false;
        }

        digitalWrite(WIFI_LED, 0);
        delay(125);
        digitalWrite(WIFI_LED, 1);
        delay(125);
    }

    digitalWrite(WIFI_LED, 1);
    digitalWrite(ERROR_LED, 1);
    return false;
}

void disconnectWiFi() {
    WiFi.disconnect(true);
    digitalWrite(WIFI_LED, 1);
}

String fetchWeatherJson() {
    if (!connectWiFi()) {
        return String();
    }

    HTTPClient client;
    client.begin("http://api.wunderground.com/api/"
                 CONFIG_APIKEY
                 "/conditions/q/"
                 CONFIG_LOCATION
                 ".json");
    PRINTLN_P("Fetching JSON");
    int httpCode = client.GET();

    String ret;
    if (httpCode > 0) {
        PRINT_P("HTTP GET code: ");
        Serial.println(httpCode);
        if (httpCode == HTTP_CODE_OK) {
            ret = client.getString();
        }
        digitalWrite(WIFI_LED, 0);
    } else {
        PRINT_P("HTTP GET failure: ");
        Serial.println(client.errorToString(httpCode));
        digitalWrite(ERROR_LED, 1);
        analogWrite(WIFI_LED, 128);
    }

    client.end();
    WiFi.disconnect(true);

    return ret;
}

void loop() {
    PRINTLN_P("\nUpdating weather");
    Serial.flush();

    auto json = fetchWeatherJson();
    if (json.length() > 0) {
        DynamicJsonBuffer buffer;
        PRINTLN_P("Parsing JSON");
        auto const & root = buffer.parseObject(json);
        auto const & current = root["current_observation"].as<JsonObject>();
        auto const & description = current["weather"];
        auto const & icon = current["icon"];
        if (description.is<char const *>() && icon.is<char const *>()) {
            PRINT_P("The weather is ");
            Serial.println(description.as<char const *>());
            auto weather = getWeather(icon.as<char const *>());
            PRINT_P("Weather enum value: ");
            Serial.printf("%d - ", static_cast<int>(weather));
            println_P(weatherName(weather));
            digitalWrite(ERROR_LED, 0);
            digitalWrite(FETCHED_LED, 1);
        } else {
            PRINTLN_P("Something's wrong:");
            root.prettyPrintTo(Serial);
            digitalWrite(ERROR_LED, 1);
        }
    }

    //while (1)
    delay(CONFIG_POLL_PERIOD);
}
