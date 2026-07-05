#include <time.h>
#include <Arduino.h>
#include "currentTime.h"
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

void currentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10000)) {
    tft.println("Failed to obtain time");
    Serial.println("Failed to obtain time");
    return;
  }
  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

  tft.println("Current time: " + String(buffer));
  Serial.println("Current time: " + String(buffer));
}