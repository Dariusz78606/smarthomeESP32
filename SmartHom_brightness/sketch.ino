
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

// Czujnik jasnosci LDR
const int LDR_PIN = 34;

// Komunikacja
WiFiClient espClient;
PubSubClient client(espClient);

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

    if (client.connect("SmartHome-Brightness-ESP32-04")) {
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

  pinMode(LDR_PIN, INPUT);

  connectWiFi();

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  // Odczyt poziomu jasnosci
  int brightness = analogRead(LDR_PIN);

  // Przygotowanie wiadomosci JSON
  String message = "{";
  message += "\"device\":\"brightness\",";
  message += "\"brightness\":";
  message += String(4063-brightness);
  message += "}";

  // Wyslanie danych do MQTT
  client.publish(mqtt_topic, message.c_str());

  Serial.print("Wyslano: ");
  Serial.println(message);

  delay(3000);
}
