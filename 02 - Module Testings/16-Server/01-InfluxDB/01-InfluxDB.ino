/*
  Rui Santos
  Complete project details at our blog: https://RandomNerdTutorials.com/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/
// Based on this library example: https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/blob/master/examples/SecureBatchWrite/SecureBatchWrite.ino

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "ESP32"
#elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
  #define WIFI_AUTH_OPEN ENC_TYPE_NONE
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID "#ThanosDidNothingWrong_2.4GHz"
// WiFi password
#define WIFI_PASSWORD "perfectlybalanced"

#define INFLUXDB_URL "https://europe-west1-1.gcp.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "jCJvDd2bMDxxABb3hjycWp23ZQKNN1BFzqu-gjm_I11rmSkZoR2-0H0qyTUpunsYJYZV3vnXvTw2de77MHyY1w=="
#define INFLUXDB_ORG "39be5680e12a0d07"
#define INFLUXDB_BUCKET "Senior_Project"

// Time zone info
#define TZ_INFO "UTC7"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensorReadings("measurements");

//BME280
Adafruit_BME280 bme; // I2C

float temperature;
float humidity;
float pressure;

// Initialize BME280
void initBME(){
  // BME280 initialisation on I2C bus
  if (!bme.begin(0x76)) {
    if (!bme.begin(0x77)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
    }
  }

  // BME280 settings for indoor navigation
  bme.setSampling( Adafruit_BME280::MODE_NORMAL,      // Sensor mode: normal
                   Adafruit_BME280::SAMPLING_X2,      // Temperature oversampling: x2
                   Adafruit_BME280::SAMPLING_X16,     // Pressure oversampling: x16
                   Adafruit_BME280::SAMPLING_X1,      // Humidity oversampling: x1
                   Adafruit_BME280::FILTER_X16,       // Filter coefficient: 16
                   Adafruit_BME280::STANDBY_MS_0_5    // Normal mode: t_standby = 0.5 ms
                 );
}

void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  
  //Init BME280 sensor
  initBME();
  
  // Add tags
  sensorReadings.addTag("device", DEVICE);
  sensorReadings.addTag("location", "office");
  sensorReadings.addTag("sensor", "bme280");

  // Accurate time is necessary for certificate validation and writing in batches
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  // Get latest sensor readings
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure()/100.0F;

  // Add readings as fields to point
  sensorReadings.addField("temperature", temperature);
  sensorReadings.addField("humidity", humidity);
  sensorReadings.addField("pressure", pressure);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensorReadings));
  
  // Write point into buffer
  client.writePoint(sensorReadings);

  // Clear fields for next usage. Tags remain the same.
  sensorReadings.clearFields();

  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}
