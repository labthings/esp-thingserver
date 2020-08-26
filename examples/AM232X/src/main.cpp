#include <Arduino.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

// Make sure we use an async web server for the WiFi manager
#define WM_ASYNC
#include <AsyncWiFiManager.h>

#include <Thing.h>
#include <WebThingAdapter.h>

Adafruit_AM2320 am2320 = Adafruit_AM2320();
WebThingAdapter *adapter;

// Define basic Thing attributes
const char *am232Types[] = {"TemperatureSensor", "HumiditySensor", nullptr};
ThingDevice weather("weather", "AM232X Weather Sensor", am232Types);
ThingProperty weatherTemp("temperature", "", NUMBER, "TemperatureProperty");
ThingProperty weatherHum("humidity", "", NUMBER, "HumidityProperty");

// Value objects to hold our sensor data
ThingPropertyValue temperatureValue;
ThingPropertyValue humidityValue;

// Define our functions

void readAM232XData() {
  // Read latest AM2320 and write them to our Thing properties
  temperatureValue.number = am2320.readTemperature();
  weatherTemp.setValue(temperatureValue);
  humidityValue.number = am2320.readHumidity();
  weatherHum.setValue(humidityValue);
}

void setup(void) {
  Serial.begin(115200);

  // Create a WiFi station
  WiFi.mode(WIFI_STA);

  // Start WiFi auto-connect routine
  AsyncWiFiManager awm;
  Serial.println("Running autoConnect");

  // Hack for https://github.com/tzapu/WiFiManager/issues/979
  awm.setEnableConfigPortal(false);
  if (!awm.autoConnect()) {
    Serial.println("Retry autoConnect");
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    awm.setEnableConfigPortal(true);
    awm.autoConnect();
  }
  // End of hack

  if (!awm.autoConnect()) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Start I2C
    am2320.begin();

    // Add Thing properties
    weatherTemp.unit = "celsius";
    weatherTemp.readOnly = true;
    weather.addProperty(&weatherTemp);
    weatherHum.unit = "percent";
    weatherHum.readOnly = true;
    weather.addProperty(&weatherHum);

    // Create our Thing adapter
    adapter = new WebThingAdapter(&weather, "AM2320", WiFi.localIP());
    // Start the server
    adapter->begin();
    Serial.println("HTTP server started");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
  }
}

void loop(void) {
  // Update sensor data
  readAM232XData();
  // Update Thing adapter
  adapter->update();
}
