// ========================================================
// ESP32 LD1115H Sensor Configuration Reader
// Purpose: Sends a "get_all" command to the LD1115H sensor
// and prints the response to the Serial Monitor.
// ========================================================

// === CONFIGURABLE PARAMETERS ===
#define RX_PIN 4         // ESP32 RX pin (connect to sensor TX)
#define TX_PIN 5         // ESP32 TX pin (connect to sensor RX)

const long SERIAL_BAUD_RATE = 115200;  // Baud rate for Serial Monitor
const long SENSOR_BAUD_RATE = 115200;  // Baud rate for LD1115H sensor

const char GET_COMMAND[] = "get_all\r\n";  // Command to retrieve sensor configuration (must include CRLF)
const int RESPONSE_TIMEOUT_MS = 3000;  // Time in milliseconds to wait for a response

// === HARDWARE SERIAL INSTANCE ===
HardwareSerial LD1115H_Serial(2);  // Use UART2 on ESP32

void setup() {
  // Initialize Serial Monitor
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial);  // Wait for Serial (optional for some boards)

  // Initialize UART2 for LD1115H sensor communication
  LD1115H_Serial.begin(SENSOR_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);

  // Print startup message
  Serial.println("\n===============================");
  Serial.println("LD1115H Sensor Configuration");
  Serial.print("Listening on RX: "); Serial.print(RX_PIN);
  Serial.print(", TX: "); Serial.println(TX_PIN);
  Serial.print("Baud Rate: "); Serial.println(SENSOR_BAUD_RATE);
  Serial.println("Requesting configuration...");
  Serial.println("===============================");

  // Send "get_all" command to retrieve sensor configuration
  LD1115H_Serial.print(GET_COMMAND);
  Serial.print("[Sent] "); Serial.println(GET_COMMAND);

  // Wait for sensor response
  unsigned long startTime = millis();
  bool responseReceived = false;

  while (millis() - startTime < RESPONSE_TIMEOUT_MS) {
    if (LD1115H_Serial.available()) {
      String response = LD1115H_Serial.readStringUntil('\n');
      response.trim();

      // Print received configuration data
      Serial.print("[Response] ");
      Serial.println(response);
      responseReceived = true;
    }
  }

  if (!responseReceived) {
    Serial.println("[ERROR] No response received. Check wiring and sensor readiness.");
  }

  Serial.println("===============================");
}

void loop() {
  // Nothing in loop since we only request config once in setup()
}
