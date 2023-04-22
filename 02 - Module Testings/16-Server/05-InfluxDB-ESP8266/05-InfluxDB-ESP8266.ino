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
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

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

// InfluxDB APIs
#define INFLUXDB_URL "https://europe-west1-1.gcp.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "zX9jxCRqaZg3XmoLDHMWIrXvNKfNN98jcFFCLsijceGo0lgkwzirGAJmm6JvYkb_8rQxYKv6CSWsxKwT7Efm8g=="
#define INFLUXDB_ORG "39be5680e12a0d07"
#define INFLUXDB_BUCKET "Autonomous Wireless Agrometeorology Station - Server test - ESP8266"

// Time zone info
#define TZ_INFO "UTC7"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensorReadings("measurements");

// Variable to hold temperature readings
float temperature, humidity, pressure;

// Create a sensor object
Adafruit_BME280 bme;  // I2C

// Ticker instance
Ticker MinuteUpdate;

// Server control and monitor
bool InfluxDB_updateFlag = false;
int failCount = 0;
String lastErrorMessage = "";

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
  
  // Add tags
  sensorReadings.addTag("device", DEVICE);
  sensorReadings.addTag("location", "test_site");
  sensorReadings.addTag("sensor", "bme280");

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (!client.validateConnection()) {
    while (1);    // Do nothing
  }

  InfluxDB_updateFlag = true;
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

  if (InfluxDB_updateFlag) {
    InfluxDB_updateFlag = false;
    ServerUpdateRoutine();
  }
}

void TimerOverflowRoutine() {
  InfluxDB_updateFlag = true;
}

void ServerUpdateRoutine() {
  // Take sensor readings
  bme.takeForcedMeasurement();
  temperature = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  humidity = bme.readHumidity();

  // Add readings as fields to point
  sensorReadings.addField("temperature", temperature);
  sensorReadings.addField("humidity", humidity);
  sensorReadings.addField("pressure", pressure);

  // Print what is going to be written
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensorReadings));

  // Write point into buffer
  client.writePoint(sensorReadings);

  // Clear fields for next usage. Tags remain the same.
  sensorReadings.clearFields();
}
