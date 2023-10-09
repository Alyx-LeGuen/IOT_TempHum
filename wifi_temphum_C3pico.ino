// Prog by Alyx LG - RenardeQuiWoof
// C3 pico temp and humidity sensor connected to thingsboard
// You like IOT things, dont you ?

#include "ThingsBoard.h"
#include "DHT.h"
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// WIFI param
#define WIFI_AP             "WIFI NAME"
#define WIFI_PASSWORD       "WIFI PASSWD"

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN               "THINGSBOARD TOKEN"
#define THINGSBOARD_SERVER  "THINGSBOARD URL"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

// Settings
#define LEDPIN    7 // LED Default pin on C3 Pico
#define NUMPIXELS 1 // TODO remplacer par fastled
Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
#define DHTPIN 6 // DHT PIN HERE
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#define UPLOADTIME 5000 // 5 secondes between update

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

void setup() {
  // initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  dht.begin();
  pixels.begin();

  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, 1, 0));
  pixels.show();
  String hostname = "C3Pico Client";
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(hostname.c_str()); //define hostname
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  InitWiFi();
  pixels.setPixelColor(0, pixels.Color(1, 0, 0));
  pixels.show();
}

void loop() {
  delay(UPLOADTIME);
  pixels.setPixelColor(0, pixels.Color(0, 0, 1));
  pixels.show();
  delay(200);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Probleme read");
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    reconnect();
  }

  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect");
      return;
    }
  }

  Serial.println("Sending data...");

  // Uploads new telemetry to ThingsBoard using MQTT.
  // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api
  // for more details

  tb.sendTelemetryFloat("temp", t);
  tb.sendTelemetryFloat("hum", h);
  tb.loop();
  pixels.setPixelColor(0, pixels.Color(1, 0, 0));
  pixels.show();
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

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
