#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

// Configurações de Hardware
#define DHT_PIN 22
#define LED_PIN 2  
#define LDR_PIN 34
#define DHT_TYPE DHT22


// URL da API
const char* apiServer = "https://SEU-CODESPACE-URL.github.dev";

// Objetos
DHT dht(DHT_PIN, DHT_TYPE);
HTTPClient http;

// Variáveis globais
struct {
  float temperature_max = 30.0;
  float humidity_min = 40.0;
  int light_min = 500;
} thresholds;

struct {
  float temperature;
  float humidity;
  int light;
  bool isValid = false;
} sensorData;

unsigned long lastSensorRead = 0;
unsigned long lastApiSend = 0;
unsigned long lastThresholdFetch = 0;

bool wifiConnected = false;
bool automationActive = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando Sistema de Automação IoT");
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  dht.begin();
  
  connectWiFi();
  fetchThresholds();
  
  Serial.println("Sistema pronto!");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Verificar WiFi
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
  } else {
    wifiConnected = true;
  }
  
  // Ler sensores a cada 2 segundos
  if (currentTime - lastSensorRead >= 2000) {
    readSensors();
    runLocalAutomation();
    lastSensorRead = currentTime;
    
    if (sensorData.isValid) {
      Serial.printf("T:%.1f°C H:%.1f%% L:%d LED:%s WiFi:%s\n", 
                   sensorData.temperature, sensorData.humidity, sensorData.light,
                   automationActive ? "ON" : "OFF", wifiConnected ? "OK" : "FAIL");
    }
  }
  
  // Enviar dados para API a cada 10 segundos
  if (wifiConnected && currentTime - lastApiSend >= 10000) {
    sendDataToAPI();
    lastApiSend = currentTime;
  }
  
  // Buscar thresholds a cada 30 segundos
  if (wifiConnected && currentTime - lastThresholdFetch >= 30000) {
    fetchThresholds();
    lastThresholdFetch = currentTime;
  }
  
  delay(100);
}

void connectWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.println("WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("Falha na conexão WiFi - modo local");
  }
}

void readSensors() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightRaw = analogRead(LDR_PIN);
  
  if (isnan(temp) || isnan(humidity)) {
    Serial.println("Erro ao ler DHT22");
    sensorData.isValid = false;
    return;
  }
  
  sensorData.temperature = temp;
  sensorData.humidity = humidity;
  sensorData.light = lightRaw;
  sensorData.isValid = true;
}

void runLocalAutomation() {
  if (!sensorData.isValid) return;
  
  bool shouldActivate = false;
  
  if (sensorData.temperature > thresholds.temperature_max) {
    shouldActivate = true;
  }
  
  if (sensorData.humidity < thresholds.humidity_min) {
    shouldActivate = true;
  }
  
  if (sensorData.light < thresholds.light_min) {
    shouldActivate = true;
  }
  
  if (shouldActivate && !automationActive) {
    digitalWrite(LED_PIN, HIGH);
    automationActive = true;
    Serial.println("AUTOMAÇÃO ATIVADA - LED LIGADO");
  } else if (!shouldActivate && automationActive) {
    digitalWrite(LED_PIN, LOW);
    automationActive = false;
    Serial.println("Condições normais - LED desligado");
  }
}

void sendDataToAPI() {
  if (!sensorData.isValid) return;
  
  http.begin(String(apiServer) + "/api/sensor-data");
  http.addHeader("Content-Type", "application/json");
  
  StaticJsonDocument<200> doc;
  doc["device_id"] = "esp32_001";
  doc["temperature"] = sensorData.temperature;
  doc["humidity"] = sensorData.humidity;
  doc["light"] = sensorData.light;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode == 200) {
    Serial.println("Dados enviados para API");
  } else {
    Serial.printf("Erro ao enviar dados: %d\n", httpResponseCode);
  }
  
  http.end();
}

void fetchThresholds() {
  http.begin(String(apiServer) + "/api/thresholds");
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      thresholds.temperature_max = doc["temperature_max"];
      thresholds.humidity_min = doc["humidity_min"];
      thresholds.light_min = doc["light_min"];
      
      Serial.println("Thresholds atualizados da API");
    }
  } else {
    Serial.printf("API indisponível, usando valores locais\n");
  }
  
  http.end();
}