#include "config.h"
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <UniversalTelegramBot.h>

// Tambahkan baris ini di NEKOAIR.ino setelah semua #include
void connectWiFi();   // didefinisikan di telegram.ino
void sendTelegram(String& msg);
void sendTelegramStatus(bool isDanger);
String buildTelegramMessage(bool isDanger);

// ── Objek global ─────────────────────────────────────────────────
Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);

// ── State Machine ─────────────────────────────────────────────────
enum Mode { IDLE,
            WELCOME,
            STANDBY,
            DANGER,
            FAREWELL };
Mode mode = IDLE;
Mode prevMode = IDLE;  // untuk deteksi transisi

// ── Variabel waktu ────────────────────────────────────────────────
unsigned long modeStart = 0;
unsigned long lastPIR = 0;
unsigned long lastSensor = 0;
unsigned long lastFrame = 0;
unsigned long lastTelegramMs = 0;
unsigned long lastWifiRetryMs = 0;
unsigned long warmupStartMs = 0;

// ── Status sistem ─────────────────────────────────────────────────
bool warmupDone = false;
bool wifiConnected = false;
bool pirState = false;

// ── Variabel animasi (dipakai di face_animation.ino) ─────────────
int frameCount = 0;
int welcomeFrame = 0;
int farewellFrame = 0;
int dangerTick = 0;
int idleAnim = 0;
unsigned long idleAnimStart = 0;
float flyAngle = 0.0f;
bool blinking = false;
unsigned long blinkStart = 0;

// ── Data sensor ───────────────────────────────────────────────────
float co_ppm = 0.0f;
float lpg_ppm = 0.0f;
float aqi_ppm = 0.0f;

// ─────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12);
  pinMode(PIN_PIR, INPUT_PULLDOWN);

  // ── I2C & OLED ──────────────────────────────────────────────────
  Wire.begin(OLED_SDA, OLED_SCL);
  delay(100);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[OLED] GAGAL! Cek kabel SDA/SCL.");
  } else {
    Serial.println("[OLED] OK!");
    oledSplash();           // Tampil "NEKO-AIR System Loading..." 2 detik
  }

  // ── DFPlayer ────────────────────────────────────────────────────
  initDFPlayer();

  // ── WiFi ─────────────────────────────────────────────────────────
  connectWiFi();

  // ── Mulai dari WELCOME — tanpa butuh PIR, tanpa warm-up 60 detik ─
  warmupDone  = true;
  mode        = WELCOME;
  modeStart   = millis();
  welcomeFrame = 0;
  lastPIR     = millis();   // reset timer PIR

  delay(500);               // beri DFPlayer waktu stabilisasi
  onEnterWelcome();         // ← Play audio_001 "Selamat datang!"
  Serial.println("[Mode] BOOT → WELCOME");
}

// ─────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── FASE 1: WARM-UP ─────────────────────────────────────────────
  if (!warmupDone) {
    int secsLeft = (int)((WARMUP_MS - (now - warmupStartMs)) / 1000);
    if (secsLeft <= 0) {
      warmupDone = true;
      Serial.println("[Sensor] ✓ Warm-up selesai!");
      oledShowMessage("Warm-up Selesai!", "Sistem aktif...");
      delay(1000);
      idleAnimStart = millis();
    } else {
      if (now - lastFrame >= 1000UL) {
        lastFrame = now;
        oledShowWarmup(secsLeft);  // dari face_animation.ino
      }
      return;
    }
  }

  // ── FASE 2: BACA SENSOR ──────────────────────────────────────────
  if (now - lastSensor >= SENSOR_PERIOD_MS) {
    lastSensor = now;
    readSensors();  // dari sensors.ino
  }

  // ── FASE 3: BACA PIR ─────────────────────────────────────────────
  bool pirNow = (digitalRead(PIN_PIR) == HIGH);
  if (pirNow) lastPIR = now;
  bool gasAlert = checkDanger();  // dari sensors.ino

  // ── FASE 4: STATE MACHINE ────────────────────────────────────────
  prevMode = mode;

  switch (mode) {

    case IDLE:
      if (pirNow) {
        mode = WELCOME;
        modeStart = now;
        welcomeFrame = 0;
        onEnterWelcome();  // audio.ino: putar file_001
        Serial.println("[Mode] IDLE → WELCOME");
      }
      break;

    case WELCOME:
      if (now - modeStart >= WELCOME_DUR_MS) {
        mode = STANDBY;
        modeStart = now;
        idleAnim = 0;
        idleAnimStart = now;
        flyAngle = 0.0f;
        onEnterStandby();  // audio.ino: putar file_002 → file_003
        Serial.println("[Mode] WELCOME → STANDBY");
      }
      break;

    case STANDBY:
      // Grace period 30 detik setelah masuk STANDBY ─ beri waktu sensor stabil
      if (now - modeStart < 30000UL) break;   // ← TAMBAHKAN INI// Transisi ke DANGER

      if (gasAlert) {
        mode = DANGER;
        modeStart = now;
        dangerTick = 0;
        onEnterDanger();  // audio.ino: putar file_004
        if (wifiConnected) {
          sendTelegramStatus(true);
          lastTelegramMs = now;
        }
        Serial.println("[Mode] STANDBY → DANGER");
        break;
      }
      // Transisi ke FAREWELL
      if (!pirNow && (now - lastPIR >= PIR_IDLE_TIMEOUT_MS)) {
        mode = FAREWELL;
        modeStart = now;
        farewellFrame = 0;
        onEnterFarewell();  // audio.ino: putar file_006
        Serial.println("[Mode] STANDBY → FAREWELL");
        break;
      }
      // Audio cek berkala di standby (tiap 2 menit)
      updateStandbyAudio(now, !gasAlert);  // audio.ino
      break;

    case DANGER:
      // Gas sudah aman → kembali ke STANDBY
      if (co_ppm < THRESHOLD_CO_LOW && aqi_ppm < THRESHOLD_VOC_LOW) {
        mode = STANDBY;
        modeStart = now;
        idleAnim = 0;
        idleAnimStart = now;
        onRecoverFromDanger();  // audio.ino: putar file_005
        if (wifiConnected) {
          String msg = "✅ *NEKO-AIR: Udara kembali aman!*\n\n" + buildTelegramMessage(false);
          sendTelegram(msg);
          lastTelegramMs = now;
        }
        Serial.println("[Mode] DANGER → STANDBY");
        break;
      }
      // Penumpang keluar saat bahaya
      if (!pirNow && (now - lastPIR >= PIR_IDLE_TIMEOUT_MS)) {
        mode = FAREWELL;
        modeStart = now;
        farewellFrame = 0;
        onEnterFarewell();
        Serial.println("[Mode] DANGER → FAREWELL");
        break;
      }
      // Kirim Telegram berkala saat bahaya
      if (wifiConnected && (now - lastTelegramMs >= TELEGRAM_DANGER_MS)) {
        lastTelegramMs = now;
        sendTelegramStatus(true);
      }
      break;

    case FAREWELL:
      // Penumpang kembali masuk
      if (pirNow) {
        mode = WELCOME;
        modeStart = now;
        welcomeFrame = 0;
        onEnterWelcome();
        Serial.println("[Mode] FAREWELL → WELCOME");
        break;
      }
      if (now - modeStart >= FAREWELL_DUR_MS) {
        mode = IDLE;
        Serial.println("[Mode] FAREWELL → IDLE");
      }
      break;
  }

  // ── FASE 5: RENDER OLED ──────────────────────────────────────────
  if (now - lastFrame >= FRAME_PERIOD_MS) {
    lastFrame = now;
    frameCount++;
    switch (mode) {
      case IDLE: faceIdle(); break;
      case WELCOME: faceWelcome(welcomeFrame++); break;
      case STANDBY: faceStandby(now - modeStart); break;
      case DANGER: faceDanger(dangerTick++); break;
      case FAREWELL: faceFarewell(farewellFrame++); break;
    }
  }

  // ── FASE 6: TELEGRAM STATUS NORMAL ──────────────────────────────
  if (wifiConnected && mode == STANDBY && (now - lastTelegramMs >= TELEGRAM_NORMAL_MS)) {
    lastTelegramMs = now;
    sendTelegramStatus(false);
  }

  // ── FASE 7: RECONNECT WiFi ────────────────────────────────────────
  if (WiFi.status() != WL_CONNECTED && !wifiConnected) {
    if (now - lastWifiRetryMs >= WIFI_RETRY_MS) {
      lastWifiRetryMs = now;
      connectWiFi();
    }
  } else {
    wifiConnected = (WiFi.status() == WL_CONNECTED);
  }

  pirState = pirNow;
  delay(20);
}