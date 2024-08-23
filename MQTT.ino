#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

// network credentials and ThingsBoard token
const char* ssid = "WIFI-SSID";
const char* password = "WIFI-PASSWORD";
const char* thingsboardServer = "demo.thingsboard.io";
const char* accessToken = "YOUR-ACCESS-TOKEN";

// Pin where the Temperature sensor is connected
#define ONE_WIRE_BUS 27

// OneWire and DallasTemperature instances
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient my_client(espClient);

// Functions for connecting to WiFi and MQTT
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void connect_To_MQTT() {
  while (!my_client.connected()) {
    Serial.print("Connecting to ThingsBoard MQTT server...");
    bool conn = my_client.connect("ESP32Client", accessToken, "");
    if (conn) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(my_client.state());
      Serial.println(" Trying to connect in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Starting WiFi and MQTT clients
  connectToWiFi();
  my_client.setServer(thingsboardServer, 1883);

  // Start the DS18B20 sensor
  sensors.begin();
}

void loop() {
  if (!my_client.connected()) {
    connect_To_MQTT();
  }
  my_client.loop();

  // Request temperature value
  sensors.requestTemperatures();

  // Read the temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  // Creating a JSON payload
  String mqtt_payload = "{";
  mqtt_payload += "\"temperature\":" + String(temperatureC);
  mqtt_payload += "}";

  // Publish the payload to ThingsBoard Dashboard
  my_client.publish("v1/devices/me/telemetry", mqtt_payload.c_str());

  // Wait before sending the next reading
  delay(3000);  // Send data every 3 seconds
}
