// LoRa TX/RX test for CardputerADV + LoRa Cap 1262
// PI4IOE5V6408 port expander: P0 = antenna enable, P7 = RX enable

#include <M5Unified.h>
#include <RadioLib.h>
#include "utility/PI4IOE5V6408_Class.hpp"

#define LORA_BW           125.0f
#define LORA_SF           7
#define LORA_CR           5
#define LORA_FREQ         915.0
#define LORA_SYNC_WORD    0x12
#define LORA_TX_POWER     0
#define LORA_PREAMBLE_LEN 6

SX1262 radio = new Module(GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_3, GPIO_NUM_6);
m5::PI4IOE5V6408_Class ioe(0x43, 400000, &m5::In_I2C);

volatile bool receivedFlag = false;
volatile bool transmittedFlag = false;

#if defined(ESP32)
IRAM_ATTR
#endif
void setRxFlag(void) { receivedFlag = true; }

#if defined(ESP32)
IRAM_ATTR
#endif
void setTxFlag(void) { transmittedFlag = true; }

M5Canvas c(&M5.Display);

uint32_t txCount = 0;
unsigned long lastTxMs = 0;
bool isTxing = false;

#define MAX_LOG 10
String logLines[MAX_LOG];
int logCount = 0;

void addLog(const String& s) {
  if (logCount < MAX_LOG) logLines[logCount++] = s;
  else {
    for (int i = 0; i < MAX_LOG - 1; i++) logLines[i] = logLines[i + 1];
    logLines[MAX_LOG - 1] = s;
  }
}

void redraw() {
  c.fillScreen(BLACK);
  c.setCursor(4, 4);
  c.setTextColor(TFT_CYAN);
  c.printf("LoRa %.0fMHz SF%d BW%.0f", LORA_FREQ, LORA_SF, LORA_BW);
  c.setTextColor(WHITE);
  for (int i = 0; i < logCount; i++) {
    c.setCursor(4, 16 + i * 10);
    c.print(logLines[i]);
  }
  c.pushSprite(0, 0);
}

void setAntenna(bool rxMode) {
  // P7 controls FM8625H RF switch: HIGH=RX, LOW=TX
  ioe.digitalWrite(7, rxMode);
}

void startListening() {
  setAntenna(true);
  delay(2);  // Let FM8625H RF switch settle after I2C command
  radio.setPacketReceivedAction(setRxFlag);
  radio.startReceive();
  isTxing = false;
}

void setup() {
  M5.begin();
  Serial.begin(115200);

  c.createSprite(M5.Display.width(), M5.Display.height());
  c.setTextSize(1);

  // I2C for port expander
  if (!m5::In_I2C.begin(I2C_NUM_0, 8, 9)) {
    Serial.println("I2C init failed");
  }

  if (ioe.begin()) {
    Serial.println("Port expander OK");
    // P0 = antenna enable (always on)
    ioe.setDirection(0, true);
    ioe.setHighImpedance(0, false);
    ioe.digitalWrite(0, true);
    // P7 = RX/TX path select
    ioe.setDirection(7, true);
    ioe.setHighImpedance(7, false);
    ioe.digitalWrite(7, true);  // Start in RX mode
    Serial.println("Antenna: P0=HIGH (power), P7=HIGH (RX mode)");
  } else {
    Serial.println("Port expander not found!");
  }

  SPI.begin(40, 39, 14, 5);

  int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER, LORA_PREAMBLE_LEN, 3.0, true);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.printf("Radio init failed: %d\n", state);
    c.fillScreen(BLACK);
    c.setCursor(4, 4);
    c.setTextColor(TFT_RED);
    c.printf("init failed: %d", state);
    c.pushSprite(0, 0);
    while (true) delay(1000);
  }
  radio.setCurrentLimit(140);
  radio.setDio2AsRfSwitch(false);  // FM8625H is on port expander, not DIO2

  Serial.printf("Radio OK: %.1fMHz BW=%.0f SF=%d CR=%d SW=0x%02X pwr=%d\n",
    LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD, LORA_TX_POWER);

  startListening();
  addLog("Ready. TX every 10s.");
  lastTxMs = millis();
  redraw();
}

void loop() {
  M5.update();

  // TX done
  if (transmittedFlag) {
    transmittedFlag = false;
    radio.finishTransmit();
    Serial.println("[TX] done, switching to RX");
    startListening();  // Switches antenna back to RX
    redraw();
  }

  // RX
  if (receivedFlag) {
    receivedFlag = false;

    String str;
    int state = radio.readData(str);

    if (state == RADIOLIB_ERR_NONE && str.length() > 0) {
      float rssi = radio.getRSSI();
      float snr = radio.getSNR();
      Serial.printf("[RX] \"%s\" RSSI:%.0f SNR:%.1f\n", str.c_str(), rssi, snr);
      addLog(String("[") + (int)rssi + "dB] " + str);
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      Serial.println("[RX] CRC mismatch!");
      addLog("CRC err");
    } else if (state != RADIOLIB_ERR_NONE) {
      Serial.printf("[RX] err: %d\n", state);
    }

    startListening();
    redraw();
  }

  // TX every 10s
  unsigned long now = millis();
  if (!isTxing && (now - lastTxMs >= 10000)) {
    String msg = String("cardputer ") + txCount++;

    setAntenna(false);  // Switch to TX
    radio.setPacketSentAction(setTxFlag);
    int s = radio.startTransmit(msg);
    if (s == RADIOLIB_ERR_NONE) {
      isTxing = true;
      Serial.printf("[TX] %s\n", msg.c_str());
      addLog("TX: " + msg);
      redraw();
    } else {
      Serial.printf("[TX] err: %d\n", s);
      startListening();
    }
    lastTxMs = now;
  }

  delay(10);
}
