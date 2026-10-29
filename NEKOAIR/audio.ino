#include "config.h"
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

// Objek DFPlayer (dideklarasikan di sini, extern di tab lain jika perlu)
HardwareSerial dfSerial(1);
DFRobotDFPlayerMini dfPlayer;
bool dfReady = false;

// ── Variabel untuk logika audio standby ──────────────────────────
unsigned long lastAudioCheckMs = 0;  // kapan terakhir putar audio_002 + 003
bool wasInDanger = false;            // untuk deteksi recovery dari danger

void initDFPlayer() {
  dfSerial.begin(9600, SERIAL_8N1, PIN_DF_RX, PIN_DF_TX);
  delay(1500);
  if (dfPlayer.begin(dfSerial)) {
    dfPlayer.volume(DF_VOLUME);
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    dfReady = true;
    Serial.printf("[DFPlayer] ✓ Siap. Jumlah file: %d\n",
                  dfPlayer.readFileCounts());
  } else {
    Serial.println("[DFPlayer] ✗ Gagal — cek UART & SD card (FAT32)");
  }
}

void playAudio(int track) {
  if (!dfReady) return;
  dfPlayer.play(track);
  Serial.printf("[Audio] ▶ Playing %04d.mp3\n", track);
}

// ── Dipanggil saat masuk MODE_WELCOME ────────────────────────────
void onEnterWelcome() {
  playAudio(file_001);  // "Selamat datang..."
}

// ── Dipanggil saat masuk MODE_STANDBY dari WELCOME ───────────────
void onEnterStandby() {
  playAudio(file_002); // "Biar saya memeriksa udara..."
  lastAudioCheckMs = millis();
}

// ── Dipanggil periodik di MODE_STANDBY (tiap AUDIO_CHECK_INTERVAL) ──
// Juga dipanggil saat baru masuk standby (dari onEnterStandby)
void onStandbyAudioReport(bool airIsGood) {
  if (airIsGood) {
    playAudio(file_003);    // "Udaranya masih normal..."
  } else {
    // Seharusnya tidak terjadi (bahaya sudah masuk DANGER),
    // tapi sebagai fallback
    playAudio(file_004);
  }
}

// ── Dipanggil saat masuk MODE_DANGER ─────────────────────────────
void onEnterDanger() {
  playAudio(file_004);  // "Oh tidak, saat ini udaranya buruk..."
  wasInDanger = true;
}

// ── Dipanggil saat keluar dari DANGER kembali ke STANDBY ─────────
void onRecoverFromDanger() {
  playAudio(file_005); // "Bagus, udara sudah membaik..."
  wasInDanger = false;
  lastAudioCheckMs = millis(); // Reset timer cek berkala
}

// ── Dipanggil saat masuk MODE_FAREWELL ───────────────────────────
void onEnterFarewell() {
  playAudio(file_006); // "Yahhh, sudah ingin pergi?..."
}

// ── Update audio standby (cek berkala) — panggil di loop() ───────
void updateStandbyAudio(unsigned long now, bool airIsGood) {
  if (now - lastAudioCheckMs >= AUDIO_CHECK_INTERVAL) {
    lastAudioCheckMs = now;
    playAudio(file_002);   // "Biar saya memeriksa..."
    delay(4000);                 // Tunggu audio_002 selesai (~4 detik)
    onStandbyAudioReport(airIsGood); // Lanjut laporan hasil
  }
  static unsigned long audioPhaseStart = 0;
  static int audioPhase = 0;
}