#include <Arduino.h>
#include <M5Cardputer.h>
#include <Wire.h>
#include "encoder8.h"

M5Canvas* canvas = nullptr;

// hue 0-255 → RGB
static void hsv_to_rgb(uint8_t hue, uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t region = hue / 43;
    uint8_t rem    = (hue - region * 43) * 6;
    uint8_t q      = 255 - rem;
    uint8_t t      = rem;
    switch (region) {
        case 0: r = 255; g = t;   b = 0;   break;
        case 1: r = q;   g = 255; b = 0;   break;
        case 2: r = 0;   g = 255; b = t;   break;
        case 3: r = 0;   g = q;   b = 255; break;
        case 4: r = t;   g = 0;   b = 255; break;
        default:r = 255; g = 0;   b = q;   break;
    }
}

static uint8_t rainbow_offset = 0;

void update_leds() {
    for (int i = 0; i < 8; i++) {
        uint8_t r, g, b;
        hsv_to_rgb((uint8_t)(rainbow_offset + i * 32), r, g, b);
        enc8_setLED(Wire, i, r, g, b);
    }
    rainbow_offset += 3;
}

void show_encoder_value() {
    int32_t encoder[8]   = {0};
    bool    buttons[8]   = {false};
    bool    sw           = false;

    for (int i = 0; i < 8; i++) {
        encoder[i] = enc8_getEncoder(Wire, i);
        buttons[i] = enc8_getButton(Wire, i);
    }
    sw = enc8_getSwitch(Wire);

    int w   = canvas->width();
    int h   = canvas->height();
    int col = w / 4;

    canvas->clear(BLACK);
    canvas->setTextSize(1);
    canvas->setTextColor(YELLOW);
    canvas->setCursor(0, 0);
    canvas->print("8-Encoder");

    for (int i = 0; i < 8; i++) {
        int      row = i < 4 ? 0 : 1;
        int      x   = (i % 4) * col;
        int      y   = 16 + row * (h / 2);
        uint32_t bg  = buttons[i] ? (uint32_t)GREEN : (uint32_t)DARKGREY;
        canvas->fillRect(x, y, col - 1, h / 2 - 2, bg);
        canvas->setTextColor(BLACK);
        canvas->drawString(String(encoder[i]), x + 2, y + 4);
    }

    canvas->setTextColor(sw ? GREEN : RED);
    canvas->drawString(sw ? "SW:ON" : "SW:OFF", 0, h - 10);

    canvas->pushSprite(0, 0);
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    Wire.begin(2, 1, 100000UL);
    canvas = new M5Canvas(&M5Cardputer.Display);
    canvas->createSprite(M5Cardputer.Display.width(), M5Cardputer.Display.height());
}

void loop() {
    M5Cardputer.update();
    show_encoder_value();
    update_leds();
    delay(50);
}
