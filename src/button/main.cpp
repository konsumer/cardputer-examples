#include "M5Cardputer.h"

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);
  Serial.begin(115200);
}

void loop() {
  M5Cardputer.update();

  if (M5Cardputer.BtnA.wasPressed()) {
    Serial.println("Button A Pressed");
  }
  if (M5Cardputer.BtnA.wasReleased()) {
    Serial.println("Button A Released");
  }
}