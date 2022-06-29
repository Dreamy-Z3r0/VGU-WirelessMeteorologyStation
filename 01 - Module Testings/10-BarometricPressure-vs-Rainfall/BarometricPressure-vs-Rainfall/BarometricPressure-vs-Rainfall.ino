#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// WiFi AP SSID
#define WIFI_SSID "#ThanosDidNothingWrong_2.4GHz"
// WiFi password
#define WIFI_PASSWORD "perfectlybalanced"
// InfluxDB v2 server url
#define INFLUXDB_URL "https://europe-west1-1.gcp.cloud2.influxdata.com"
// InfluxDB v2 server or cloud API token
#define INFLUXDB_TOKEN "2gHfy65I931P33qydCua8mUD22vf3mjjPpri_SpDVWroGfE6QDC6nCa869Hhkm2POQIcIwEtW6mxAugMvLNQlg=="
// InfluxDB v2 organization id
#define INFLUXDB_ORG "eeit2016_hoang.dn@student.vgu.edu.vn"
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET "BarometricPressure"

// Set timezone string
#define TZ_INFO "<-07>7"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensor("barometric_pressure");

#define SENSOR "BME280"
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
unsigned long delayTime;

void setup() {
  Serial.begin(9600);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Add tag(s)
  sensor.addTag("device", DEVICE);
  sensor.addTag("sensor", SENSOR);
  sensor.addTag("datatype", "BAROMETRIC_PRESSURE");

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  if (!bme.begin(0x77, &Wire)) {
    Serial.println("Could not find a valid BME280 sensor. Check wiring.");
    while (1);
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,     // forced mode
                  Adafruit_BME280::SAMPLING_X1,     // temperature oversampling
                  Adafruit_BME280::SAMPLING_X1,     // pressure oversampling
                  Adafruit_BME280::SAMPLING_X1,     // humidity oversampling
                  Adafruit_BME280::FILTER_OFF   );  // filter off
  delayTime = 60000;

  Serial.println("--- Weather monitoring ---");
  Serial.println("-> forced mode, filter off, 1x temperature / 1x humidity / 1x pressure oversampling\n");
}

void loop() {
  static float BME_BarometricPressure;

  // Take a forced measurement on the BME280 and store barometric pressure in hPa
  bme.takeForcedMeasurement();
  BME_BarometricPressure = bme.readPressure() / 100.0F;

  // Print the new data on the serial monitor
  Serial.print("Barometric pressure = ");
  Serial.print(BME_BarometricPressure);
  Serial.println(" hPa");

  // Store measured value into point
  sensor.clearFields();
  // Write the field key and the field value to the point
  sensor.addField("pressure", BME_BarometricPressure);

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
    Serial.println();
  }

  delay(delayTime);
}
