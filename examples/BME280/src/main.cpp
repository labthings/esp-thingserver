/*
  WiFi Web Server LED control via web of things (e.g., iot.mozilla.org gateway)
  based on WiFi101.h example "Provisioning_WiFiWebServer.ino"

 A simple web server that lets you control an LED via the web.
 This sketch will print the IP address of your WiFi device (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the onboard LED.

 You can also connect via the Things Gateway http-on-off-wifi-adapter.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Circuit:
 * WiFi using Microchip (Atmel) WINC1500
 * LED attached to pin 1 (onboard LED) for SAMW25
 * LED attached to pin 6 for MKR1000

 created 25 Nov 2012
 by Tom Igoe

 updates: dh, kg 2018
 */

#define LARGE_JSON_BUFFERS 1

#include <Arduino.h>
#define WM_ASYNC
#include <AsyncWiFiManager.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <Thing.h>
#include <WebThingAdapter.h>

#ifndef PIN_STATE_HIGH
#define PIN_STATE_HIGH HIGH
#endif
#ifndef PIN_STATE_LOW
#define PIN_STATE_LOW LOW
#endif

#if defined(LED_BUILTIN)
const int ledPin = LED_BUILTIN;
#else
const int ledPin = 13; // manually configure LED pin
#endif

WebThingAdapter *adapter;

const char *bme280Types[] = {"TemperatureSensor", nullptr};
ThingDevice weather("bme280", "BME280 Weather Sensor", bme280Types);
ThingProperty weatherTemp("temperature", "", NUMBER, "TemperatureProperty");
ThingProperty weatherHum("humidity", "", NUMBER, nullptr);
ThingProperty weatherPres("pressure", "", NUMBER, nullptr);

Adafruit_BME280 bme; // I2C

void readBME280Data() {
  ThingPropertyValue value;

  value.number = bme.readPressure();
  weatherPres.setValue(value);
  value.number = bme.readTemperature();
  weatherTemp.setValue(value);
  value.number = bme.readHumidity();
  weatherHum.setValue(value);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  AsyncWiFiManager awm;
  // awm.resetSettings();
  bool res;
  Serial.println("Running autoConnect");
  res = awm.autoConnect("AutoConnectAP", "password"); // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    digitalWrite(ledPin, HIGH); // active low led

    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // connected, make the LED stay on
    digitalWrite(ledPin, PIN_STATE_HIGH);

    // default settings
    unsigned status;
    status = bme.begin(BME280_ADDRESS_ALTERNATE);  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }

    weatherTemp.unit = "celsius";
    weather.addProperty(&weatherTemp);
    weather.addProperty(&weatherPres);
    weather.addProperty(&weatherHum);

    adapter = new WebThingAdapter(&weather, "weathersensor", WiFi.localIP());
    adapter->begin();
  }

}

void loop() {
  readBME280Data();
  adapter->update();
}
