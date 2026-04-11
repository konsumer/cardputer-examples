#include <M5Cardputer.h>
#include <SPI.h>
#include <SD.h>

static constexpr int SD_SPI_SCK_PIN  = 40;
static constexpr int SD_SPI_MISO_PIN = 39;
static constexpr int SD_SPI_MOSI_PIN = 14;
static constexpr int SD_SPI_CS_PIN   = 12;
static constexpr const char* TEST_FILE_PATH = "/cardputer-adv-sd-test.txt";

bool beginSDCard() {
  // CardputerADV shared-bus gate for SD/LoRa coexistence.
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  delay(2);

  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  // Fastest known-good path: try 25MHz first, fallback to 16MHz.
  if (SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    return true;
  }

  SD.end();
  delay(10);
  return SD.begin(SD_SPI_CS_PIN, SPI, 16000000);
}

const char* cardTypeToString(uint8_t type) {
  switch (type) {
    case CARD_MMC: return "MMC";
    case CARD_SD: return "SDSC";
    case CARD_SDHC: return "SDHC";
    default: return "UNKNOWN";
  }
}

void listFiles(fs::FS& fs, const char* dirname, uint8_t levels) {
  File root = fs.open(dirname);
  if (!root || !root.isDirectory()) {
    Serial.printf("Cannot open directory: %s\n", dirname);
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.printf("[DIR ] %s\n", file.path());
      if (levels > 0) {
        listFiles(fs, file.path(), levels - 1);
      }
    } else {
      Serial.printf("[FILE] %s (%u bytes)\n", file.path(), static_cast<unsigned>(file.size()));
    }
    file = root.openNextFile();
  }
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  Serial.begin(115200);
  delay(250);

  if (!beginSDCard()) {
    Serial.println("SD init failed");
    return;
  }

  const uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("=== SD INFO ===");
  Serial.printf("Type: %s\n", cardTypeToString(cardType));
  Serial.printf("Size: %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));
  Serial.printf("Total: %llu MB\n", SD.totalBytes() / (1024ULL * 1024ULL));
  Serial.printf("Used: %llu MB\n", SD.usedBytes() / (1024ULL * 1024ULL));

  Serial.println("\n=== FILE LIST ===");
  listFiles(SD, "/", 8);

  Serial.println("\n=== CREATE FILE ===");
  File writeFile = SD.open(TEST_FILE_PATH, FILE_WRITE);
  if (!writeFile) {
    Serial.printf("Failed to create %s\n", TEST_FILE_PATH);
  } else {
    writeFile.println("Hello from CardputerADV SD test.");
    writeFile.println("Create -> Read -> Delete complete.");
    writeFile.close();
    Serial.printf("Created: %s\n", TEST_FILE_PATH);
  }

  Serial.println("\n=== READ FILE ===");
  File readFile = SD.open(TEST_FILE_PATH);
  if (!readFile) {
    Serial.printf("Failed to open %s\n", TEST_FILE_PATH);
  } else {
    while (readFile.available()) {
      Serial.write(readFile.read());
    }
    readFile.close();
    Serial.println();
  }

  Serial.println("=== DELETE FILE ===");
  if (SD.remove(TEST_FILE_PATH)) {
    Serial.printf("Deleted: %s\n", TEST_FILE_PATH);
  } else {
    Serial.printf("Failed to delete %s\n", TEST_FILE_PATH);
  }

  Serial.println("SD demo complete");
}

void loop() {
  delay(1000);
}
