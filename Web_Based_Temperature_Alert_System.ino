/*
  ESP32 Web-Based Temperature Alert System
  - Reads temperature from DS18B20
  - Serves a web page with live readings
  - Triggers LED + buzzer alert if temp exceeds threshold
*/

#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================== USER SETTINGS ==================
const char* ssid = "Janan";
const char* password = "ObuaJanan@#5060";
const float TEMP_THRESHOLD = 37.5; // Celsius
// ====================================================

// Pin assignments
#define ONE_WIRE_BUS 15   // DS18B20 data pin
#define LED_PIN 16        // LED pin
#define BUZZER_PIN 5      // Buzzer pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiServer server(80);

float currentTemp = 0.0;

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Start DS18B20
  sensors.begin();

  // Connect to Wi-Fi
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  // Start server
  server.begin();
}

void loop() {
  // Read temperature
  sensors.requestTemperatures();
  currentTemp = sensors.getTempCByIndex(0);

  // Alert logic
  if (currentTemp >= TEMP_THRESHOLD) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 2000);
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  // Handle web requests
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    // Send HTML response
    String html = "<!DOCTYPE html><html>";
    html += "<head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<meta http-equiv='refresh' content='2'>";
    html += "<style>";
    html += "body { font-family: Arial; text-align: center; padding: 20px;";
    if (currentTemp >= TEMP_THRESHOLD) {
      html += "background-color: #ffcccc;"; // red background
    } else {
      html += "background-color: #ccffcc;"; // green background
    }
    html += "}";
    html += "h1 { font-size: 2.5em; }";
    html += "p { font-size: 1.5em; }";
    html += "</style></head><body>";
    html += "<h1>ESP32 Health Temperature Monitor</h1>";
    html += "<p>Current Temperature: <b>" + String(currentTemp, 1) + " &deg;C</b></p>";
    if (currentTemp >= TEMP_THRESHOLD) {
      html += "<p style='color:red;'>ALERT: High Temperature!</p>";
    } else {
      html += "<p style='color:green;'>Status: Normal</p>";
    }
    html += "</body></html>";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html);

    delay(1);
    client.stop();
  }
}
