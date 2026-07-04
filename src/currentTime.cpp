#include <time.h>
#include <Arduino.h>
#include "currentTime.h"

extern TFT_eSPI tft; // Declare the external TFT_eSPI instance

void currentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10000)) {
    Serial.println("Failed to obtain time");
    return;
  }

  char buffer[80];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  tft.println("Current time: " + String(buffer));
}

