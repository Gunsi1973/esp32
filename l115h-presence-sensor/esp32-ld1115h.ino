#define RX_PIN 4
#define TX_PIN 5

HardwareSerial LD1115H_Serial(2);

// Adjustable sensitivity thresholds
int TH1 = 1200;   // Motion sensitivity (higher = less sensitive)
int TH2 = 1800;   // Presence sensitivity (higher = less sensitive)
unsigned long last_movement_time = 0;  // Timestamp of last detected movement
unsigned long clear_time = 750;        // Delay before declaring "no movement" (ms)

// Function to interpret sensor mode with fallback to "Unknown Mode"
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
  Serial.println("LD1115H Sensor started...");
}

void loop() {
  if (LD1115H_Serial.available()) {
    String sensorData = LD1115H_Serial.readStringUntil('\n');  
    sensorData.trim(); 

    int firstComma = sensorData.indexOf(',');
    int spaceAfterMode = sensorData.indexOf(' ', firstComma + 2);
    
    if (firstComma != -1 && spaceAfterMode != -1) {
      String status = sensorData.substring(0, firstComma);
      String modeStr = sensorData.substring(firstComma + 2, spaceAfterMode);
      String value = sensorData.substring(spaceAfterMode + 1);
      int signalStrength = value.toInt();
      int mode = modeStr.toInt();  

      String modeDescription = interpretMode(mode);

      // Movement detected -> Save timestamp, but only if mode is valid
      if (status == "mov" && signalStrength > TH1 && modeDescription != "Unknown Mode") {
        last_movement_time = millis();
        Serial.printf("🔵 Movement detected! (Mode: %s | Signal: %d)\n", modeDescription.c_str(), signalStrength);
      } 
      // Presence detected -> Only report it if it's "logical"
      else if (status == "occ" && signalStrength > TH2 && modeDescription != "Unknown Mode") {
        // Presence should only be reported if enough time has passed since movement
        if (millis() - last_movement_time > 500) {  // 500ms cooldown after movement
          Serial.printf("🟢 Presence detected! (Mode: %s | Signal: %d)\n", modeDescription.c_str(), signalStrength);
        }
      }
    }
  }

  // Check if enough time has passed since last movement to declare "no movement"
  if (millis() - last_movement_time > clear_time) {
    Serial.println("🔻 No movement detected.");
    last_movement_time = millis() + 9999999;  // Blocks further output until new movement is detected
  }

  delay(100);
}
