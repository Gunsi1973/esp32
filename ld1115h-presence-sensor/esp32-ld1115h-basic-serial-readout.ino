// ========================================================
// ESP32 LD1115H UART Debugging Script
// Purpose: Reads and prints raw data from the LD1115H sensor 
// to the Serial Monitor for troubleshooting.
// ========================================================

// === CONFIGURABLE PARAMETERS ===
#define RX_PIN 4         // ESP32 RX pin (connect to sensor TX)
#define TX_PIN 5         // ESP32 TX pin (connect to sensor RX)

const long SERIAL_BAUD_RATE = 115200;  // Baud rate for Serial Monitor
const long SENSOR_BAUD_RATE = 115200;  // Baud rate for LD1115H sensor

const int READ_INTERVAL_MS = 1000;  // Time in milliseconds between sensor reads

// === HARDWARE SERIAL INSTANCE ===
HardwareSerial LD1115H_Serial(2);  // Use UART2 on ESP32

void setup() {
  // Initialize Serial Monitor
  Serial.begin(SERIAL_BAUD_RATE);  
  while (!Serial);  // Wait for Serial to be ready (optional for some boards)

  // Initialize UART2 for LD1115H sensor communication
  LD1115H_Serial.begin(SENSOR_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

  // Print startup message
  Serial.println("\n===============================");
  Serial.println("LD1115H Sensor Debugging");
  Serial.print("Listening on RX: "); Serial.print(RX_PIN);
  Serial.print(", TX: "); Serial.println(TX_PIN);
  Serial.print("Baud Rate: "); Serial.println(SENSOR_BAUD_RATE);
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

  delay(READ_INTERVAL_MS);  // Wait before the next read
}
