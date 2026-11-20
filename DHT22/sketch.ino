#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// =====================================
// KONFIGURACJA DHT22
// =====================================

const int DHT_PIN = 15;
const int DHT_TYPE = DHT22;

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================
// WIFI (Wokwi)
// =====================================

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// const char* WIFI_SSID = "KCKU_LM";
// const char* WIFI_PASSWORD = "banan1999";

// =====================================
// MQTT
// =====================================

const char* MQTT_SERVER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "HomeAssistant_WSBMerito";

// =====================================
// OBIEKTY I INTERWAL
// =====================================

WiFiClient espClient;
PubSubClient mqtt(espClient);

const unsigned long PUBLISH_INTERVAL = 3000;
unsigned long lastPublishTime = 0;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Laczenie z WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi polaczone!");
}

void reconnectMQTT() {
  static unsigned long lastMqttAttempt = 0;

  if (mqtt.connected()) {
    return;
  }

  if (millis() - lastMqttAttempt < 2000 && lastMqttAttempt != 0) {
    return;
  }
  lastMqttAttempt = millis();

  String clientId = "SmartHome-DHT22-";
  clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.println("Laczenie z MQTT...");
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("MQTT polaczone!");
  } else {
    Serial.print("Blad MQTT: ");
    Serial.println(mqtt.state());
  }
}

void publishSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Blad odczytu z DHT22!");
    return;
  }

  char payload[128];
  snprintf(
    payload,
    sizeof(payload),
    "{\"device\":\"temperature\",\"temperature\":%.1f,\"humidity\":%.0f}",
    t,
    h
  );

  if (mqtt.publish(MQTT_TOPIC, payload)) {
    Serial.print("Wyslano MQTT: ");
    Serial.println(payload);
  } else {
    Serial.println("Nie udalo sie wyslac MQTT!");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  reconnectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqtt.connected()) {
    reconnectMQTT();
  }

  mqtt.loop();

  if (millis() - lastPublishTime >= PUBLISH_INTERVAL) {
    lastPublishTime = millis();
    publishSensorData();
  }
}