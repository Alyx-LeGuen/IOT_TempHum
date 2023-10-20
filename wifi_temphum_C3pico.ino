// Prog by Alyx LG - RenardeQuiWoof
// C3 pico temp and humidity sensor connected to thingsboard
// You like IOT things, dont you ?

#include "ThingsBoard.h"
#include "DHT.h"
#include <WiFi.h>
#include <Adafruit_NeoPixel.h> // TODO remplacer par fastled

// WIFI param
#define WIFI_AP             "WIFI NAME"
#define WIFI_PASSWORD       "WIFI PASSWD"

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define SENSOR_NAME         "C3Pico NAME"
#define TOKEN               "THINGSBOARD TOKEN"
#define THINGSBOARD_SERVER  "THINGSBOARD URL"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

// Settings
#define LEDPIN    7 // LED Default pin on C3 Pico
#define NUMPIXELS 1 // LED Default pin on C3 Pico
Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
#define DHTPIN 6 // DHT PIN HERE
#define DHTTYPE DHT22 // DHT MODEL
DHT dht(DHTPIN, DHTTYPE);
#define RUNTIME 120000 // 120 secondes between update

// Initialize ThingsBoard and Wifi
WiFiClient espClient;
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD); // initialize serial for debugging
  dht.begin(); // initialize dht

  // initialize NeoPixel (red booting and green when setup finished)
  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, 1, 0));
  pixels.show();

  // initialize wifi
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  String WIFI_SENSOR_NAME=SENSOR_NAME;
  WiFi.setHostname(WIFI_SENSOR_NAME.c_str());
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  InitWiFi();

  pixels.setPixelColor(0, pixels.Color(1, 0, 0));
  pixels.show();
}

void loop() {
  delay(RUNTIME);
  // Start sending data, set neopixel to blue
  pixels.setPixelColor(0, pixels.Color(0, 0, 1));
  pixels.show();
  delay(100);

  // Temp and humidity
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Probleme read");
    return;
  }

  // Check/Connect wifi/thingsboard
  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }
  if (!tb.connected()) {
    Serial.printf("Connecting to: %s with token %s\n", THINGSBOARD_SERVER, TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  // Everythings OK - start sending telemetry
  Serial.println("Sending data...");

  tb.sendTelemetryFloat("temp", t);
  tb.sendTelemetryFloat("hum", h);
  tb.loop();

  // Go back to green and wait
  pixels.setPixelColor(0, pixels.Color(1, 0, 0));
  pixels.show();
}

void InitWiFi()
{
  // attempt to connect to WiFi network
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}
