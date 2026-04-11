// Disable restarting receiver after each send.
// Save 450 bytes program memory and 269 bytes RAM if receiving functions are not used.
#define DISABLE_CODE_FOR_RECEIVER
#define SEND_PWM_BY_TIMER
#define IR_TX_PIN 44

#include "M5Cardputer.h"
// Third-party library: https://github.com/Arduino-IRremote/Arduino-IRremote
#include <IRremote.hpp>

uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);  // enableKeyboard
  Serial.begin();

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextColor(GREEN);
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.setTextFont(&fonts::Orbitron_Light_24);
  M5Cardputer.Display.setTextSize(1);

  IrSender.begin(DISABLE_LED_FEEDBACK);
  IrSender.setSendPin(IR_TX_PIN);
}

void loop() {
  Serial.println();
  Serial.print(F("Send now: address=0x1111, command=0x"));
  Serial.print(sCommand, HEX);
  Serial.print(F(", repeats="));
  Serial.print(sRepeats);
  Serial.println();
  Serial.println(F("Send standard NEC with 16 bit address"));

  M5Cardputer.Display.clear();
  M5Cardputer.Display.drawString("IR NEC SEND", M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 - 40);
  M5Cardputer.Display.drawString("ADDR:0x1111", M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2);
  M5Cardputer.Display.drawString("CMD:0x" + String(sCommand, HEX), M5Cardputer.Display.width() / 2, M5Cardputer.Display.height() / 2 + 40);

  M5Cardputer.Display.fillCircle(32, 105, 8, GREEN);
  IrSender.sendNEC(0x1111, sCommand, sRepeats);
  // IrSender.sendOnkyo(0x1111, 0x2223, sRepeats);

  sCommand += 1;
  delay(500);

  M5Cardputer.Display.fillCircle(32, 105, 8, YELLOW);
  delay(500);
} 