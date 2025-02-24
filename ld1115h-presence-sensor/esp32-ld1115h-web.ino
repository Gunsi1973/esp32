// Name: ESP32 LD1115H Sensor Web Monitor
// Description: Monitor the LD1115H sensor data on a web page with live updates and a chart.

const char* ssid = "XXX";
const char* password = "XXX";

#include <WiFi.h>
#include <WebServer.h>

#define RX_PIN 4
#define TX_PIN 5

HardwareSerial LD1115H_Serial(2);

WebServer server(80);

String sensorData = "Waiting for data...";
unsigned long last_movement_time = 0;
unsigned long clear_time = 2000;  // 2 Sekunden bis "No movement detected" gesetzt wird

// Function to interpret sensor mode
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

void setup() {
  Serial.begin(115200);
  LD1115H_Serial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Web routes
  server.on("/", []() {
    server.send(200, "text/html", getWebPage());
  });

  server.on("/data", []() {
    server.send(200, "text/plain", sensorData);
  });

  server.begin();
}

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

  // If no movement for `clear_time`, set "No movement detected"
  if (millis() - last_movement_time > clear_time) {
    sensorData = "🔻 No movement detected.";
  }

  server.handleClient();
}

// HTML + JavaScript for live updates & chart
String getWebPage() {
  return R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 Sensor Monitor</title>
      <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

        <style>
          body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }
          h1 { color: #007BFF; }
          #data { font-size: 1.5em; color: #333; margin-top: 20px; }
          #chart-container { width: 100%; height: 60vh; }
          canvas { width: 100% !important; height: 100% !important; }
          #controls { margin-top: 10px; }
          input[type="number"] { width: 60px; padding: 5px; text-align: center; }
        </style>

        <div id="controls">
          <label for="dataPoints">Data Points:</label>
          <input type="number" id="dataPoints" min="10" max="200" value="75">
        </div>

        <script>
          let maxDataPoints = 75;  // Default number of visible data points
          let dataPoints = [];
          let labels = [];
          let chart;

          document.getElementById("dataPoints").addEventListener("change", function() {
            maxDataPoints = parseInt(this.value);
          });

          function updateData() {
            fetch('/data')
              .then(response => response.text())
              .then(data => {
                document.getElementById('data').innerText = data;

                let match = data.match(/Signal: (\d+)/);
                if (match) {
                  let signal = parseInt(match[1]);

                  // Adjust based on user-defined maxDataPoints
                  while (dataPoints.length >= maxDataPoints) {
                    dataPoints.shift();
                    labels.shift();
                  }

                  dataPoints.push(signal);
                  labels.push(new Date().toLocaleTimeString().split(" ")[0]);

                  chart.data.labels = labels;
                  chart.data.datasets[0].data = dataPoints;
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
                }
              }
            });

            setInterval(updateData, 1000);
          };
        </script>

        <div id="chart-container">
          <canvas id="chart"></canvas>
        </div>
    </head>
    <body>
      <h1>ESP32 Sensor Monitor</h1>
      <p>Live sensor data:</p>
      <p id="data">Waiting for data...</p>
      <canvas id="chart"></canvas>
    </body>
    </html>
  )rawliteral";
}
