#include <WiFi.h>
#include <WebServer.h>

// ---------------------------------------------------
// 🔴 WI-FI CREDENTIALS
// ---------------------------------------------------
const char* ssid = "iPhone";        
const char* password = "12345678"; 

WebServer server(80);

// ---------------------------------------------------
// 🔴 PIN DEFINITIONS (RAW GPIO for ESP32S3 Dev Module)
// ---------------------------------------------------
// --- SLOT 1 ---
const int trig1 = 21;  // Was D10
const int echo1 = 38;  // Was D11
const int ir1_Left  = 18;  // Was D9
const int ir1_Right = 47;  // Was D12

// --- SLOT 2 ---
const int trig2 = 9;   // Was D6
const int echo2 = 10;  // Was D7
const int ir2_Left  = 17;  // Was D8
const int ir2_Right = 8;   // Was D5

// ---------------------------------------------------
// 🟢 HELPER FUNCTIONS
// ---------------------------------------------------

// 1. Read Distance Sensor Safely
long getSafeDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  // Timeout set to ~23ms to prevent freezing
  long duration = pulseIn(echo, HIGH, 23200); 
  
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

// 2. Determine Slot Status
String getSlotStatus(long distance, int irLeft, int irRight) {
  if (irLeft == LOW || irRight == LOW) return "MISALIGNED";
  else if (distance > 0 && distance < 10) return "OCCUPIED";
  else return "FREE";
}

// 3. Handle Website Requests
void handleData() {
  long d1 = getSafeDistance(trig1, echo1);
  String s1 = getSlotStatus(d1, digitalRead(ir1_Left), digitalRead(ir1_Right));
  
  long d2 = getSafeDistance(trig2, echo2);
  String s2 = getSlotStatus(d2, digitalRead(ir2_Left), digitalRead(ir2_Right));

  String json = "{\"s1\": \"" + s1 + "\", \"s2\": \"" + s2 + "\", \"s3\": \"FREE\"}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

// ---------------------------------------------------
// 🔵 MAIN SETUP
// ---------------------------------------------------
void setup() {
  // Initialize Serial
  Serial.begin(115200);

  // Setup Pins
  pinMode(trig1, OUTPUT); pinMode(echo1, INPUT);
  pinMode(ir1_Left, INPUT_PULLUP); pinMode(ir1_Right, INPUT_PULLUP);

  pinMode(trig2, OUTPUT); pinMode(echo2, INPUT);
  pinMode(ir2_Left, INPUT_PULLUP); pinMode(ir2_Right, INPUT_PULLUP);

  Serial.println("\n\n--------------------------------");
  Serial.println("✅ PARK MATE STARTING...");

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retry++;
    if (retry > 20) {
      Serial.println("\n⚠️ WiFi Connection Failed! Check Password.");
      break; 
    }
  }

  Serial.println("\n✅ WIFI CONNECTED!");
  Serial.print("➡️ IP ADDRESS: http://");
  Serial.println(WiFi.localIP()); 
  Serial.println("--------------------------------\n");

  server.on("/data", handleData); 
  server.begin();
}

void loop() {
  server.handleClient(); 
}
