#include <Arduino.h>
#include <AM232X.h>

#define WM_ASYNC
#include <AsyncWiFiManager.h>

#include <Thing.h>
#include <WebThingAdapter.h>

AM232X AM2320;
WebThingAdapter *adapter;

const char *am232Types[] = {"TemperatureSensor", "HumiditySensor", nullptr};
ThingDevice weather("weather", "AM232X Weather Sensor", am232Types);
ThingProperty weatherTemp("temperature", "", NUMBER, "TemperatureProperty");
ThingProperty weatherHum("humidity", "", NUMBER, "HumidityProperty");

ThingPropertyValue temperatureValue;
ThingPropertyValue humidityValue;

void readAM232XData() {
  if (AM2320.read() == AM232X_OK) {
    temperatureValue.number = AM2320.getTemperature();
    weatherTemp.setValue(temperatureValue);
    humidityValue.number = AM2320.getHumidity();
    weatherHum.setValue(humidityValue);
  }
}

void setup(void) {
  Serial.begin(115200);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(AM232X_LIB_VERSION);

  WiFi.mode(WIFI_STA);

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

    Wire.begin();

    weatherTemp.unit = "celsius";
    weatherTemp.readOnly = true;
    weather.addProperty(&weatherTemp);
    weatherTemp.unit = "percent";
    weatherHum.readOnly = true;
    weather.addProperty(&weatherHum);

    adapter = new WebThingAdapter(&weather, "w25", WiFi.localIP());
    adapter->begin();
    Serial.println("HTTP server started");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
  }
}

void loop(void) {
  // TODDO: Write this function
  readAM232XData();
  adapter->update();
}
