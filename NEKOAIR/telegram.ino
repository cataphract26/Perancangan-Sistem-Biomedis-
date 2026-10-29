#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

const char* ssid = "Infinix NOTE 40";
const char* password = "rasyid54312";

#define BOTtoken "8613117537:AAHPkFfo1oTs8eSUmUH2b2XNkuNDIyWaCvc"
#define CHAT_ID "1474534121"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// ── Fungsi untuk menghubungkan ESP32 ke WiFi ─────────────────────
void connectWiFi() {
  Serial.print("\n[WiFi] Menghubungkan ke ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  // Tunggu sampai connect, maksimal 20 kali percobaan (10 detik)
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] ✓ Berhasil Terhubung!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WiFi] ✗ Gagal Terhubung.");
  }
}

// ── Fungsi untuk merakit teks pesan Telegram ─────────────────────
String buildTelegramMessage(bool isDanger) {
  String msg = "";
  
  if (isDanger) {
    msg += "🚨 *PERINGATAN BAHAYA UDARA BURUK!* 🚨\n\n";
    msg += "Sistem mendeteksi lonjakan gas berlebih di ruangan.\n\n";
  } else {
    msg += "📊 *Laporan Rutin Kualitas Udara* 📊\n\n";
  }
  
  // Masukkan nilai pembacaan sensor
  msg += "Kondisi saat ini:\n";
  msg += "💨 CO  : " + String(co_ppm) + " ppm\n";
  msg += "🔥 LPG : " + String(lpg_ppm) + " ppm\n";
  msg += "🌫️ AQI : " + String(aqi_ppm) + " ppm\n";
  
  return msg;
}

// ── Fungsi untuk mengirim status ke Telegram ─────────────────────
void sendTelegramStatus(bool isDanger) {
  // Rakit pesannya dulu
  String msgToSend = buildTelegramMessage(isDanger);
  
  // Kirim pesannya menggunakan fungsi sendTelegram yang sudah kamu perbaiki sebelumnya
  sendTelegram(msgToSend); 
}

// ================================================================
//  FUNGSI: KIRIM PESAN KE TELEGRAM BOT API
// ================================================================
void sendTelegram(String& message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Telegram] Tidak terkirim: WiFi offline.");
    return;
  }

  // Escape karakter untuk JSON
  String escaped = message;
  escaped.replace("\\", "\\\\");
  escaped.replace("\"", "\\\"");
  escaped.replace("\n", "\\n");
  escaped.replace("\r", "");

  // Buat URL dan payload
  String url     = String("https://api.telegram.org/bot") + BOT_TOKEN + "/sendMessage";
  String payload = String("{\"chat_id\":\"") + CHAT_ID + "\","
                   "\"text\":\"" + escaped + "\","
                   "\"parse_mode\":\"Markdown\"}";

  // Gunakan WiFiClientSecure untuk koneksi HTTPS
  WiFiClientSecure client;
  client.setInsecure(); // Skip verifikasi SSL certificate (cukup untuk development)

  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000);

  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    Serial.println("[Telegram] ✓ Pesan terkirim!");
  } else if (httpCode > 0) {
    Serial.printf("[Telegram] ✗ HTTP Error: %d\n", httpCode);
    Serial.println(http.getString());
  } else {
    Serial.printf("[Telegram] ✗ Connection Error: %s\n",
                  http.errorToString(httpCode).c_str());
  }

  http.end();
}