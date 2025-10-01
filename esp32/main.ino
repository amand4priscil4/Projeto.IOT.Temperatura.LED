#include "DHTesp.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define DHT_PIN 4
#define LED_PIN 5
#define LDR_PIN 3

DHTesp dht;

const char* ssid = "*******************";
const char* password = "******************";
String apiKey = "******************";  
const char* server = "http://api.thingspeak.com/update";

// Temporizadores
unsigned long previousMillisSend = 0;
unsigned long previousMillisSerial = 0;
const long intervalSend = 20000;   // 20s para envio
const long intervalSerial = 3000;  // 3s para mostrar no monitor

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.setup(DHT_PIN, DHTesp::DHT11);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void loop() {
  // Lê sensores
  float temp = dht.getTemperature();
  float hum = dht.getHumidity();
  int lightLevel = analogRead(LDR_PIN);

  // Lógica de alerta do LED
  if (temp > 30 || hum < 40 || lightLevel < 500) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Mostra dados no Serial a cada 3s
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisSerial >= intervalSerial) {
    previousMillisSerial = currentMillis;

    Serial.print("Temp: "); Serial.print(temp); Serial.print("°C | ");
    Serial.print("Umidade: "); Serial.print(hum); Serial.print("% | ");
    Serial.print("Luz: "); Serial.println(lightLevel);

    if (digitalRead(LED_PIN)) {
      Serial.println("🚨 ALERTA ATIVADO!");
    }
    Serial.println("-------------------------");
  }

  // Envia dados para ThingSpeak a cada 20s
  if (currentMillis - previousMillisSend >= intervalSend) {
    previousMillisSend = currentMillis;

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String(server) + "?api_key=" + apiKey +
                   "&field1=" + String(temp) +
                   "&field2=" + String(hum) +
                   "&field3=" + String(lightLevel);

      http.begin(url);
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.println("✅ Dados enviados ao ThingSpeak!");
        Serial.println("-------------------------");
      } else {
        Serial.println("❌ Erro ao enviar dados.");
        Serial.println("-------------------------");
      }
      http.end();
    }
  }
}
