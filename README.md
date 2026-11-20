# SmartHome - Rozproszony system IoT (ESP32 + MQTT)

Projekt prezentujący architekturę rozproszonego inteligentnego domu opartego na mikrokontrolerach **ESP32** oraz protokole **MQTT**. Osobne węzły symulowane w środowisku **Wokwi** odczytują parametry otoczenia i przesyłają je do fizycznej centrali sterującej wyświetlaczem oraz oświetleniem.

## Architektura systemu

System składa się z czterech niezależnych urządzeń komunikujących się przez wspólny broker MQTT:

| Urządzenie | Środowisko | Elementy | Zadanie |
| :--- | :--- | :--- | :--- |
| **ESP32 nr 1 (Centrala)** | Fizyczne | LCD 16x2 I2C, dioda LED, rezystor 220 Ω | Odbiór danych MQTT, prezentacja na LCD, automatyka światła |
| **ESP32 nr 2** | Wokwi | Czujnik DHT22 | Pomiar temperatury i wilgotności |
| **ESP32 nr 3** | Wokwi | Czujnik PIR | Wykrywanie ruchu |
| **ESP32 nr 4** | Wokwi | Fotorezystor LDR | Pomiar poziomu jasności |

## Komunikacja MQTT i format danych

Wszystkie węzły publikują wiadomości w formacie JSON na ten sam topic, a centrala rozróżnia źródło na podstawie kluczy w odebranym obiekcie:

* **Broker:** `broker.hivemq.com`
* **Port:** `1883`
* **Topic:** `HomeAssistant_WSBMerito`

### Przykładowe wiadomości JSON

* **Temperatura i wilgotność (ESP32 nr 2):**
  ```json
  {"device": "temperature", "temperature": 24.5, "humidity": 55}
  ```
* **Ruch (ESP32 nr 3):**
  ```json
  {"device": "motion", "motion": true}
  ```
* **Jasność (ESP32 nr 4):**
  ```json
  {"device": "brightness", "brightness": 350}
  ```

## Połączenia sprzętowe centrali (ESP32 nr 1)

| Element | Pin / Połączenie w ESP32 |
| :--- | :--- |
| **LCD SDA** | GPIO 21 |
| **LCD SCL** | GPIO 22 |
| **LCD VCC** | 5V |
| **LCD GND** | GND |
| **Dioda LED** | GPIO 18 (przez rezystor 220 Ω do GND) |

## Logika automatyki

1. **Wyświetlacz LCD:** prezentuje aktualną temperaturę oraz wilgotność. Do momentu odebrania pierwszej paczki z danego czujnika wyświetla komunikat `Nieznana`.
2. **Sterowanie LED:** dioda zapala się automatycznie tylko wtedy, gdy jednocześnie spełnione są dwa warunki: wykryto ruch w oknie czasowym (`MOTION_TIMEOUT`) oraz jasność spada poniżej ustalonego progu (`DARK_THRESHOLD`).

## Wymagane biblioteki (Arduino / PlatformIO / Wokwi)

* **Centrala:** `LiquidCrystal_I2C`, `PubSubClient`, `ArduinoJson` (v7)
* **Węzeł DHT22:** `DHT sensor library`, `Adafruit Unified Sensor`, `PubSubClient`
* **Węzły PIR i LDR:** `PubSubClient` (oraz wbudowane `WiFi.h`)
