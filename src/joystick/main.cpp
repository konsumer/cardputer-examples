#include <Arduino.h>
#include <M5Cardputer.h>
#include <Wire.h>
#include <m5_unit_joystick2.hpp>

M5UnitJoystick2 joystick2;
M5Canvas* canvas = nullptr;

void show_joystick() {
  uint8_t bootloader_ver, firmware_ver, button;
  uint16_t adc_x, adc_y;
  bootloader_ver = joystick2.get_bootloader_version();
  firmware_ver   = joystick2.get_firmware_version();
  joystick2.get_joy_adc_16bits_value_xy(&adc_x, &adc_y);
  button = joystick2.get_button_value();

  canvas->clear(BLACK);
  canvas->setTextSize(1);
  canvas->setTextColor(YELLOW);
  canvas->setCursor(0, 0);
  canvas->printf("bootloader ver:%d\nfirmware ver:%d\nx adc:%d\ny adc:%d\nbutton: %d", bootloader_ver, firmware_ver, adc_x, adc_y, button);
  canvas->pushSprite(0, 0);
}


void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    M5Cardputer.Display.setRotation(1);
    canvas = new M5Canvas(&M5Cardputer.Display);
    canvas->createSprite(M5Cardputer.Display.width(), M5Cardputer.Display.height());

    joystick2.begin(&Wire, JOYSTICK2_ADDR, 2, 1);
    joystick2.set_rgb_color(0x00ff00);
}

void loop() {
    show_joystick();
    delay(100);
}