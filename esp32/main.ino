#include "DHTesp.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Definição de pinos
#define DHT_PIN 4      // Pino DATA do DHT11
#define LED_PIN 5      // LED de alerta
#define LDR_PIN 3      // LDR conectado ao ADC

// Cria objeto DHTesp
DHTesp dht;

// Credenciais WiFi
const char* ssid = "SENAC-Mesh";       // coloque sua rede WiFi
const char* password = "09080706";  // coloque a senha

// Chave e URL do ThingSpeak
String apiKey = "0ZUXWK7UG4KHTV9E";  
const char* server = "http://api.thingspeak.com/update";

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Inicializa DHT11
  dht.setup(DHT_PIN, DHTesp::DHT11);
  delay(2000);

  // Conecta WiFi
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

  // Mostra valores no monitor serial
  Serial.print("Temp: "); Serial.print(temp); Serial.print("°C | ");
  Serial.print("Umidade: "); Serial.print(hum); Serial.print("% | ");
  Serial.print("Luz: "); Serial.println(lightLevel);

  // Lógica de alerta
  if (temp > 30 || hum < 40 || lightLevel < 500) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("🚨 ALERTA ATIVADO!");
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Envia dados para o ThingSpeak
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
    } else {
      Serial.println("❌ Erro ao enviar dados.");
    }
    http.end();
  }

  Serial.println("-------------------------");
  delay(20000); // ThingSpeak aceita 1 leitura a cada 15s (20s seguro)
}
