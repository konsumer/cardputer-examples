#include "M5Unified.h"
#include "MultipleSatellite.h"

// GPS 1262 LoRa CAP: CASIC chip defaults to 9600 baud
static const int RXPin = 15, TXPin = 13;
static const uint32_t GPSBaud = 115200;

MultipleSatellite gps(Serial1, GPSBaud, SERIAL_8N1, RXPin, TXPin);

M5Canvas c(&M5.Display);

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  c.createSprite(M5.Display.width(), M5.Display.height());
  c.setTextSize(1);
  c.setTextColor(WHITE);

  Serial.begin(115200);

  gps.begin();
  delay(500);

  // Hot start: reuse cached almanac/ephemeris for fast fix (~seconds vs minutes)
  // Use BOOT_COLD_START only if you suspect corrupted almanac data
  gps.setSystemBootMode(BOOT_HOST_START);
  delay(500);

  Serial.println("GPS ready, waiting for NMEA...");
}

void loop() {
  M5.update();

  // Feed all available bytes into TinyGPS++ parser and echo to USB serial
  while (Serial1.available()) {
    char c_raw = Serial1.read();
    Serial.write(c_raw);  // mirror to USB serial monitor
    gps.encode(c_raw);
  }

  c.fillScreen(BLACK);
  c.setCursor(4, 4);
  c.printf("Sats  : %d", gps.satellites.isValid() ? (int)gps.satellites.value() : 0);
  c.setCursor(4, 14);
  c.printf("Fix   : %s", gps.location.isValid() ? "YES" : "NO");
  c.setCursor(4, 24);
  if (gps.location.isValid()) {
    c.printf("Lat: %.6f", gps.location.lat());
    c.setCursor(4, 34);
    c.printf("Lng: %.6f", gps.location.lng());
  } else {
    c.print("Lat: --");
    c.setCursor(4, 34);
    c.print("Lng: --");
  }
  c.setCursor(4, 44);
  c.printf("HDOP  : %.1f", gps.hdop.isValid() ? gps.hdop.hdop() : 0.0f);
  c.setCursor(4, 54);
  c.printf("Chars : %lu", gps.charsProcessed());
  c.setCursor(4, 64);
  c.printf("Sentences: %lu", gps.sentencesWithFix());
  
  c.pushSprite(0, 0);
  delay(100);
}
