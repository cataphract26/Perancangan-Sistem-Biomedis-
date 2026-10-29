#include "config.h"

extern float co_ppm;
extern float aqi_ppm;

void readSensors() {
  // Rata-rata 5 pembacaan untuk anti-noise
  long sum7 = 0, sum135 = 0;
  for (int i = 0; i < 5; i++) {
    sum7   += analogRead(PIN_MQ7);
    sum135 += analogRead(PIN_MQ135);
    delay(5);
  }
  int raw7   = sum7   / 5;
  int raw135 = sum135 / 5;

  // Validasi: ADC < 50 = sensor belum terpasang atau belum warm-up
  co_ppm  = (raw7   > 50 && raw7   < 4000) ? max(0.0f, (raw7   * MQ7_SLOPE)   + MQ7_OFFSET)   : 0.0f;
  aqi_ppm = (raw135 > 50 && raw135 < 4000) ? max(0.0f, (raw135 * MQ135_SLOPE) + MQ135_OFFSET) : 0.0f;

  Serial.printf("[Sensor] RAW7=%4d → CO=%.1fppm (thr:%.0f) %s | RAW135=%4d → VOC=%.1fppm (thr:%.0f) %s\n",
              raw7,   co_ppm,  THRESHOLD_CO,  (co_ppm  >= THRESHOLD_CO  ? "⚠ALERT" : "OK"),
              raw135, aqi_ppm, THRESHOLD_VOC, (aqi_ppm >= THRESHOLD_VOC ? "⚠ALERT" : "OK"));
}

bool checkDanger() {
  return (co_ppm  >= THRESHOLD_CO ||
          aqi_ppm >= THRESHOLD_VOC);
}