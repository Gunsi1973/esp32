// ================================================================
// ESP32 LD1115H Sensor Web Monitor
// ================================================================
// Purpose: This script reads movement data from the LD1115H sensor
// and provides a web-based dashboard with live updates and a chart.
// ================================================================

// Includes
#include <WiFi.h>
#include <WebServer.h>

// === CONFIGURABLE PARAMETERS ===
const char* WIFI_SSID = "xxx";                   // WiFi SSID
const char* WIFI_PASSWORD = "xxx";     // WiFi Password

const int RX_PIN = 4;  // ESP32 UART RX (connect to sensor TX)
const int TX_PIN = 5;  // ESP32 UART TX (connect to sensor RX)
const long SERIAL_BAUD_RATE = 115200;  // Serial Monitor baud rate
const long SENSOR_BAUD_RATE = 115200;  // LD1115H sensor baud rate
const int SERVER_PORT = 80;  // Web server port
const unsigned long CLEAR_TIME_MS = 2000;  // Time before "No movement detected" (in milliseconds)

// === GLOBAL VARIABLES ===
HardwareSerial LD1115H_Serial(2);  // UART2 for the sensor
WebServer server(SERVER_PORT);  // Web server instance

String sensorData = "Waiting for data...";  // Stores sensor readings
unsigned long last_movement_time = 0;  // Tracks last detected movement

// ================================================================
// Function: Interpret sensor mode and return readable text
// ================================================================
String interpretMode(int mode) {
  switch (mode) {
    case 1: return "Very light movement";
    case 3: return "Small movement";
    case 5: return "Stronger movement";
    case 6: return "Calm presence";
    case 7: return "Presence remains stable";
    case 8: return "Clear presence";
    case 9: return "Very strong presence";
    default: return "Unknown Mode";
  }
}

// ================================================================
// Function: Setup ESP32 - Initialize Serial, WiFi, and Web Server
// ================================================================
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  LD1115H_Serial.begin(SENSOR_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

  // Connecting to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup Web Routes
  server.on("/", []() { server.send(200, "text/html", getWebPage()); });
  server.on("/data", []() { server.send(200, "text/plain", sensorData); });

  server.begin();
}

// ================================================================
// Function: Main Loop - Read Sensor Data and Serve Web Requests
// ================================================================
void loop() {
  if (LD1115H_Serial.available()) {
    String rawData = LD1115H_Serial.readStringUntil('\n');
    rawData.trim();

    int firstComma = rawData.indexOf(',');
    int spaceAfterMode = rawData.indexOf(' ', firstComma + 2);

    if (firstComma != -1 && spaceAfterMode != -1) {
      String status = rawData.substring(0, firstComma);
      String modeStr = rawData.substring(firstComma + 2, spaceAfterMode);
      String value = rawData.substring(spaceAfterMode + 1);
      int signalStrength = value.toInt();
      int mode = modeStr.toInt();

      String modeDescription = interpretMode(mode);

      if (status == "mov") {
        sensorData = "🔵 Movement detected! (Mode: " + modeDescription + " | Signal: " + String(signalStrength) + ")";
        last_movement_time = millis();
      } else if (status == "occ") {
        sensorData = "🟢 Presence detected! (Mode: " + modeDescription + " | Signal: " + String(signalStrength) + ")";
        last_movement_time = millis();
      }
    }
  }

  // Reset to "No movement detected" if no movement for defined time
  if (millis() - last_movement_time > CLEAR_TIME_MS) {
    sensorData = "🔻 No movement detected.";
  }

  server.handleClient();
}

// ================================================================
// Function: HTML + JavaScript Web Interface for Sensor Dashboard
// ================================================================
String getWebPage() {
  return R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Sensor Monitor</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-zoom@1.2.1"></script>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }
    h1 { color: #007BFF; }
    #data { font-size: 1.5em; color: #333; margin-top: 20px; }
    #chart-container { width: 100%; height: 60vh; }
    canvas { width: 100% !important; height: 100% !important; }
    #controls { margin-top: 10px; }
    input[type="number"] { width: 60px; padding: 5px; text-align: center; }
  </style>
</head>
<body>
  <h1>ESP32 Sensor Monitor</h1>
  <p>Live sensor data:</p>
  <p id="data">Waiting for data...</p>

  <div id="controls">
    <label for="dataPoints">Data Points:</label>
    <input type="number" id="dataPoints" min="10" max="200" value="75">
    <button id="downloadCsv">Download CSV</button>
  </div>

  <div id="chart-container">
    <canvas id="chart"></canvas>
  </div>

  <script>
    let maxDataPoints = 75;  // Live display limit
    let maxStoragePoints = 21600;  // 6 hours of storage
    let dataPoints = [];
    let labels = [];
    let storedData = [];

    document.getElementById("dataPoints").addEventListener("change", function() {
      maxDataPoints = parseInt(this.value);
    });

    document.getElementById('downloadCsv').addEventListener('click', function() {
      let csvContent = "data:text/csv;charset=utf-8,Time,Signal Strength\n";
      storedData.forEach(entry => {
        csvContent += `${entry.time},${entry.value}\n`;
      });
      const encodedUri = encodeURI(csvContent);
      const link = document.createElement("a");
      link.setAttribute("href", encodedUri);
      link.setAttribute("download", "sensor_data_6h.csv");
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
    });

    function getColor(signal) {
      if (signal < 5000) return 'green';
      if (signal < 15000) return 'yellow';
      return 'red';
    }

    function updateData() {
      fetch('/data')
        .then(response => response.text())
        .then(data => {
          document.getElementById('data').innerText = data;

          let match = data.match(/Signal: (\d+)/);
          if (match) {
            let signal = parseInt(match[1]);
            let timestamp = new Date().toLocaleTimeString().split(" ")[0];

            // Manage live data (for graph display)
            while (dataPoints.length >= maxDataPoints) {
              dataPoints.shift();
              labels.shift();
            }
            dataPoints.push(signal);
            labels.push(timestamp);

            // Store data for CSV (only last 6 hours)
            if (storedData.length >= maxStoragePoints) {
              storedData.shift();  // Remove oldest entry
            }
            storedData.push({ time: timestamp, value: signal });

            // Update chart color
            chart.data.labels = labels;
            chart.data.datasets[0].data = dataPoints;
            chart.data.datasets[0].borderColor = getColor(signal);
            chart.data.datasets[0].backgroundColor = getColor(signal) + '33';
            chart.update();
          }
        })
        .catch(error => console.error("Error fetching data:", error));
    }

    window.onload = function() {
      let ctx = document.getElementById('chart').getContext('2d');
      chart = new Chart(ctx, {
        type: 'line',
        data: {
          labels: [],
          datasets: [{
            label: 'Signal Strength',
            data: [],
            borderColor: 'blue',
            backgroundColor: 'rgba(0, 123, 255, 0.2)',
            fill: true,
            tension: 0.1
          }]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          scales: {
            x: { display: true },
            y: { 
              beginAtZero: true,
              suggestedMax: function() { return Math.max(...dataPoints, 1000) + 1000; } 
            }
          },
          plugins: {
            zoom: {
              pan: {
                enabled: true,
                mode: 'xy'
              },
              zoom: {
                enabled: true,
                mode: 'xy',
                speed: 0.1,
                limits: {
                  x: { min: 10, max: 21600 },
                  y: { min: 0 }
                }
              }
            }
          }
        }
      });
      setInterval(updateData, 1000);
    };
  </script>
</body>
</html>


)rawliteral";
}
