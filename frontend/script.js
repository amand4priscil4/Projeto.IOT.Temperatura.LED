// ======================
// Configuração da API
// ======================
const API_BASE = 'https://backend-ino.onrender.com';

// Conectar ao backend via WebSocket (Socket.IO)
const socket = io(API_BASE);

// Variáveis globais
let sensorData = [];

// ======================
// Inicialização
// ======================
document.addEventListener('DOMContentLoaded', function() {
    loadThresholds(); // thresholds iniciais ainda via fetch
});

// ======================
// Eventos de WebSocket
// ======================

// Receber novos dados de sensores em tempo real
socket.on("newData", (data) => {
    updateSensorValues(data);
    updateStatus(true);
});

// Receber thresholds do servidor
socket.on("thresholds", (thresholds) => {
    document.getElementById('tempInput').value = thresholds.temperature_max;
    document.getElementById('humidityInput').value = thresholds.humidity_min;
    document.getElementById('lightInput').value = thresholds.light_min;

    document.getElementById('tempThreshold').textContent = thresholds.temperature_max;
    document.getElementById('humidityThreshold').textContent = thresholds.humidity_min;
    document.getElementById('lightThreshold').textContent = thresholds.light_min;
});

// Se houver erro de conexão
socket.on("connect_error", () => updateStatus(false));

// ======================
// Funções principais
// ======================

// Carregar dados do dashboard (fallback/manual)
async function loadDashboard() {
    try {
        const response = await fetch(`${API_BASE}/dashboard`);
        const data = await response.json();
        
        updateSensorValues(data);
        updateStatus(true);
        
    } catch (error) {
        console.error('Erro ao carregar dashboard:', error);
        updateStatus(false);
    }
}

// Atualizar valores dos sensores
function updateSensorValues(data) {
    if (data.latest) {
        const latest = data.latest;
        
        document.getElementById('tempValue').textContent = latest.temperature.toFixed(1);
        document.getElementById('humidityValue').textContent = latest.humidity.toFixed(1);
        document.getElementById('lightValue').textContent = latest.light;
        
        // Mostrar/esconder alertas
        toggleAlert('tempAlert', latest.temperature > data.thresholds.temperature_max);
        toggleAlert('humidityAlert', latest.humidity < data.thresholds.humidity_min);
        toggleAlert('lightAlert', latest.light < data.thresholds.light_min);
        
        document.getElementById('lastUpdate').textContent = 
            new Date(latest.timestamp).toLocaleString('pt-BR');
    }
}

// Alternar alertas
function toggleAlert(elementId, show) {
    const element = document.getElementById(elementId);
    element.style.display = show ? 'block' : 'none';
    if (show) {
        element.classList.add('danger');
    }
}

// Carregar thresholds atuais
async function loadThresholds() {
    try {
        const response = await fetch(`${API_BASE}/thresholds`);
        const thresholds = await response.json();
        
        document.getElementById('tempInput').value = thresholds.temperature_max;
        document.getElementById('humidityInput').value = thresholds.humidity_min;
        document.getElementById('lightInput').value = thresholds.light_min;
        
        document.getElementById('tempThreshold').textContent = thresholds.temperature_max;
        document.getElementById('humidityThreshold').textContent = thresholds.humidity_min;
        document.getElementById('lightThreshold').textContent = thresholds.light_min;
        
    } catch (error) {
        console.error('Erro ao carregar thresholds:', error);
    }
}

// Atualizar thresholds no backend
async function updateThresholds() {
    const newThresholds = {
        temperature_max: parseFloat(document.getElementById('tempInput').value),
        humidity_min: parseInt(document.getElementById('humidityInput').value),
        light_min: parseInt(document.getElementById('lightInput').value)
    };
    
    try {
        const response = await fetch(`${API_BASE}/thresholds`, {
            method: 'PUT',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(newThresholds)
        });
        
        if (response.ok) {
            alert('Limites atualizados com sucesso!');
            loadThresholds();
        } else {
            alert('Erro ao atualizar limites!');
        }
        
    } catch (error) {
        console.error('Erro ao atualizar thresholds:', error);
        alert('Erro de conexão!');
    }
}

// Atualizar status da conexão
function updateStatus(online) {
    const indicator = document.getElementById('statusIndicator');
    if (online) {
        indicator.textContent = 'Online';
        indicator.className = 'status-indicator';
    } else {
        indicator.textContent = 'Offline';
        indicator.className = 'status-indicator offline';
    }
}