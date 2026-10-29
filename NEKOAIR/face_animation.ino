// ── Fungsi Helper ────────────────────────────────────────────────
void oledSplash() {
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(20, 20);
  oled.println("NEKO-AIR");
  oled.setTextSize(1);
  oled.setCursor(25, 45);
  oled.println("System Loading...");
  oled.display();
  delay(2000);
}

void oledShowWarmup(int secs) {
  oled.clearDisplay();
  oled.drawRect(14, 40, 100, 10, SSD1306_WHITE);
  int barWidth = map(secs, 60, 0, 0, 96);
  oled.fillRect(16, 42, barWidth, 6, SSD1306_WHITE);
  
  oled.setCursor(30, 10);
  oled.print("WARMING UP");
  oled.setCursor(45, 25);
  oled.print(secs); oled.print(" s");
  oled.display();
}

void oledShowMessage(String l1, String l2) {
  oled.clearDisplay();
  oled.setCursor(0, 20); oled.println(l1);
  oled.setCursor(0, 40); oled.println(l2);
  oled.display();
}

// ── Animasi Wajah ────────────────────────────────────────────────
void drawEyes(int xL, int yL, int xR, int yR, int w, int h) {
  oled.fillRoundRect(xL, yL, w, h, 2, SSD1306_WHITE);
  oled.fillRoundRect(xR, yR, w, h, 2, SSD1306_WHITE);
}

void faceIdle() {
  oled.clearDisplay();
  // Mata tertutup (garis)
  oled.drawLine(35, 30, 55, 30, SSD1306_WHITE);
  oled.drawLine(75, 30, 95, 30, SSD1306_WHITE);
  oled.setCursor(45, 45); oled.print("zZZ...");
  oled.display();
}

void faceWelcome(int frame) {
  oled.clearDisplay();
  int h = map(min(frame, 10), 0, 10, 1, 15);
  drawEyes(35, 30 - h/2, 75, 30 - h/2, 20, h);
  oled.display();
}

void faceStandby(unsigned long elapsed) {
  extern float co_ppm, aqi_ppm;        // referensi variabel global dari NEKOAIR.ino

  oled.clearDisplay();
  oled.invertDisplay(false);
  oled.setTextColor(SSD1306_WHITE);     // ← FIX BUG: wajib ada!

  // Animasi mata kedip — dinaikkan ke y=2 agar ada ruang data di bawah
  int h = 15;
  if (millis() % 4000 < 200) h = 1;
  drawEyes(35, 2, 75, 2, 20, h);

  // Garis pemisah antara wajah dan data
  oled.drawFastHLine(0, 22, 128, SSD1306_WHITE);

  // Data sensor — 2 baris (CO dan VOC/AQI)
  oled.setTextSize(1);
  oled.setCursor(0, 26);
  oled.printf("CO  : %6.1f ppm", co_ppm);

  oled.setCursor(0, 36);
  oled.printf("VOC : %6.1f ppm", aqi_ppm);

  // Baris status — teks besar, kontras
  oled.drawFastHLine(0, 48, 128, SSD1306_WHITE);
  oled.setCursor(0, 52);
  if (co_ppm >= THRESHOLD_CO || aqi_ppm >= THRESHOLD_VOC) {
    oled.print("!! UDARA BURUK !!");
  } else {
    oled.print("  Udara: AMAN  ");
  }

  oled.display();
}

void faceDanger(int tick) {
  extern float co_ppm, aqi_ppm;

  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);     // ← wajib ada!

  if (tick % 2 == 0) {
    oled.invertDisplay(true);
  } else {
    oled.invertDisplay(false);
  }

  // Mata marah (segitiga, posisi lebih kecil agar ada ruang data)
  oled.fillTriangle(28, 0,  52, 16, 28, 16, SSD1306_WHITE);
  oled.fillTriangle(100, 0, 76, 16, 100, 16, SSD1306_WHITE);

  oled.setTextSize(1);
  oled.setCursor(32, 20); oled.print("!! BAHAYA !!");
  oled.setCursor(0,  34); oled.printf("CO  : %.1f ppm", co_ppm);
  oled.setCursor(0,  45); oled.printf("VOC : %.1f ppm", aqi_ppm);
  oled.display();
}

void faceFarewell(int frame) {
  oled.clearDisplay();
  drawEyes(35, 30, 75, 30, 20, 2); // Mata sayu
  oled.setCursor(35, 45); oled.print("Bye Bye~");
  oled.display();
}