#include <unity.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/interfaces/TempRhSensorDriver.h"
#include "dehydrator/sensors/TempRhReader.h"

using dehydrator::TempRhRawSample;
using dehydrator::TempRhReader;
using dehydrator::TempRhReading;
using dehydrator::TempRhSensorDriver;
using dehydrator::config::CalibrationConfig;

class FakeTempRhDriver : public TempRhSensorDriver {
 public:
  TempRhRawSample nextSample;
  uint8_t readCount = 0;

  TempRhRawSample readSample() override {
    readCount++;
    return nextSample;
  }
};

CalibrationConfig calibration() { return dehydrator::config::CALIBRATION; }

void test_valid_temp_rh_sample_returns_temperature_and_rh() {
  FakeTempRhDriver driver;
  driver.nextSample.tempCentiC = 2350;
  driver.nextSample.rhCentiPercent = 4250;
  driver.nextSample.valid = true;
  TempRhReader reader(driver, calibration());

  const TempRhReading reading = reader.read();

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_EQUAL_INT16(24, reading.tempC);
  TEST_ASSERT_EQUAL_UINT8(43U, reading.rhPercent);
  TEST_ASSERT_EQUAL_UINT8(1U, driver.readCount);
}

void test_invalid_driver_sample_returns_invalid_reading() {
  FakeTempRhDriver driver;
  driver.nextSample.valid = false;
  TempRhReader reader(driver, calibration());

  const TempRhReading reading = reader.read();

  TEST_ASSERT_FALSE(reading.valid);
}

void test_temperature_offset_is_applied() {
  FakeTempRhDriver driver;
  driver.nextSample.tempCentiC = 2000;
  driver.nextSample.rhCentiPercent = 5000;
  driver.nextSample.valid = true;
  CalibrationConfig config = calibration();
  config.tempRhTempOffsetCentiC = 150;
  TempRhReader reader(driver, config);

  const TempRhReading reading = reader.read();

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_EQUAL_INT16(22, reading.tempC);
}

void test_rh_offset_is_applied() {
  FakeTempRhDriver driver;
  driver.nextSample.tempCentiC = 2000;
  driver.nextSample.rhCentiPercent = 5000;
  driver.nextSample.valid = true;
  CalibrationConfig config = calibration();
  config.tempRhRhOffsetCentiPercent = -250;
  TempRhReader reader(driver, config);

  const TempRhReading reading = reader.read();

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_EQUAL_UINT8(48U, reading.rhPercent);
}

void test_temperature_below_plausible_range_is_invalid() {
  FakeTempRhDriver driver;
  driver.nextSample.tempCentiC = -4100;
  driver.nextSample.rhCentiPercent = 5000;
  driver.nextSample.valid = true;
  TempRhReader reader(driver, calibration());

  const TempRhReading reading = reader.read();

  TEST_ASSERT_FALSE(reading.valid);
}

void test_temperature_above_plausible_range_is_invalid() {
  FakeTempRhDriver driver;
  driver.nextSample.tempCentiC = 8600;
  driver.nextSample.rhCentiPercent = 5000;
  driver.nextSample.valid = true;
  TempRhReader reader(driver, calibration());

  const TempRhReading reading = reader.read();

  TEST_ASSERT_FALSE(reading.valid);
}

void test_rh_above_100_percent_is_invalid() {
  FakeTempRhDriver driver;
  driver.nextSample.tempCentiC = 2000;
  driver.nextSample.rhCentiPercent = 10050;
  driver.nextSample.valid = true;
  TempRhReader reader(driver, calibration());

  const TempRhReading reading = reader.read();

  TEST_ASSERT_FALSE(reading.valid);
}

void test_negative_calibrated_rh_is_invalid() {
  FakeTempRhDriver driver;
  driver.nextSample.tempCentiC = 2000;
  driver.nextSample.rhCentiPercent = 100;
  driver.nextSample.valid = true;
  CalibrationConfig config = calibration();
  config.tempRhRhOffsetCentiPercent = -200;
  TempRhReader reader(driver, config);

  const TempRhReading reading = reader.read();

  TEST_ASSERT_FALSE(reading.valid);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_valid_temp_rh_sample_returns_temperature_and_rh);
  RUN_TEST(test_invalid_driver_sample_returns_invalid_reading);
  RUN_TEST(test_temperature_offset_is_applied);
  RUN_TEST(test_rh_offset_is_applied);
  RUN_TEST(test_temperature_below_plausible_range_is_invalid);
  RUN_TEST(test_temperature_above_plausible_range_is_invalid);
  RUN_TEST(test_rh_above_100_percent_is_invalid);
  RUN_TEST(test_negative_calibrated_rh_is_invalid);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
