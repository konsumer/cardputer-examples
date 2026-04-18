#pragma once
#include <Wire.h>

#define ENC8_ADDR 0x41
#define ENC8_REG_ENCODER 0x00
#define ENC8_REG_BUTTON  0x50
#define ENC8_REG_SWITCH  0x60
#define ENC8_REG_RGB     0x70

static void enc8_read(TwoWire& wire, uint8_t reg, uint8_t* data, uint8_t len) {
    wire.beginTransmission(ENC8_ADDR);
    wire.write(reg);
    wire.endTransmission();
    wire.requestFrom(ENC8_ADDR, (int)len);
    for (uint8_t i = 0; i < len; i++) data[i] = wire.read();
}

static void enc8_write(TwoWire& wire, uint8_t reg, const uint8_t* data, uint8_t len) {
    wire.beginTransmission(ENC8_ADDR);
    wire.write(reg);
    wire.write(data, len);
    wire.endTransmission();
}

inline int32_t enc8_getEncoder(TwoWire& wire, uint8_t index) {
    uint8_t d[4];
    enc8_read(wire, ENC8_REG_ENCODER + index * 4, d, 4);
    return (int32_t)(d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24));
}

inline bool enc8_getButton(TwoWire& wire, uint8_t index) {
    uint8_t d;
    enc8_read(wire, ENC8_REG_BUTTON + index, &d, 1);
    return d;
}

inline bool enc8_getSwitch(TwoWire& wire) {
    uint8_t d;
    enc8_read(wire, ENC8_REG_SWITCH, &d, 1);
    return d;
}

inline void enc8_setLED(TwoWire& wire, uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t d[3] = {r, g, b};
    enc8_write(wire, ENC8_REG_RGB + index * 3, d, 3);
}
