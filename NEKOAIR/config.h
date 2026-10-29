#ifndef CONFIG_H
#define CONFIG_H

// ── WiFi & Telegram ──────────────────────────────────────────────
#define WIFI_SSID       "[isi nama wifi]"
#define WIFI_PASSWORD   "[isi kata sandi wifi]"
#define BOT_TOKEN       "[isi token bot telegram]"
#define CHAT_ID         "[isi ID chat telegram]"

// ── Pin Definitions (REVISI — hindari GPIO0 dan GPIO8) ───────────
#define PIN_MQ7         1     // GPIO1, ADC1 CH1 — pindah dari GPIO0!
#define PIN_MQ135       2     // GPIO2, ADC1 CH2
#define PIN_PIR         3     // GPIO3, Digital input
#define OLED_SCL        4     // GPIO4 — pindah dari GPIO7
#define PIN_DF_TX       5     // GPIO5 → DFPlayer RX (via 1kΩ)
#define PIN_DF_RX       6     // GPIO6 ← DFPlayer TX
#define OLED_SDA        10    // GPIO10 — pindah dari GPIO8 (GPIO8 = LED onboard!)

// Threshold masuk DANGER 
#define THRESHOLD_CO     120.0f
#define THRESHOLD_VOC    300.0f

// Threshold keluar DANGER — lebih rendah ~60% dari threshold masuk
#define THRESHOLD_CO_LOW  90.0f   
#define THRESHOLD_VOC_LOW 270.0f  

// ── Nomor File Audio ─────────────────────────────────────────────
#define file_001        1
#define file_002        2
#define file_003        3
#define file_004        4
#define file_005        5
#define file_006        6

// ── Timing ───────────────────────────────────────────────────────
#define WARMUP_MS            60000UL
#define PIR_IDLE_TIMEOUT_MS  3600000UL
#define WELCOME_DUR_MS        5000UL
#define FAREWELL_DUR_MS       5000UL
#define SENSOR_PERIOD_MS      1000UL
#define FRAME_PERIOD_MS         80UL
#define AUDIO_CHECK_INTERVAL 120000UL
#define TELEGRAM_DANGER_MS   30000UL
#define TELEGRAM_NORMAL_MS   60000UL
#define WIFI_RETRY_MS        30000UL

// ── OLED ─────────────────────────────────────────────────────────
#define SCREEN_W    128
#define SCREEN_H     64
#define OLED_ADDR   0x3C

// ── DFPlayer ─────────────────────────────────────────────────────
#define DF_VOLUME    75

// ── Kalibrasi Sensor ─────────────────────────────────────────────
#define MQ7_SLOPE     0.050f
#define MQ7_OFFSET   -15.0f
#define MQ135_SLOPE   0.090f
#define MQ135_OFFSET -18.0f

#endif
