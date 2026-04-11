#include "M5Unified.h"
#include <RadioLib.h>
#include <SPI.h>

#define LORA_TX_POWER     22
#define LORA_PREAMBLE_LEN 20

// Reticulum defaults
#define LORA_BW           125.0f
#define LORA_SYNC_WORD    0x12
#define LORA_SF           7
#define LORA_CR           5

// US
#define LORA_FREQ         915.0

// Cardputer SPI bus: SCK=40, MOSI=14, MISO=39
// GPS 1262 CAP LoRa pins: NSS=5, IRQ=4, RST=3, BUSY=6
SPIClass spi(HSPI);
SX1262 radio = new Module(5, 4, 3, 6, spi);

M5Canvas c(&M5.Display);

// Log of received packets shown on screen
#define MAX_LOG 11
String log_lines[MAX_LOG];
int log_count = 0;

volatile bool rx_flag = false;
int i = 0;

void IRAM_ATTR on_receive() {
  rx_flag = true;
}

void redraw() {
  c.fillScreen(BLACK);
  c.setCursor(4, 4);
  c.setTextColor(TFT_CYAN);
  c.print("LoRa RX  915MHz SF7 BW125");
  c.setTextColor(WHITE);
  for (int i = 0; i < log_count; i++) {
    c.setCursor(4, 16 + i * 10);
    c.print(log_lines[i]);
  }
  c.pushSprite(0, 0);
}

void add_log(const String& line) {
  if (log_count < MAX_LOG) {
    log_lines[log_count++] = line;
  } else {
    // scroll up
    for (int i = 0; i < MAX_LOG - 1; i++) {
      log_lines[i] = log_lines[i + 1];
    }
    log_lines[MAX_LOG - 1] = line;
  }
  redraw();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  c.createSprite(M5.Display.width(), M5.Display.height());
  c.setTextSize(1);

  Serial.begin(115200);

  spi.begin(40, 39, 14, 5); // SCK, MISO, MOSI, SS

  int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER, LORA_PREAMBLE_LEN, 3.0, true);
  if (state != RADIOLIB_ERR_NONE) {
    c.fillScreen(BLACK);
    c.setCursor(4, 4);
    c.setTextColor(TFT_RED);
    c.printf("init failed: %d", state);
    c.pushSprite(0, 0);
    while (true) { delay(1000); }
  }

  radio.setDio1Action(on_receive);
  radio.startReceive();

  add_log("init OK, listening...");
  redraw();
}

void loop() {
  M5.update();

  add_log(String(i++));
  delay(1000);
  c.pushSprite(0, 0);

  if (rx_flag) {
    rx_flag = false;

    String data;
    int state = radio.readData(data);

    if (state == RADIOLIB_ERR_NONE) {
      float rssi = radio.getRSSI();
      float snr  = radio.getSNR();
      Serial.printf("[RX] %s  RSSI:%.0f SNR:%.1f\n", data.c_str(), rssi, snr);
      add_log(String("[") + (int)rssi + "dB] " + data);
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      add_log("CRC err");
    } else {
      add_log(String("rx err: ") + state);
    }

    redraw();
    radio.startReceive();
  }
}
