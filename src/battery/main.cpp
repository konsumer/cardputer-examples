#include <M5Cardputer.h>

void setup() {
  M5Cardputer.begin();
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold9pt7b);
  M5Cardputer.Display.setCursor(0, 0);

  M5Cardputer.Display.print("   Cardputer (-Adv)\n");
  M5Cardputer.Display.print("    Battery Status\n\n");
  M5Cardputer.Display.print("  Percent:\n");
  M5Cardputer.Display.print("  Voltage:\n");
}

void loop() {
  M5Cardputer.update();

  bool isCharging = M5Cardputer.Power.isCharging();
  int batteryLevel = M5Cardputer.Power.getBatteryLevel();      // 0 - 100 %
  int batteryVoltage = M5Cardputer.Power.getBatteryVoltage();  // unit: mV

  M5Cardputer.Display.setCursor(120, 55);
  M5Cardputer.Display.printf("%3d %%", batteryLevel);
  M5Cardputer.Display.setCursor(120, 72);
  M5Cardputer.Display.printf("%4d mV", batteryVoltage);

  delay(1000);
}