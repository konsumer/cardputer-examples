#include <Arduino.h>
#include <M5Cardputer.h>
#include "ath10.h"

// conver celsius to fahrenheit
#define TO_FAHRENHEIT(c) ((c) * 9.0f / 5.0f + 32.0f)

// priont out what is found on current i2c bus
static void scanI2C() {
    Serial.println("Scanning I2C bus...");
    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  Device at 0x%02X\n", addr);
            found++;
        }
    }
    Serial.printf("Scan done. %d device(s) found.\n", found);
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);
    Wire.begin(2, 1, 100000UL);  // external Grove: SDA=GPIO2, SCL=GPIO1
    Serial.begin(115200);
    while (!Serial && millis() < 3000) { delay(10); }

    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextSize(1.5);

    scanI2C();

    if (aht10_begin()) {
        Serial.println("AHT10 init good.");
    } else {
        Serial.println("AHT10 init failed. Is the sensor connected to the Grove port?");
    }
}

void loop() {
    M5Cardputer.update();

    float humidity, temperature;
    if (aht10_read(humidity, temperature)) {
        float gamma    = log(humidity / 100.0f) + 17.62f * temperature / (243.5f + temperature);
        float dewpoint = 243.5f * gamma / (17.62f - gamma);

        float tempF     = TO_FAHRENHEIT(temperature);
        float dewpointF = TO_FAHRENHEIT(dewpoint);

        Serial.printf("Humidity: %.1f%%  Temp: %.2fC / %.2fF  Dewpoint: %.2fC / %.2fF\n", humidity, temperature, tempF, dewpoint, dewpointF);

        M5Cardputer.Display.clear(BLACK);
        M5Cardputer.Display.setCursor(0, 0);
        M5Cardputer.Display.printf("Humidity: %.1f%%\nTemp:     %.1fC / %.1fF\nDewpoint: %.1fC / %.1fF", humidity, temperature, tempF, dewpoint, dewpointF);
    } else {
        Serial.println("Read failed.");
    }

    delay(500);
}
