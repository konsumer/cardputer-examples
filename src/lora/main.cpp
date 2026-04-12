// LoRa chat demo for CardputerADV + LoRa Cap 1262
// PI4IOE5V6408 port expander: P0 = antenna enable, P7 = RX enable

#include "M5Unified.h"
#include <M5Cardputer.h>
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

M5Canvas logCanvas(&M5Cardputer.Display);
String inputBuffer;
String pendingTxMessage;
bool isTxing = false;

constexpr int LOG_MARGIN = 8;
constexpr int LOG_TOP = 24;
constexpr int PROMPT_HEIGHT = 14;

void pushLogCanvas() {
  logCanvas.pushSprite(LOG_MARGIN, LOG_TOP);
}

void drawChrome() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextFont(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(ORANGE);
  M5Cardputer.Display.drawCenterString("LoRa Chat", M5Cardputer.Display.width() / 2, 4);
  M5Cardputer.Display.drawRect(4, LOG_TOP - 4,
                               M5Cardputer.Display.width() - 8,
                               M5Cardputer.Display.height() - (LOG_TOP + PROMPT_HEIGHT) - 2,
                               BLUE);
  pushLogCanvas();
}

void drawPrompt() {
  int promptY = M5Cardputer.Display.height() - PROMPT_HEIGHT - 2;
  M5Cardputer.Display.fillRect(0, promptY, M5Cardputer.Display.width(), PROMPT_HEIGHT + 2, BLACK);
  M5Cardputer.Display.setCursor(4, promptY + 2);
  M5Cardputer.Display.setTextColor(CYAN);
  M5Cardputer.Display.printf("> %s", inputBuffer.c_str());
}

void appendLog(const String& name, const String& message, uint16_t nameColor = YELLOW) {
  logCanvas.setTextColor(nameColor);
  logCanvas.print(name);
  logCanvas.print(": ");
  logCanvas.setTextColor(WHITE);
  logCanvas.println(message);
  pushLogCanvas();
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

void handleKeyboard() {
  if (!(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed())) {
    return;
  }

  Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
  for (auto key : status.word) {
    inputBuffer += key;
  }

  if (status.del && inputBuffer.length() > 0) {
    inputBuffer.remove(inputBuffer.length() - 1);
  }

  if (status.enter) {
    String msg = inputBuffer;
    msg.trim();
    inputBuffer = "";
    drawPrompt();
    if (msg.length() > 0) {
      if (isTxing) {
        appendLog("System", "Radio busy, wait...", RED);
      } else {
        setAntenna(false);
        radio.setPacketSentAction(setTxFlag);
        int state = radio.startTransmit(msg);
        if (state == RADIOLIB_ERR_NONE) {
          isTxing = true;
          pendingTxMessage = msg;
        } else {
          appendLog("System", String("TX error: ") + state, RED);
          startListening();
        }
      }
    }
    return;
  }

  drawPrompt();
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  Serial.begin(115200);

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

  int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SYNC_WORD,
                          LORA_TX_POWER, LORA_PREAMBLE_LEN, 3.0, true);
  if (state != RADIOLIB_ERR_NONE) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(4, 4);
    M5Cardputer.Display.setTextColor(RED);
    M5Cardputer.Display.printf("init failed: %d", state);
    while (true) delay(1000);
  }

  radio.setCurrentLimit(140);
  radio.setDio2AsRfSwitch(false);  // FM8625H is on port expander, not DIO2
  radio.setPacketReceivedAction(setRxFlag);
  
  int canvasWidth = M5Cardputer.Display.width() - LOG_MARGIN * 2;
  int canvasHeight = M5Cardputer.Display.height() - LOG_TOP - PROMPT_HEIGHT - 8;
  logCanvas.createSprite(canvasWidth, canvasHeight);
  logCanvas.fillSprite(BLACK);
  logCanvas.setTextColor(WHITE);
  logCanvas.setTextSize(1);
  logCanvas.setTextScroll(true);

  drawChrome();
  startListening();
  appendLog("System", "Keyboard to send, always listening", ORANGE);
  drawPrompt();
}

void loop() {
  M5Cardputer.update();
  handleKeyboard();

  if (transmittedFlag) {
    transmittedFlag = false;
    radio.finishTransmit();
    startListening();
    if (pendingTxMessage.length() > 0) {
      appendLog("TX", pendingTxMessage, CYAN);
      pendingTxMessage = "";
    }
  }

  if (receivedFlag) {
    receivedFlag = false;

    String payload;
    int state = radio.readData(payload);

    if (state == RADIOLIB_ERR_NONE && payload.length() > 0) {
      float rssi = radio.getRSSI();
      float snr = radio.getSNR();
      String meta = String((int)rssi) + "dBm/" + String(snr, 1) + "dB";
      appendLog("RX", payload + " (" + meta + ")", GREEN);
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      appendLog("RX", "CRC mismatch", YELLOW);
    } else if (state != RADIOLIB_ERR_NONE) {
      appendLog("RX", String("err ") + state, RED);
    }

    startListening();
  }

  delay(5);
}
