#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <cmath>
#include "secrets.h"
#include "issTracker.h"

extern TFT_eSPI tft;

#define EARTH_RADIUS_KM 6371

double toRadians(double degrees) {
  return degrees * M_PI / 180.0;
}

// Haversine formula: great-circle distance between two lat/lon points
double haversineDistance(double isslatitude, double isslongitude) {
  double lat1 = toRadians(MY_LAT);
  double lon1 = toRadians(MY_LONG);
  double lat2 = toRadians(isslatitude);
  double lon2 = toRadians(isslongitude);
  double dLat = lat2 - lat1;
  double dLon = lon2 - lon1;
  double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
             std::cos(lat1) * std::cos(lat2) *
             std::sin(dLon / 2) * std::sin(dLon / 2);
  double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
  return EARTH_RADIUS_KM * c;
}

void issTracker() {
  if (WiFi.status() != WL_CONNECTED) {
    tft.println("WiFi not connected");
    Serial.println("WiFi not connected");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, "https://api.wheretheiss.at/v1/satellites/25544");

  int httpCode = http.GET();
  if (httpCode != 200) {
    tft.println("Failed to reach ISS API");
    Serial.println("Failed to reach ISS API, HTTP code: " + String(httpCode));
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    tft.println("Failed to parse ISS data");
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  float lat = doc["latitude"];
  float lon = doc["longitude"];
  double distance = haversineDistance(lat, lon);

  tft.println("ISS position:");
  tft.println(String(lat) + ", " + String(lon));
  tft.println("Distance: " + String(distance) + " km");

  if (distance <= 400) {
    tft.println("Overhead!");
  }

  Serial.println("ISS position: " + String(lat) + ", " + String(lon));
  Serial.println("Distance: " + String(distance) + " km");
  if (distance <= 400) {
    Serial.println("Ahoy! ISS overhead!");
  } else {
    Serial.println("The ISS is elsewhere.");
  }
}