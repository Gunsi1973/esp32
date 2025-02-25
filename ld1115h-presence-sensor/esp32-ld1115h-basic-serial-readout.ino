// ========================================================
// ESP32 LD1115H UART Debugging Script
// Purpose: Reads and prints raw data from the LD1115H sensor 
// to the Serial Monitor for troubleshooting.
// ========================================================

#define RX_PIN 4  // ESP32 RX pin (connect to sensor TX)
#define TX_PIN 5  // ESP32 TX pin (connect to sensor RX)

HardwareSerial LD1115H_Serial(2);  // Use UART2 on ESP32

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);  
  while (!Serial);  // Wait for Serial to be ready (optional, useful for some boards)

  // Initialize UART2 for LD1115H sensor communication
  LD1115H_Serial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  // Print startup message
  Serial.println("\n===============================");
  Serial.println("LD1115H Sensor Debugging");
  Serial.println("Waiting for data...");
  Serial.println("===============================");
}

void loop() {
  // Check if data is available from the sensor
  if (LD1115H_Serial.available()) {
    String rawData = LD1115H_Serial.readStringUntil('\n');  // Read sensor data until newline
    rawData.trim();  // Remove leading/trailing spaces and newlines

    // Print received data to Serial Monitor
    Serial.print("[Received] ");
    Serial.println(rawData);
  } else {
    Serial.println("[INFO] No data received...");
  }

  delay(1000);  // Wait 1 second before checking again
}
