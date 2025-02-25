#define RX_PIN 16
#define TX_PIN 17

HardwareSerial LD1115H_Serial(2);  // Use UART2 on ESP32

void setup() {
  Serial.begin(115200);  // Monitor output
  LD1115H_Serial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);  // Sensor UART

  Serial.println("LD1115H Sensor Test - Waiting for data...");
}

void loop() {
  if (LD1115H_Serial.available()) {
    String rawData = LD1115H_Serial.readStringUntil('\n');  // Read a line from the sensor
    rawData.trim();  // Remove extra spaces/newlines

    Serial.print("Raw Data: ");
    Serial.println(rawData);
  } else {
    Serial.println("No data received...");
  }

  delay(1000);  // Wait 1 second before checking again
}
