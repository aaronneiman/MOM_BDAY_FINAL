// Libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <cmath>
#include "secrets.h"
#include <WiFiClientSecure.h>

// Constants
#define EARTH_RADIUS_KM 6371 // Earth radius in KM
#define EARTH_RADIUS_MI 3958.8 // Earth radius in MI

// Helper function to convert degrees to radians
double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

// Helper function to find great circle distance via haversine equation, written by Claude
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

    double distanceInKm = EARTH_RADIUS_KM * c;

    return distanceInKm;
}
// Setup, runs once on startup
void setup() {
  Serial.begin(115200); // Establish connection between computer and device, establish baud rate
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Establish connection to wifi given ssid and password
  while (WiFi.status() != WL_CONNECTED){ // As long as wifi isn't connected
    delay(500); // Wait half a second and try again
  }
  Serial.println("WiFi connected!"); // Once wifi is connected, tell the user
  pinMode(2, OUTPUT); // Run power out of pin 2
}
// Loop, runs continuously 
void loop() {
  WiFiClientSecure client; // Build a 'vehicle' called "client" that will allow us to establish a secure connection
  client.setInsecure(); // Tell vehicle "client" to ignore website's ID badge

  HTTPClient http; // Creates a web-request-capable object called http
  http.begin(client, "https://api.wheretheiss.at/v1/satellites/25544"); // Tells the object to drive client to this address
  
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
    }
    
    float lat = doc["latitude"];
    float lon = doc["longitude"];

    Serial.print("Current ISS coordinates: ");
    Serial.print(lat);
    Serial.print(", ");
    Serial.println(lon);
    Serial.println("Current great circle distance: ");
    Serial.print(haversineDistance(lat, lon));
    Serial.println(" kilometers away");

    if (haversineDistance(lat, lon) <= 400) {
      digitalWrite(2, HIGH);
      Serial.println("Ahoy! ISS overhead!");
    } else {
      digitalWrite(2, LOW);
      Serial.println("The ISS is elsewhere. Keep checking.");
    }
    Serial.println();

    http.end();
  }
  delay(30000); // check every 30 seconds
}