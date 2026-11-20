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
