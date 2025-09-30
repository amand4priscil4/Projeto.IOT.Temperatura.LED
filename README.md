# Sistema de Automação IoT - ESP32

Protótipo de automação IoT usando ESP32 com sensores DHT22 (temperatura/umidade) e LDR (luminosidade), integrando regras de automação locais e em nuvem com arquitetura de microserviços.

---

## Links do Projeto

- **Simulação Wokwi:** https://wokwi.com/projects/443357177542960129
- **Dashboard Online:** https://projeto-iot-temperatura-led.vercel.app/
- **Repositório GitHub:** https://github.com/amand4priscil4/Projeto.IOT.Temperatura.LED

---

## Equipe

- **Amanda Priscila Alves**
- **Ana Beatriz Gomes da Silva**
- **Lucas da Silva**
- **Kellvyn Oliveira**

---

## Objetivo do Projeto

Desenvolver um sistema de automação IoT que demonstra quando faz sentido tomar decisões localmente (no dispositivo) versus centralizadamente (na nuvem), implementando:

1. **Regras locais:** Automação executada diretamente no ESP32
2. **Integração com ThingSpeak:** Coleta e armazenamento de dados em nuvem
3. **Arquitetura de microserviços:** Backend separado do Frontend
4. **Persistência de dados:** MongoDB para histórico de leituras
5. **Fail-safe:** Sistema continua funcionando mesmo sem conexão com a internet
6. **Dashboard:** Interface web para visualização e controle remoto

---

## Arquitetura do Sistema

```
┌─────────────┐
│   ESP32     │ (Sensores DHT22 + LDR)
└──────┬──────┘
       │
       ├─────────────────────────────────┐
       │                                 │
       ▼                                 ▼
┌─────────────┐                  ┌──────────────┐
│ ThingSpeak  │                  │ API Frontend │
│   (Cloud)   │                  │  (Porta 3000)│
└──────┬──────┘                  └──────┬───────┘
       │                                 │
       ▼                                 │
┌─────────────────────┐                 │
│   API Backend       │◄────────────────┘
│ Node.js + MongoDB   │
│    (Porta 4000)     │
└─────────────────────┘
```

### Fluxo de Dados

1. **ESP32 → ThingSpeak**: Envia leituras dos sensores (temperatura, umidade, luminosidade)
2. **Backend → ThingSpeak**: Importa dados a cada 1 minuto e armazena no MongoDB
3. **Frontend → Backend**: Busca dados históricos e estatísticas
4. **Dashboard → Usuário**: Exibe dados em tempo real com interface web

### Componentes

**Hardware:**
- ESP32-C3 (microcontrolador com WiFi)
- DHT22 (sensor de temperatura e umidade)
- LDR (sensor de luminosidade)
- LED vermelho (indicador de alerta)
- Resistores (220Ω para LED, 10kΩ para LDR)

**Software:**
- **Firmware:** C++ (Arduino IDE)
- **Backend API:** Node.js + Express + MongoDB + Mongoose
- **Frontend API:** Node.js + Express
- **Dashboard:** HTML + CSS + JavaScript
- **Deploy:** Vercel (cloud hosting)

---

## Como Foi Feito

### 1. Prototipagem do Circuito

Iniciamos desenhando o circuito no Wokwi, um simulador online de Arduino/ESP32:

**Conexões:**
- DHT22 Data → GPIO22
- LDR Analog → GPIO34
- LED → GPIO2 (através de resistor 220Ω)
- VCC → 3V3
- GND → GND

O circuito foi testado virtualmente antes da montagem física, permitindo validar a lógica de automação sem hardware.

### 2. Desenvolvimento do Firmware (ESP32)

Criamos o código do ESP32 com três modos de operação:

**Modo Local (Edge Computing):**
```cpp
if (temperatura > limite_max || 
    umidade < limite_min || 
    luz < limite_min) {
    digitalWrite(LED_PIN, HIGH); // Liga LED
}
```

**Integração com ThingSpeak:**
```cpp
// Envia dados via HTTP POST
String url = "https://api.thingspeak.com/update?api_key=";
url += apiKey;
url += "&field1=" + String(temperatura);
url += "&field2=" + String(umidade);
url += "&field3=" + String(luminosidade);
```

**Fail-Safe:**
```cpp
if (!thingSpeakDisponivel) {
    // Continua operando com valores padrão locais
    temperatura_max = 30.0;
    umidade_min = 40.0;
    luz_min = 500;
}
```

### 3. Backend API (Porta 4000)

Desenvolvemos uma API em Node.js que integra ThingSpeak com MongoDB:

**Funcionalidades:**
- Importa dados do ThingSpeak automaticamente a cada 1 minuto
- Armazena histórico no MongoDB (evita duplicatas)
- Expõe endpoints REST para consulta de dados
- Calcula estatísticas agregadas (médias, máximos, mínimos)

**Endpoints:**

**GET /leituras?limit=50**
- Retorna últimas N leituras do banco
- Ordenadas da mais recente para a mais antiga
- Exemplo: `http://localhost:4000/leituras?limit=100`

**GET /ultima**
- Retorna a leitura mais recente
- Útil para exibir dados em tempo real

**GET /estatisticas**
- Retorna estatísticas das últimas 24 horas
- Médias de temperatura, umidade e luminosidade
- Valores máximos e mínimos

**GET /status**
- Verifica saúde da API e conexão MongoDB
- Retorna total de leituras armazenadas

**Tecnologias:**
- Express.js (framework web)
- Mongoose (ODM para MongoDB)
- Axios (requisições HTTP)
- CORS (comunicação entre APIs)

### 4. Frontend API (Porta 3000)

API intermediária que serve o dashboard e processa lógica de negócio:

**Responsabilidades:**
- Servir arquivos estáticos (HTML, CSS, JS)
- Buscar dados do Backend (porta 4000)
- Gerenciar thresholds (limites de alerta)
- Formatar dados para o dashboard
- Calcular alertas baseados nos limites configurados

**Endpoints:**

**GET /api/sensor-data?limit=50**
- Busca dados do backend MongoDB
- Formata para o padrão esperado pelo dashboard
- Adiciona campo `alert` baseado nos thresholds

**GET /api/dashboard**
- Retorna dados resumidos para exibição
- Última leitura + estatísticas 24h
- Inclui thresholds configurados

**GET /api/thresholds**
- Retorna limites configurados
- Temperatura máxima, umidade mínima, luz mínima

**PUT /api/thresholds**
- Atualiza limites remotamente
- Permite ajuste fino sem reprogramar o dispositivo

**GET /api/status**
- Verifica saúde do frontend e backend
- Útil para diagnóstico de problemas

### 5. Dashboard Web

Interface web responsiva com atualização em tempo real:

**Funcionalidades:**
- Exibição de temperatura, umidade e luminosidade atual
- Alertas visuais quando valores excedem limites
- Configuração remota de thresholds
- Estatísticas das últimas 24 horas
- Atualização automática a cada 5 segundos
- Design responsivo (mobile-friendly)

**Tecnologias:**
- HTML5 semântico
- CSS3 com flexbox/grid
- JavaScript ES6+ (fetch API)
- Design responsivo

### 6. Integração com ThingSpeak

**Configuração do Canal:**
- Field 1: Temperatura (°C)
- Field 2: Umidade (%)
- Field 3: Luminosidade (lux)

**Processo de Importação:**
1. Backend faz requisição GET para ThingSpeak a cada 1 minuto
2. Recebe JSON com até 100 últimas leituras
3. Verifica duplicatas usando `created_at` único
4. Insere novos dados no MongoDB
5. Registra estatísticas no console

**Vantagens:**
- Histórico persistente (não se perde)
- Backup automático em nuvem
- Análise de dados de longo prazo
- Escalabilidade

### 7. Persistência com MongoDB

**Schema do Banco:**
```javascript
{
  created_at: Date,        // Timestamp (único)
  device_id: String,       // Identificador do dispositivo
  temperatura: Number,     // Temperatura em °C
  umidade: Number,         // Umidade em %
  iluminacao: Number,      // Luminosidade em lux
  timestamps: true         // createdAt, updatedAt automáticos
}
```

**Recursos:**
- Índice único em `created_at` (evita duplicatas)
- Validação automática de tipos
- Queries otimizadas com agregações
- Escalável para milhões de registros

### 8. Deploy

**Opções de Deploy:**

**Desenvolvimento Local:**
- Backend: `http://localhost:4000`
- Frontend: `http://localhost:3000`
- MongoDB: `mongodb://localhost:27017`

**Produção (Vercel):**
- Frontend hospedado no Vercel
- Backend pode ser hospedado em:
  - Railway (suporta MongoDB)
  - Render (free tier)
  - Heroku
  - DigitalOcean

**MongoDB em Produção:**
- MongoDB Atlas (free tier 512MB)
- Conexão via string de conexão segura
- Backup automático

---

## Decisões de Arquitetura

### Por que Duas APIs Separadas?

**Separação de Responsabilidades:**
- **Backend (4000)**: Foco em dados e integração com ThingSpeak/MongoDB
- **Frontend (3000)**: Foco em lógica de apresentação e servir dashboard

**Vantagens:**
- Escalabilidade independente
- Manutenção mais simples
- Possibilidade de múltiplos frontends
- Melhor organização do código
- Deploy independente

### Por que MongoDB?

**Vantagens sobre memória:**
- Dados persistem após reinicialização
- Escalável para milhões de registros
- Queries complexas e agregações
- Backup e recuperação
- Suporte a índices

**Casos de uso:**
- Análise histórica de tendências
- Relatórios de longo prazo
- Machine learning sobre dados históricos

### Por que ThingSpeak?

**Vantagens:**
- API simples e confiável
- Visualizações prontas
- MQTT disponível
- Free tier generoso
- Backup em nuvem

### Estratégia Híbrida (Nossa Escolha)

Implementamos o melhor dos três mundos:
- **Autonomia:** Decisões críticas locais no ESP32
- **Persistência:** MongoDB para histórico confiável
- **Backup:** ThingSpeak como fonte secundária
- **Flexibilidade:** Thresholds ajustáveis remotamente
- **Escalabilidade:** Arquitetura de microserviços

---

## Estrutura do Projeto

```
Projeto.IOT.Temperatura.LED/
├── backend/
│
