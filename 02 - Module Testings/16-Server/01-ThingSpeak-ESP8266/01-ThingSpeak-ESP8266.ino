/*
  Adapted from WriteSingleField Example from ThingSpeak Library (Mathworks)
  
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-thingspeak-publish-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <Arduino.h>

// ESP8266 WiFi-handlers
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"

// Database-handling library
#include "ThingSpeak.h"

// Sensor-handling libraries
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

// ESP8266 timer: Ticker
#include <Ticker.h>


#define SEALEVELPRESSURE_HPA (1013.25)

// WiFi network credentials
// WiFi AP SSID
#define WIFI_SSID "Station - test network"
// WiFi password
#define WIFI_PASSWORD "testnetwork"

WiFiClient client;

// ThingSpeak channel access configurations
unsigned long channelNumber = 2103173;
const char *writeAPIKey = "5PVRJ36BC4RTN489";
const char *readAPIKey = "9Z59G0ZU5UZC72VX";

// Variable to hold temperature readings
float temperature, humidity, pressure;

// Create a sensor object
Adafruit_BME280 bme;  // I2C

// Ticker instance
Ticker MinuteUpdate;

// Server control and monitor
bool ThingSpeak_updateFlag = false;
int failCount = 0;
int lastErrorCode = 200;

void setup() {
  Serial.begin(115200);  //Initialise serial
  
  if (!bme.begin(0x76, &Wire)) {     // Check for BME280 at I2C address 0x76
    if (!bme.begin(0x77, &Wire)) {   // Check for BME280 at I2C address 0x77
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
    }
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
  
  WiFi.mode(WIFI_STA); 

  bool WiFi_connected = false;
  unsigned long attempt = 0;
  do {
    Serial.print("(");
    Serial.print(attempt);
    Serial.print(") Attempting to connect to ");
    Serial.println(WIFI_SSID);
    
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
    if (wifiMulti.run() == WL_CONNECTED) {
      WiFi_connected = true;
      Serial.print("Connected. IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      attempt += 1;
      delay(5000);
    }
  } while (!WiFi_connected);
  
  ThingSpeak.begin(client);  // Initialise ThingSpeak
  ThingSpeak_updateFlag = true;
  
  int failCount_temp = ThingSpeak.readIntField(channelNumber, 4, readAPIKey);
  if (0 < failCount_temp) failCount = failCount_temp;

  MinuteUpdate.attach(60, TimerOverflowRoutine);
}

void loop() {
  // Connect or reconnect to WiFi
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    while (wifiMulti.run() != WL_CONNECTED) {
      wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);  // Connect to WPA/WPA2 network
      Serial.print(".");
      delay(5000);     
    }
    Serial.println("\nConnected.");
  }

  if (ThingSpeak_updateFlag) {
    ThingSpeak_updateFlag = false;
    ServerUpdateRoutine();
  }
}

void TimerOverflowRoutine() {
  ThingSpeak_updateFlag = true;
}

void ServerUpdateRoutine() {
  // Take sensor readings
  bme.takeForcedMeasurement();
  temperature = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  humidity = bme.readHumidity();

  // Update fields
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pressure);
  ThingSpeak.setField(4, failCount);

  if (200 != lastErrorCode) {
    ThingSpeak.setField(5, lastErrorCode);
    lastErrorCode = 200;
  }

  // Write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(channelNumber, writeAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    failCount += 1;
    lastErrorCode = x;
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}
