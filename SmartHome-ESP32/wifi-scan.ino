#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// =====================================
// LCD
// =====================================

LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================================
// WIFI
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
// LED
// =====================================

const int LED_PIN = 18;

// =====================================
// OBIEKTY
// =====================================

WiFiClient espClient;
PubSubClient mqtt(espClient);

// =====================================
// DANE CZUJNIKOW
// =====================================

float temperature = 0;
float humidity = 0;

bool motion = false;
int brightness = 0;

// Informacja, czy otrzymalismy dane
bool temperatureKnown = false;
bool humidityKnown = false;
bool motionKnown = false;
bool brightnessKnown = false;

// Czas ostatniego wykrycia ruchu
unsigned long lastMotionTime = 0;

// Tryb swiatla
bool lightOn = false;

// Progi automatyki
const int DARK_THRESHOLD = 1000;
const unsigned long MOTION_TIMEOUT = 10000;

// =====================================
// ODBIERANIE WIADOMOSCI MQTT
// =====================================

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("Odebrano MQTT:");
  Serial.println(message);

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println("Blad JSON!");
    return;
  }

  // Aktualizacja tylko danych, ktore przyszly
  if (!doc["temperature"].isNull()) {
    temperature = doc["temperature"];
    temperatureKnown = true;
  }

  if (!doc["humidity"].isNull()) {
    humidity = doc["humidity"];
    humidityKnown = true;
  }

  if (!doc["motion"].isNull()) {
    motion = doc["motion"];
    motionKnown = true;

    if (motion) {
      lastMotionTime = millis();
    }
  }

  if (!doc["brightness"].isNull()) {
    brightness = doc["brightness"];
    brightnessKnown = true;
  }

  Serial.println("Dane zaktualizowane.");
}

// =====================================
// WIFI
// =====================================

void connectWiFi() {

  Serial.println("Laczenie z WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi polaczone!");
}

// =====================================
// MQTT
// =====================================

void reconnectMQTT() {

  static unsigned long lastMqttAttempt = 0;

  if (mqtt.connected()) {
    return;
  }

  if (millis() - lastMqttAttempt < 2000 && lastMqttAttempt != 0) {
    return;
  }
  lastMqttAttempt = millis();

  String clientId = "SmartHome-Central-";
  clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

  Serial.println("Laczenie z MQTT...");

  if (mqtt.connect(clientId.c_str())) {

    Serial.println("MQTT polaczone!");

    mqtt.subscribe(MQTT_TOPIC);

    Serial.println("Nasluchiwanie MQTT aktywne.");

  } else {

    Serial.print("Blad MQTT: ");
    Serial.println(mqtt.state());
  }
}

// =====================================
// LCD
// =====================================

void displayData() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");

  if (temperatureKnown) {
    lcd.print(temperature, 1);
    lcd.print("C");
  } else {
    lcd.print("Nieznana");
  }

  lcd.setCursor(0, 1);
  lcd.print("Wilg:");

  if (humidityKnown) {
    lcd.print(humidity, 0);
    lcd.print("%");
  } else {
    lcd.print("Nieznana");
  }
}

// =====================================
// AUTOMATYKA SWIATLA
// =====================================

void controlLight() {

  bool shouldTurnOn = false;

  // Automatykę uruchamiamy tylko,
  // gdy znamy ruch i jasność.

  if (motionKnown && brightnessKnown) {

    bool isDark = brightness < DARK_THRESHOLD;

    bool recentMotion =
      motionKnown && (millis() - lastMotionTime < MOTION_TIMEOUT);

    shouldTurnOn = recentMotion && isDark;
  }

  lightOn = shouldTurnOn;

  digitalWrite(LED_PIN, lightOn ? HIGH : LOW);
}

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("SmartHome");
  lcd.setCursor(0, 1);
  lcd.print("Uruchamianie");

  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  delay(1000);
  displayData();
}

// =====================================
// GLOWNA PETLA
// =====================================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqtt.connected()) {
    reconnectMQTT();
  }

  mqtt.loop();

  controlLight();

  static unsigned long lastDisplayUpdate = 0;

  if (millis() - lastDisplayUpdate >= 1000) {
    lastDisplayUpdate = millis();
    displayData();
  }
}