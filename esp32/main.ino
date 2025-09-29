#include "DHT.h"

#define DHT_PIN 4
#define LED_PIN 5  
#define LDR_PIN 34
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  
  Serial.println("Sistema de automação iniciado!");
}

void loop() {
  // Lê sensores
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightLevel = analogRead(LDR_PIN);
  
  // Mostra valores no monitor serial
  Serial.print("Temp: "); Serial.print(temp); Serial.print("°C | ");
  Serial.print("Umidade: "); Serial.print(humidity); Serial.print("% | ");
  Serial.print("Luz: "); Serial.println(lightLevel);
  
  // Lógica de automação
  if (temp > 30 || humidity < 40 || lightLevel < 500) {
    digitalWrite(LED_PIN, HIGH);  // Liga LED
    Serial.println("🚨 ALERTA ATIVADO!");
  } else {
    digitalWrite(LED_PIN, LOW);   // Desliga LED
  }
  
  delay(2000);  // Aguarda 2 segundos
}
