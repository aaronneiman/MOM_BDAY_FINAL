#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <TFT_eSPI.h>
#include "weatherNow.h"
#include "secrets.h" // must define: MY_LAT, MY_LON

extern TFT_eSPI tft;

void weatherNow() {
  if (WiFi.status() != WL_CONNECTED) {
    tft.println("WiFi not connected");
    Serial.println("WiFi not connected");
    return;
  }

  // Figure out which array index corresponds to "right now"
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10000)) {
    tft.println("Failed to obtain time");
    Serial.println("Failed to obtain time");
    return;
  }
  int currentHourIndex = timeinfo.tm_hour;

  String url = String("https://api.open-meteo.com/v1/forecast?latitude=")
               + MY_LAT + "&longitude=" + MY_LONG
               + "&current=apparent_temperature"
               + "&hourly=precipitation_probability,dew_point_2m"
               + "&wind_speed_unit=mph&temperature_unit=fahrenheit"
               + "&forecast_days=1";

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != 200) {
    tft.println("Failed to retrieve weather data");
    Serial.println("Weather request failed, HTTP code: " + String(httpCode));
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, payload)) {
    tft.println("Failed to parse weather data");
    Serial.println("Failed to parse weather data");
    return;
  }

  float feelsLike  = doc["current"]["apparent_temperature"];
  int   rainChance = doc["hourly"]["precipitation_probability"][currentHourIndex];
  float dewPoint   = doc["hourly"]["dew_point_2m"][currentHourIndex];

  tft.println("Feels like: " + String(feelsLike) + "F");
  tft.println("Rain chance: " + String(rainChance) + "%");
  tft.println("Dew point: " + String(dewPoint) + "F");

  Serial.println("Feels like: " + String(feelsLike) + "F");
  Serial.println("Rain chance: " + String(rainChance) + "%");
  Serial.println("Dew point: " + String(dewPoint) + "F");
}