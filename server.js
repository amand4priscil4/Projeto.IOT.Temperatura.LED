const express = require('express');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

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

// Endpoint para buscar dados históricos (GET)
app.get('/api/sensor-data', (req, res) => {
  const limit = parseInt(req.query.limit) || 50;
  const recentData = sensorData.slice(-limit).reverse();
  
  res.json({
    success: true,
    data: recentData,
    total: sensorData.length
  });
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
  
  console.log('Thresholds atualizados:', thresholds);
  res.json({ success: true, thresholds });
});

// Endpoint para dashboard (dados resumidos)
app.get('/api/dashboard', (req, res) => {
  const latest = sensorData[sensorData.length - 1];
  const last24h = sensorData.filter(d => {
    const dataTime = new Date(d.timestamp);
    const now = new Date();
    return (now - dataTime) <= 24 * 60 * 60 * 1000;
  });
  
  const stats = {
    latest: latest || null,
    thresholds,
    stats_24h: {
      total_readings: last24h.length,
      alerts: last24h.filter(d => d.alert).length,
      avg_temperature: last24h.length > 0 ? 
        (last24h.reduce((sum, d) => sum + d.temperature, 0) / last24h.length).toFixed(1) : 0,
      avg_humidity: last24h.length > 0 ? 
        (last24h.reduce((sum, d) => sum + d.humidity, 0) / last24h.length).toFixed(1) : 0,
      avg_light: last24h.length > 0 ? 
        Math.round(last24h.reduce((sum, d) => sum + d.light, 0) / last24h.length) : 0
    }
  };
  
  res.json(stats);
});

// Endpoint de status
app.get('/api/status', (req, res) => {
  res.json({
    status: 'online',
    timestamp: new Date().toISOString(),
    total_readings: sensorData.length
  });
});

// Servir o dashboard na raiz
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'dashboard.html'));
});

// Iniciar servidor
app.listen(PORT, () => {
  console.log(`API rodando em http://localhost:${PORT}`);
  console.log(`Dashboard em http://localhost:${PORT}`);
});

// Export para Vercel
module.exports = app;
