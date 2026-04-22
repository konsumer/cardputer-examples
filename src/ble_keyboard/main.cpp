#include <M5Cardputer.h>

// M5Cardputer's Keyboard_def.h #defines these names; undef them so BleKeyboard
// can declare its own const uint8_t variables with the correct BLE HID values.
#undef KEY_LEFT_CTRL
#undef KEY_LEFT_SHIFT
#undef KEY_LEFT_ALT
#undef KEY_BACKSPACE
#undef KEY_TAB
#undef KEY_ENTER

#include <NimBLEDevice.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("CardputerADV", "M5Stack", 100);

bool wasConnected = false;
char lastKey = 0;

void drawStatus(bool connected) {
  M5Cardputer.Display.fillRect(0, 20, M5Cardputer.Display.width(), 14, BLACK);
  M5Cardputer.Display.setCursor(4, 22);
  if (connected) {
    M5Cardputer.Display.setTextColor(GREEN);
    M5Cardputer.Display.print("Connected   ");
  } else {
    M5Cardputer.Display.setTextColor(YELLOW);
    M5Cardputer.Display.print("Waiting for host...");
  }
  M5Cardputer.Display.setTextColor(WHITE);
}

void drawLastKey(const char* label) {
  M5Cardputer.Display.fillRect(0, 40, M5Cardputer.Display.width(), 14, BLACK);
  M5Cardputer.Display.setCursor(4, 42);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.printf("Sent: %s", label);
  M5Cardputer.Display.setTextColor(WHITE);
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold9pt7b);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.drawCenterString("BLE Keyboard", M5Cardputer.Display.width() / 2, 4);

  // bonding=true, MITM=false, SC=true — required by macOS/Windows/iOS or they drop immediately
  NimBLEDevice::setSecurityAuth(true, false, true);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

  bleKeyboard.begin();
  drawStatus(false);
}

void loop() {
  M5Cardputer.update();

  bool connected = bleKeyboard.isConnected();
  if (connected != wasConnected) {
    wasConnected = connected;
    drawStatus(connected);
  }

  if (!connected) return;

  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    for (auto c : status.word) {
      bleKeyboard.print(c);
      char buf[2] = {c, 0};
      drawLastKey(buf);
    }
    if (status.del) {
      bleKeyboard.write(KEY_BACKSPACE);
      drawLastKey("[DEL]");
    }
    if (status.enter) {
      bleKeyboard.write(KEY_RETURN);
      drawLastKey("[ENTER]");
    }
  }
}
