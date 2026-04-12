// LoRa chat demo for CardputerADV + LoRa Cap 1262
// Uses a dedicated FreeRTOS radio task that keeps SX1262 in RX except
// during transmissions. The UI mirrors the simple chat layout from src/keyboard.

#include <Arduino.h>
#include "M5Unified.h"
#include "M5Cardputer.h"
#include <RadioLib.h>
#include "utility/PI4IOE5V6408_Class.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <algorithm>
#include <cstring>

#define LORA_BW           125.0f
#define LORA_SF           7
#define LORA_CR           5
#define LORA_FREQ         915.0
#define LORA_SYNC_WORD    0x12
#define LORA_TX_POWER     0
#define LORA_PREAMBLE_LEN 6

static constexpr size_t MAX_MESSAGE_LEN = 192;
static constexpr uint8_t TX_QUEUE_DEPTH = 6;
static constexpr uint8_t EVENT_QUEUE_DEPTH = 12;
static constexpr uint32_t RADIO_TASK_STACK = 4096;
static constexpr uint32_t RADIO_TASK_PRIORITY = 1;
static constexpr uint32_t RADIO_TASK_DELAY_MS = 5;

SX1262 radio = new Module(GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_3, GPIO_NUM_6);
m5::PI4IOE5V6408_Class ioe(0x43, 400000, &m5::In_I2C);

M5Canvas canvas(&M5Cardputer.Display);
String inputBuffer;

struct RadioJob {
  char text[MAX_MESSAGE_LEN + 1];
};

enum class RadioEventType : uint8_t { Rx, TxOk, TxErr, Info };

struct RadioEvent {
  RadioEventType type;
  char text[MAX_MESSAGE_LEN + 1];
  float rssi;
  float snr;
  int status;
};

QueueHandle_t txQueue = nullptr;
QueueHandle_t eventQueue = nullptr;
TaskHandle_t radioTaskHandle = nullptr;

void radioTask(void* arg);
void restartReceive(const char* reason = nullptr);

void copyToBuffer(char* dest, size_t size, const String& src) {
  if (!dest || size == 0) {
    return;
  }
  size_t len = src.length();
  if (len >= size) {
    len = size - 1;
  }
  memcpy(dest, src.c_str(), len);
  dest[len] = '\0';
}

void queueEvent(RadioEventType type, const String& text, float rssi = 0.0f, float snr = 0.0f, int status = 0) {
  if (!eventQueue) {
    return;
  }
  RadioEvent evt = {};
  evt.type = type;
  evt.rssi = rssi;
  evt.snr = snr;
  evt.status = status;
  copyToBuffer(evt.text, sizeof(evt.text), text);
  xQueueSend(eventQueue, &evt, 0);
}

void setAntenna(bool rxMode) {
  // P7 controls FM8625H RF switch: HIGH=RX, LOW=TX
  ioe.digitalWrite(7, rxMode);
  delay(2);
}

void drawPrompt() {
  int promptY = M5Cardputer.Display.height() - 12;
  M5Cardputer.Display.fillRect(0, promptY, M5Cardputer.Display.width(), 12, BLACK);
  M5Cardputer.Display.setCursor(4, promptY + 2);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.printf("> %s", inputBuffer.c_str());
}

void say(const String& name, const String& message, uint16_t color = YELLOW) {
  canvas.setTextColor(color);
  canvas.print(name);
  canvas.print(": ");
  canvas.setTextColor(WHITE);
  canvas.println(message);
  canvas.pushSprite(8, 20);
  Serial.printf("%s: %s\n", name.c_str(), message.c_str());
}

bool enqueueTx(const String& msg) {
  if (!txQueue) {
    return false;
  }
  RadioJob job = {};
  copyToBuffer(job.text, sizeof(job.text), msg);
  return xQueueSend(txQueue, &job, 0) == pdPASS;
}

void handleKeyboard() {
  if (!(M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed())) {
    return;
  }

  Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
  for (auto key : status.word) {
    inputBuffer += key;
  }

  if (status.del && !inputBuffer.isEmpty()) {
    inputBuffer.remove(inputBuffer.length() - 1);
  }

  if (status.enter) {
    String msg = inputBuffer;
    msg.trim();
    inputBuffer = "";
    drawPrompt();
    if (!msg.isEmpty()) {
      if (!enqueueTx(msg)) {
        say("System", "TX queue full", RED);
      }
    }
    return;
  }

  drawPrompt();
}

void pumpRadioEvents() {
  if (!eventQueue) {
    return;
  }

  RadioEvent evt;
  while (xQueueReceive(eventQueue, &evt, 0) == pdPASS) {
    switch (evt.type) {
      case RadioEventType::Rx: {
        String meta = String((int)evt.rssi) + "dBm/" + String(evt.snr, 1) + "dB";
        say("RX", String(evt.text) + " (" + meta + ")", GREEN);
        break;
      }
      case RadioEventType::TxOk:
        say("TX", evt.text, CYAN);
        break;
      case RadioEventType::TxErr:
        say("System", String("TX error: ") + evt.status + " (" + evt.text + ")", RED);
        break;
      case RadioEventType::Info:
        say("System", evt.text, ORANGE);
        break;
    }
  }
}

void radioTask(void* arg) {
  setAntenna(true);
  restartReceive("boot");

  for (;;) {
    RadioJob job;
    if (txQueue && xQueueReceive(txQueue, &job, 0) == pdPASS) {
      radio.standby();
      setAntenna(false);
      int txState = radio.transmit(job.text);
      setAntenna(true);
      radio.clearIrqFlags(RADIOLIB_SX126X_IRQ_ALL);
      restartReceive("post-tx");
      if (txState == RADIOLIB_ERR_NONE) {
        queueEvent(RadioEventType::TxOk, String(job.text));
      } else {
        queueEvent(RadioEventType::TxErr, String(job.text), 0, 0, txState);
      }
      continue;
    }

    uint16_t irq = radio.getIrqFlags();
    if (irq & RADIOLIB_SX126X_IRQ_RX_DONE) {
      String payload;
      int rxState = radio.readData(payload);
      radio.clearIrqFlags(RADIOLIB_SX126X_IRQ_RX_DONE |
                          RADIOLIB_SX126X_IRQ_HEADER_ERR |
                          RADIOLIB_SX126X_IRQ_CRC_ERR);
      restartReceive("rx-done");

      if (rxState == RADIOLIB_ERR_NONE && payload.length() > 0) {
        float rssi = radio.getRSSI();
        float snr = radio.getSNR();
        queueEvent(RadioEventType::Rx, payload, rssi, snr);
      } else if (rxState == RADIOLIB_ERR_CRC_MISMATCH) {
        queueEvent(RadioEventType::Info, "RX CRC mismatch", 0, 0, rxState);
      } else if (rxState != RADIOLIB_ERR_NONE) {
        queueEvent(RadioEventType::Info, String("RX read err ") + rxState, 0, 0, rxState);
      }
    } else if (irq & RADIOLIB_SX126X_IRQ_TIMEOUT) {
      radio.clearIrqFlags(RADIOLIB_SX126X_IRQ_TIMEOUT);
      restartReceive("timeout");
    }

    vTaskDelay(pdMS_TO_TICKS(RADIO_TASK_DELAY_MS));
  }
}

void restartReceive(const char* reason) {
  int state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE) {
    String msg = "RX start err";
    if (reason) {
      msg += " (";
      msg += reason;
      msg += ")";
    }
    msg += ": ";
    msg += state;
    queueEvent(RadioEventType::Info, msg, 0, 0, state);
  }
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
    ioe.digitalWrite(7, true);
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
  radio.setDio2AsRfSwitch(false);

  txQueue = xQueueCreate(TX_QUEUE_DEPTH, sizeof(RadioJob));
  eventQueue = xQueueCreate(EVENT_QUEUE_DEPTH, sizeof(RadioEvent));
  if (!txQueue || !eventQueue) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setCursor(4, 4);
    M5Cardputer.Display.setTextColor(RED);
    M5Cardputer.Display.println("queue alloc failed");
    while (true) delay(1000);
  }

  xTaskCreate(radioTask, "LoRaRadio", RADIO_TASK_STACK, nullptr, RADIO_TASK_PRIORITY, &radioTaskHandle);

  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.drawCenterString("LoRa Chat", M5Cardputer.Display.width() / 2, 4);
  M5Cardputer.Display.drawRect(4, 18, M5Cardputer.Display.width() - 8,
                               M5Cardputer.Display.height() - 34, BLUE);

  canvas.createSprite(M5Cardputer.Display.width() - 16,
                      M5Cardputer.Display.height() - 38);
  canvas.setTextScroll(true);
  canvas.fillSprite(BLACK);

  say("System", "Radio ready.", ORANGE);
  drawPrompt();
}

void loop() {
  M5Cardputer.update();
  handleKeyboard();
  pumpRadioEvents();
  delay(5);
}
