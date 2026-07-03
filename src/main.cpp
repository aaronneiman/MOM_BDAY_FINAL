/*
  ============================================================
  MOM_BDAY_FINAL — main.cpp
  ============================================================
  This file is "plumbing only" — it does NOT contain any actual
  gift features (no weather logic, no ISS logic, etc). Those live in
  their own .cpp/.h file pairs and just get plugged in here.

  main.cpp's four jobs:
  1. WiFi connection      — on boot, and again if it ever drops
  2. OTA update check     — on boot, then once a day after that
  3. Button handling       — every loop
  4. Running whichever state is currently active — every loop

  To add a new state later:
    1. Create newthing_state.h / newthing_state.cpp (copy an existing
       pair as a template)
    2. #include the new header below
    3. Add the function name to the states[] array below
  Nothing else in this file needs to change — NUM_STATES updates itself.
  ============================================================
*/

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "secrets.h" // WIFI_SSID, WIFI_PASSWORD, WEATHER_API_KEY

// Each state's actual logic lives in its own file pair — main.cpp just
// knows their names, not how they work internally.

#include "clock_state.h"
#include "weather_state.h"
#include "iss_state.h"

// ------------------------------------------------------------
// CONFIG
// ------------------------------------------------------------

const char* VERSION_URL = "https://raw.githubusercontent.com/YOUR_USERNAME/YOUR_REPO/main/version.json";
#define FIRMWARE_VERSION 1
const unsigned long UPDATE_CHECK_INTERVAL = 24UL * 60UL * 60UL * 1000UL; // 24 hours

// TTGO T-Display buttons.
// GPIO0 has a usable internal pull-up. GPIO35 is input-only with an
// external pull-up already on the board, so it uses plain INPUT.
#define BUTTON_NEXT 0
#define BUTTON_PREV 35


// ------------------------------------------------------------
// STATE MACHINE
// ------------------------------------------------------------
// This array is the entire list of "screens" the gift can show. Order
// here is the order buttons cycle through. NUM_STATES is computed from
// the array's length, so it never needs manual updating.

int currentState = 0;

void (*states[])() = {
  stateClock,
  stateWeather,
  stateISS,
  // add new state function names here as you build them
};

const int NUM_STATES = sizeof(states) / sizeof(states[0]);


// ------------------------------------------------------------
// Function declarations
// ------------------------------------------------------------
void connectToWiFi();
void checkForUpdate();
void handleButtons();


// ------------------------------------------------------------
// SETUP — runs once, on power-up
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(BUTTON_PREV, INPUT);

  connectToWiFi();

  // Sync the clock once here via NTP. stateClock() then just reads the
  // chip's own clock afterward — no repeated network calls needed.
  configTime(0, 0, "pool.ntp.org");

  checkForUpdate(); // catch any update shipped while the device was off
}


// ------------------------------------------------------------
// LOOP — runs forever
// ------------------------------------------------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  static unsigned long lastUpdateCheck = 0;
  if (millis() - lastUpdateCheck > UPDATE_CHECK_INTERVAL) {
    checkForUpdate();
    lastUpdateCheck = millis();
  }

  handleButtons();

  states[currentState](); // run whichever state is currently active

  delay(1000);
}


// ------------------------------------------------------------
// BUTTON HANDLING
// ------------------------------------------------------------
void handleButtons() {
  static bool lastNext = HIGH;
  static bool lastPrev = HIGH;

  bool next = digitalRead(BUTTON_NEXT);
  bool prev = digitalRead(BUTTON_PREV);

  if (next == LOW && lastNext == HIGH) {
    currentState = (currentState + 1) % NUM_STATES;
    Serial.printf("State -> %d\n", currentState);
  }

  if (prev == LOW && lastPrev == HIGH) {
    currentState = (currentState - 1 + NUM_STATES) % NUM_STATES;
    Serial.printf("State -> %d\n", currentState);
  }

  lastNext = next;
  lastPrev = prev;
}


// ------------------------------------------------------------
// WIFI CONNECTION
// ------------------------------------------------------------
void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed, will retry in loop().");
  }
}


// ------------------------------------------------------------
// OTA UPDATE CHECK
// ------------------------------------------------------------
void checkForUpdate() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("Checking for firmware update...");

  HTTPClient http;
  http.begin(VERSION_URL);
  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("Version check failed, HTTP code: %d\n", httpCode);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, payload)) {
    Serial.println("Failed to parse version.json");
    return;
  }

  int remoteVersion = doc["version"] | 0;
  const char* binUrl = doc["bin_url"] | "";

  if (remoteVersion <= FIRMWARE_VERSION || strlen(binUrl) == 0) {
    Serial.println("Already up to date.");
    return;
  }

  Serial.printf("New version available: %d (current: %d). Updating...\n",
                remoteVersion, FIRMWARE_VERSION);

  WiFiClientSecure client;
  client.setInsecure();

  t_httpUpdate_return ret = httpUpdate.update(client, binUrl);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("Update failed. Error (%d): %s\n",
                    httpUpdate.getLastError(),
                    httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No update needed.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("Update successful, rebooting...");
      break;
  }
}