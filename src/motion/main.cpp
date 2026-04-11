#include <M5Cardputer.h>

m5::imu_data_t imuData;

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);  // enableKeyboard
  M5Cardputer.Display.setFont(&fonts::FreeMonoBold9pt7b);

  M5Cardputer.Display.clear();
  M5Cardputer.Display.setCursor(0, 0);
  M5Cardputer.Display.print("    IMU Live Data");
}

void loop() {
  // Use M5 for Imu class, not M5Cardputer
  M5.Imu.update();
  imuData = M5.Imu.getImuData();

  M5Cardputer.Display.setCursor(0, 30);
  M5Cardputer.Display.println("      Acc    Gyr");

  M5Cardputer.Display.printf("  x  % 4.2f  % 4.2f\n", imuData.accel.x, imuData.gyro.x);
  M5Cardputer.Display.printf("  y  % 4.2f  % 4.2f\n", imuData.accel.y, imuData.gyro.y);
  M5Cardputer.Display.printf("  z  % 4.2f  % 4.2f\n", imuData.accel.z, imuData.gyro.z);

  delay(100);
}