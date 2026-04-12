#include "M5Cardputer.h"

M5Canvas canvas(&M5Cardputer.Display);
std::string data = "";

// clear text area & draw current input
void drawPrompt() {
  M5Cardputer.Display.fillRect(0, M5Cardputer.Display.height() - 12, M5Cardputer.Display.width(), 12, BLACK);
  M5Cardputer.Display.setCursor(4, M5Cardputer.Display.height() - 10);
  M5Cardputer.Display.printf("> %s", data.c_str());
}

// add scrolled text attributed to a person, with a color
void say(std::string name, std::string message, int color = YELLOW) {
  canvas.setTextColor(color);
  canvas.printf("%s:", name.c_str());
  canvas.setTextColor(WHITE);
  canvas.println(message.c_str());
  canvas.pushSprite(8, 20);
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);  // enableKeyboard
  M5Cardputer.Display.setRotation(1);

  // prompt and input text are this color
  M5Cardputer.Display.setTextColor(WHITE);

  // draw outer UI directly on screen
  M5Cardputer.Display.drawCenterString("Chat Demo", M5Cardputer.Display.width()/2, 4);
  M5Cardputer.Display.drawRect(4, 18, M5Cardputer.Display.width() - 8, M5Cardputer.Display.height() - 34, BLUE);

  // scrolling text goes into a canvas
  canvas.createSprite(M5Cardputer.Display.width() - 16, M5Cardputer.Display.height() - 38);
  canvas.setTextScroll(true);

  // add an initial messages from "system"
  say("System", "Type your messages.", ORANGE);
  say("System", "This will scroll.", ORANGE);

  // draw initial
  drawPrompt();
}

void loop() {
  M5Cardputer.update();

  // handle input
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    for (auto i : status.word) {
      data += i;
    }
    if (status.del) {
      if (!data.empty()) {
        data.pop_back();
      }
    }
    if (status.enter) {
      say("You", data);
      data = "";
    }
    drawPrompt();
  }
} 