#include <Wire.h>
#include "MAX30105.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ====== KONFIGURASI ======
#define SDA_PIN 23
#define SCL_PIN 22
#define BUTTON_PIN 17
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const char* ssid = "LKJ-63X";
const char* password = "komjar1963";
const char* serverIP = "10.26.112.136";   // IP server Flask
const int serverPort = 5001;

const int DURATION_SECONDS = 60;
const int SAMPLE_INTERVAL_MS = 40;  // 25 Hz
const int NUM_SAMPLES = DURATION_SECONDS * 1000 / SAMPLE_INTERVAL_MS;

// ==========================
MAX30105 particleSensor;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ====== OLED ======
void showCenteredText(String line1, String line2 = "") {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  if (line2 == "") {
    display.setCursor(20, 28);
    display.println(line1);
  } else {
    display.setCursor(25, 20);
    display.println(line1);
    display.setCursor(30, 35);
    display.println(line2);
  }
  display.display();
}

// ====== WIFI ======
void connectWiFi() {
  Serial.printf("[WiFi] Connecting to %s...\n", ssid);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WiFi] Connected! IP: ");
    Serial.println(WiFi.localIP());
    showCenteredText("WiFi connected", WiFi.localIP().toString());
  } else {
    Serial.print("[WiFi] Failed. Status code: ");
    Serial.println(WiFi.status());
    showCenteredText("WiFi failed", "Check router");
  }
  delay(2000);
}

// ====== KIRIM KE SERVER ======
void sendToServer(String csvContent) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] WiFi not connected, aborting upload.");
    showCenteredText("WiFi lost", "Upload gagal");
    return;
  }

  // === CEK CSV SIZE ===
  int csvSize = csvContent.length();
  Serial.printf("[DEBUG] CSV size: %d bytes\n", csvSize);

  if (csvSize > 45000) {
    Serial.println("[ERROR] CSV terlalu besar! Upload dibatalkan.");
    showCenteredText("CSV terlalu besar", "Gagal upload");
    return;
  }

  HTTPClient http;
  String endpoint = "http://" + String(serverIP) + ":" + String(serverPort) + "/upload";
  http.begin(endpoint);

  http.addHeader("Content-Type", "text/csv");
  http.addHeader("Content-Length", String(csvSize));
  http.setTimeout(30000);

  Serial.println("[HTTP] Uploading CSV to " + endpoint);

  int httpCode = http.POST(csvContent);

  // === DEBUG JIKA ERROR ===
  if (httpCode <= 0) {
    Serial.printf("[HTTP] POST failed: %d\n", httpCode);
    Serial.println("[HTTP] Detail: " + http.errorToString(httpCode));
    showCenteredText("Upload gagal", "HTTP error");
    http.end();
    return;
  }

  Serial.printf("[HTTP] Response Code: %d\n", httpCode);
  String payload = http.getString();
  Serial.println("[HTTP] Raw Payload:");
  Serial.println(payload);

  // Parsing JSON
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("[Error] JSON parsing failed: ");
    Serial.println(error.c_str());
    showCenteredText("Prediksi gagal", "JSON error");
    http.end();
    return;
  }

  // DEBUG struktur JSON
  serializeJsonPretty(doc, Serial);

  // FIX PARSING — gunakan as<float>()
  float sbp   = doc["predictions"]["SBP"].as<float>();
  float dbp   = doc["predictions"]["DBP"].as<float>();
  float pulse = doc["predictions"]["Pulse"].as<float>();

  Serial.printf("[Result] SBP: %.2f | DBP: %.2f | Pulse: %.2f\n", sbp, dbp, pulse);

  // Tampilkan ke OLED
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Prediksi:");
  display.setCursor(0, 25);
  display.printf("SBP: %.1f", sbp);
  display.setCursor(0, 35);
  display.printf("DBP: %.1f", dbp);
  display.setCursor(0, 45);
  display.printf("Pulse: %.1f", pulse);
  display.display();

  delay(6000);
  http.end();
}

// ====== PEMBACAAN SENSOR ======
void readAndSendData() {
  Serial.println("[Test] Pemeriksaan dimulai selama 60 detik...");
  showCenteredText("Pemeriksaan", "berlangsung...");

  String csv = "Waktu(s),IR,RED\n";
  unsigned long startTime = millis();
  unsigned long nextSample = startTime;

  for (int i = 0; i < NUM_SAMPLES; i++) {
    while (millis() < nextSample) {
      yield();
    }

    unsigned long t = millis() - startTime;
    long red = particleSensor.getRed();
    long ir = particleSensor.getIR();

    csv += String(t / 1000.0, 3) + "," + String(ir) + "," + String(red) + "\n";

    if (i % 50 == 0) {
      Serial.printf("[Sensor] t=%.3f s | IR=%ld | RED=%ld\n", t / 1000.0, ir, red);
    }

    nextSample += SAMPLE_INTERVAL_MS;
  }

  Serial.println("[Test] Pembacaan selesai. Mengirim ke server...");
  showCenteredText("Mengirim data...");
  sendToServer(csv);

  showCenteredText("Upload selesai", "Mode standby");
  Serial.println("[Test] Upload selesai. Kembali ke mode standby.\n");
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OLED] Initialization failed.");
    while (true);
  }

  display.setRotation(2);
  showCenteredText("Inisialisasi...");

  if (!particleSensor.begin(Wire)) {
    Serial.println("[Sensor] MAX30102 not detected!");
    showCenteredText("MAX30102 gagal");
    while (true);
  }

  particleSensor.setup();
  particleSensor.setSampleRate(25); 
  Serial.println("[Sensor] MAX30102 initialized successfully.");

  connectWiFi();

  showCenteredText("Tekan tombol", "utk mulai");
  Serial.println("[System] Ready. Press button to start test.");
}

// ====== LOOP ======
void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(100);
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("[Button] Ditekan. Pemeriksaan dimulai...");
      showCenteredText("Mulai otomatis...");
      delay(1000);
      readAndSendData();
      showCenteredText("Tekan tombol", "utk mulai lagi");
    }
  }
}
