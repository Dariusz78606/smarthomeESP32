
#include <WiFi.h>
#include <PubSubClient.h>

// Wi-Fi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";
// const char* WIFI_SSID = "KCKU_LM";
// const char* WIFI_PASSWORD = "banan1999";

// MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "HomeAssistant_WSBMerito";

// Czujnik PIR
const int PIR_PIN = 18;

// Obiekty komunikacyjne
WiFiClient espClient;
PubSubClient client(espClient);

// Ostatni odczyt czujnika
int lastMotion = -1;

void connectWiFi() {
  WiFi.begin(ssid, password);

  Serial.print("Laczenie z Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nPolaczono z Wi-Fi!");
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Laczenie z MQTT...");

    if (client.connect("SmartHome-Motion-ESP32-03")) {
      Serial.println("Polaczono!");
    } else {
      Serial.print("Blad: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);

  connectWiFi();

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  int motion = digitalRead(PIR_PIN);

  // Wysylamy wiadomosc, gdy stan czujnika sie zmieni
  if (motion != lastMotion) {

    String message = "{";
    message += "\"device\":\"motion\",";
    message += "\"motion\":";
    message += (motion == HIGH ? "true" : "false");
    message += "}";

    client.publish(mqtt_topic, message.c_str());

    Serial.print("Wyslano: ");
    Serial.println(message);

    lastMotion = motion;
  }

  delay(100);
}
