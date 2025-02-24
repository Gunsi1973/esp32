// change sensor sensitiviy via web-interface

const char* ssid = "xxx";
const char* password = "xxx";

#include <WiFi.h>
#include <WebServer.h>

#define RX_PIN 4
#define TX_PIN 5

HardwareSerial LD1115H_Serial(2);

WebServer server(80);

int TH1 = 1200;  // Bewegungsempfindlichkeit
int TH2 = 1800;  // Präsenzempfindlichkeit

void setup() {
  Serial.begin(115200);
  LD1115H_Serial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  server.on("/set-th", []() {
    if (server.hasArg("th1")) {
      TH1 = server.arg("th1").toInt();
      Serial.printf("New TH1: %d\n", TH1);
      LD1115H_Serial.printf("th1=%d\n", TH1);
    }
    if (server.hasArg("th2")) {
      TH2 = server.arg("th2").toInt();
      Serial.printf("New TH2: %d\n", TH2);
      LD1115H_Serial.printf("th2=%d\n", TH2);
    }
    server.send(200, "text/plain", "OK");
  });

  server.on("/", []() {
    server.send(200, "text/html", getWebPage());
  });

  server.begin();
}

void loop() {
  server.handleClient();
}

String getWebPage() {
  return R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 Sensor Control</title>
      <script>
        function updateTH1() {
          let th1 = document.getElementById("th1").value;
          document.getElementById("th1Value").innerText = th1;
          fetch(`/set-th?th1=${th1}`);
        }
        
        function updateTH2() {
          let th2 = document.getElementById("th2").value;
          document.getElementById("th2Value").innerText = th2;
          fetch(`/set-th?th2=${th2}`);
        }
      </script>
      <style>
        body { font-family: Arial, sans-serif; text-align: center; padding: 20px; }
        h1 { color: #007BFF; }
        #controls { margin-top: 10px; }
        input[type="range"] { width: 300px; }
      </style>
    </head>
    <body>
      <h1>ESP32 Sensor Control</h1>
      <div id="controls">
        <label for="th1">Movement Sensitivity (TH1):</label>
        <input type="range" id="th1" min="50" max="2500" step="50" value="1200" oninput="updateTH1()">
        <span id="th1Value">1200</span>

        <br><br>
        <label for="th2">Presence Sensitivity (TH2):</label>
        <input type="range" id="th2" min="50" max="2500" step="50" value="1800" oninput="updateTH2()">
        <span id="th2Value">1800</span>
      </div>
    </body>
    </html>
  )rawliteral";
}
