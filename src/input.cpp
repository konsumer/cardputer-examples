#include "M5Cardputer.h"

M5Canvas c(&M5Cardputer.Display);

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  c.createSprite(M5Cardputer.Display.width(), M5Cardputer.Display.height());
  c.setTextSize(2);
}

void loop() {
  M5Cardputer.update();

  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isPressed()) {
      // Get the current state of all keys
      auto status = M5Cardputer.Keyboard.keysState();

      c.fillScreen(BLACK);
      c.setCursor(0, 0);
      c.print("Pressed: ");

      // Loop through standard character keys
      for (auto i : status.word) {
        if (i == ' ') {
          c.print(" [SPACE]");
        } else {
          c.print(i);
        }
      }

      // Check for specific modifier or special keys
      if (status.fn) c.print(" [FN]");
      if (status.enter) c.print(" [ENTER]");
      if (status.del) c.print(" [DEL]");
    }
  }
  c.pushSprite(0, 0);
}
