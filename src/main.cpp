#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <limits.h>

#define D0 16
#define D1  5
#define D2  4
#define D3  0
#define D4  2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

uint8_t const pins[] = { D0, D1, D2, D3, D4, D5, D6, D7, D8 };

#define RED D2
#define GREEN D0
#define BLUE D1

//#define POLL_PERIOD  30000 /* 30s */
#define POLL_PERIOD 300000 /*  5m */

String ssid;
String password;
String url;

String readString(char const * prompt) {
  Serial.printf("\n%s: ", prompt);
  auto str = Serial.readStringUntil('\n');
  str.trim();
  return str;
}

void setupWiFi() {
  Serial.setTimeout(LONG_MAX);
  ssid = readString("SSID");
  password = readString("password");
  auto apikey = readString("api key");
  auto location = readString("ST/city_name: ");
  url = "http://api.wunderground.com/api/" + apikey + "/conditions/q/" + location + ".json";
}

void setup() {
  for (auto pin : pins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 1);
  }

  Serial.begin(115200);
  setupWiFi();

  for (auto pin : pins) {
    digitalWrite(pin, 0);
  }
}

void loop() {
  WiFi.begin(ssid.c_str(), password.c_str());

  Serial.println("\nConnecting...\n");
  bool flashState = true;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(BLUE, flashState);
    delay(50);
    flashState = !flashState;
  }

  digitalWrite(BLUE, 1);
  digitalWrite(GREEN, 1);

  Serial.printf("Connected, IP address: %s\n", WiFi.localIP().toString().c_str());

  HTTPClient client;
  client.begin(url.c_str());
  int httpCode = client.GET();

  if (httpCode > 0) {
    Serial.printf("HTTP GET code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      auto json = client.getString();
      DynamicJsonBuffer buffer;
      auto const & root = buffer.parseObject(json);
      auto const & current = root["current_observation"].as<JsonObject>();
      auto const & weather = current["weather"];
      if (weather.is<char const *>()) {
        Serial.printf("The weather is %s\n", weather.as<char const *>());
        digitalWrite(RED, 0);
      } else {
        Serial.println("Something's wrong:");
        root.prettyPrintTo(Serial);
        digitalWrite(RED, 1);
      }
    }
  } else {
    Serial.printf("HTTP GET failure: %s\n", client.errorToString(httpCode).c_str());
    digitalWrite(RED, 1);
  }

  client.end();
  WiFi.disconnect(true);

  digitalWrite(BLUE, 0);
  Serial.println("\n\n\n");
  //while (1)
  delay(POLL_PERIOD);
}
