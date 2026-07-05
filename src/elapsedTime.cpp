#include "elapsedTime.h"
#include <time.h>
#include <TFT_eSPI.h>
#include <Arduino.h>

extern TFT_eSPI tft;

const time_t referenceTime = -743155560; // reference point, in seconds since epoch

void elapsedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10000)) {
    tft.println("Failed to obtain time");
    Serial.println("Failed to obtain time");
    return;
  }

  time_t now = mktime(&timeinfo);
  time_t age = now - referenceTime;

  tft.println("Donald Trump is");
  tft.println("Years: " + String((long)age / (365 * 24 * 60 * 60)));
  tft.println("Days: " + String((long)age / (24 * 60 * 60) % 365));
  tft.println("Hours: " + String((long)age / (60 * 60) % 24));
  tft.println("Minutes: " + String((long)age / 60 % 60));
  tft.println("Seconds: " + String((long)age % 60));
  tft.println("old.");

  Serial.println("Donald Trump is");
  Serial.println("Years: " + String((long)age / (365 * 24 * 60 * 60)));
  Serial.println("Days: " + String((long)age / (24 * 60 * 60) % 365));
  Serial.println("Hours: " + String((long)age / (60 * 60) % 24));
  Serial.println("Minutes: " + String((long)age / 60 % 60));
  Serial.println("Seconds: " + String((long)age % 60));
  Serial.println("old.");
}