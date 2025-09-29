const express = require('express');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'frontend')));

// Dados em memória
let sensorData = [];
let thresholds = {
  temperature_max: 30.0,
  humidity_min: 40.0,
  light_min: 500
};

// Endpoint para receber dados dos sensores (POST)
app.post('/api/sensor-data', (req, res) => {
  try {
    const { temperature, humidity, light, device_id } = req.body;
    
    const newData = {
      id: Date.now(),
      device_id: device_id || 'esp32_001',
      temperature: parseFloat(temperature),
      humidity: parseFloat(humidity),
      light: parseInt(light),
      timestamp: new Date().toISOString(),
      alert: temperature > thresholds.temperature_max || 
             humidity < thresholds.humidity_min || 
             light < thresholds.light_min
    };
    
    sensorData.push(newData);
    if (sensorData.length > 100) {
      sensorData = sensorData.slice(-100);
    }
    
    console.log('Dados recebidos:', newData);
    res.json({ success: true, data: newData });
    
  } catch (error) {
    res.status(400).json({ success: false, error: error.message });
  }
});

// Endpoint para buscar thresholds (GET)
app.get('/api/thresholds', (req, res) => {
  res.json(thresholds);
});

// Endpoint para atualizar thresholds (PUT)
app.put('/api/thresholds', (req, res) => {
  const { temperature_max, humidity_min, light_min } = req.body;
  
  if (temperature_max !== undefined) thresholds.temperature_max = parseFloat(temperature_max);
  if (humidity_min !== undefined) thresholds.humidity_min = parseFloat(humidity_min);
  if (light_min !== undefined) thresholds.light_min = parseInt(light_min);
  
  res.json({ success: true, thresholds });
});

// Endpoint para dashboard
app.get('/api/dashboard', (req, res) => {
  const latest = sensorData[sensorData.length - 1];
  
  res.json({
    latest: latest || null,
    thresholds,
    total_readings: sensorData.length
  });
});

// Servir o dashboard
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'frontend', 'dashboard.html'));
});

// Iniciar servidor
app.listen(PORT, () => {
  console.log(`API rodando em http://localhost:${PORT}`);
  console.log(`Dashboard em http://localhost:${PORT}`);
});