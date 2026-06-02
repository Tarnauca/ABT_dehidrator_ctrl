#include <unity.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/Pt50SensorModel.h"
#include "dehydrator/interfaces/AnalogInput.h"
#include "dehydrator/sensors/Pt50Reader.h"

using dehydrator::AnalogInput;
using dehydrator::Pt50Reader;
using dehydrator::Pt50Reading;
using dehydrator::Pt50SensorModel;
using dehydrator::config::CalibrationConfig;
using dehydrator::config::DividerOrientation;

class FakeAnalogInput : public AnalogInput {
 public:
  uint8_t lastChannel = 255U;
  uint16_t nextAdc = 0;

  uint16_t read(uint8_t channel) override {
    lastChannel = channel;
    return nextAdc;
  }
};

CalibrationConfig calibration() { return dehydrator::config::CALIBRATION; }

uint16_t adcForResistanceFixedHigh(const CalibrationConfig& config,
                                   int32_t resistanceMilliOhms) {
  const int64_t numerator =
      static_cast<int64_t>(config.adcMaxCount) * resistanceMilliOhms;
  const int64_t denominator =
      static_cast<int64_t>(config.pt50FixedResistorMilliOhms) +
      resistanceMilliOhms;
  return static_cast<uint16_t>((numerator + (denominator / 2)) / denominator);
}

int32_t resistanceAtTempC(const CalibrationConfig& config, int16_t tempC) {
  const int64_t delta =
      (static_cast<int64_t>(config.pt50NominalMilliOhms) *
       config.pt50AlphaPpmPerC * tempC) /
      1000000L;
  return static_cast<int32_t>(config.pt50NominalMilliOhms + delta);
}

void test_nominal_pt50_resistance_converts_to_zero_celsius() {
  const CalibrationConfig config = calibration();
  const uint16_t adc =
      adcForResistanceFixedHigh(config, config.pt50NominalMilliOhms);

  const Pt50Reading reading = Pt50SensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 0, reading.tempC);
  TEST_ASSERT_INT32_WITHIN(200, config.pt50NominalMilliOhms,
                           reading.resistanceMilliOhms);
}

void test_higher_pt50_resistance_converts_to_about_100_celsius() {
  const CalibrationConfig config = calibration();
  const int32_t resistanceAt100C = resistanceAtTempC(config, 100);
  const uint16_t adc = adcForResistanceFixedHigh(config, resistanceAt100C);

  const Pt50Reading reading = Pt50SensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 100, reading.tempC);
}

void test_offset_calibration_adjusts_temperature() {
  CalibrationConfig config = calibration();
  config.pt50OffsetCentiC = 250;
  const uint16_t adc =
      adcForResistanceFixedHigh(config, config.pt50NominalMilliOhms);

  const Pt50Reading reading = Pt50SensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 3, reading.tempC);
}

void test_scale_calibration_adjusts_temperature_span() {
  CalibrationConfig config = calibration();
  config.pt50ScalePpm = 1100000;
  const int32_t resistanceAt50C = resistanceAtTempC(config, 50);
  const uint16_t adc = adcForResistanceFixedHigh(config, resistanceAt50C);

  const Pt50Reading reading = Pt50SensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 55, reading.tempC);
}

void test_zero_adc_is_invalid() {
  const Pt50Reading reading = Pt50SensorModel::convert(calibration(), 0);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_max_adc_is_invalid() {
  const CalibrationConfig config = calibration();

  const Pt50Reading reading =
      Pt50SensorModel::convert(config, config.adcMaxCount);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_near_max_adc_is_invalid_without_temperature_wraparound() {
  CalibrationConfig config = calibration();

  const Pt50Reading reading =
      Pt50SensorModel::convert(config, config.adcMaxCount - 8U);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_out_of_plausible_temperature_range_is_invalid() {
  CalibrationConfig config = calibration();
  config.pt50MaxValidTempC = 60;
  const int32_t resistanceAt100C = resistanceAtTempC(config, 100);
  const uint16_t adc = adcForResistanceFixedHigh(config, resistanceAt100C);

  const Pt50Reading reading = Pt50SensorModel::convert(config, adc);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_pt50_high_side_orientation_is_supported() {
  CalibrationConfig config = calibration();
  config.pt50DividerOrientation = DividerOrientation::Pt50HighFixedLow;
  const uint16_t adc = static_cast<uint16_t>(
      (static_cast<uint32_t>(config.adcMaxCount) *
       config.pt50FixedResistorMilliOhms) /
      (config.pt50FixedResistorMilliOhms + config.pt50NominalMilliOhms));

  const Pt50Reading reading = Pt50SensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 0, reading.tempC);
}

void test_pt50_high_side_orientation_high_temperature_direction() {
  CalibrationConfig config = calibration();
  config.pt50DividerOrientation = DividerOrientation::Pt50HighFixedLow;
  const int32_t resistanceAt100C = resistanceAtTempC(config, 100);
  const uint16_t adc = static_cast<uint16_t>(
      (static_cast<int64_t>(config.adcMaxCount) *
       config.pt50FixedResistorMilliOhms) /
      (config.pt50FixedResistorMilliOhms + resistanceAt100C));

  const Pt50Reading reading = Pt50SensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 100, reading.tempC);
}

void test_pt50_high_side_orientation_near_zero_adc_is_invalid() {
  CalibrationConfig config = calibration();
  config.pt50DividerOrientation = DividerOrientation::Pt50HighFixedLow;

  const Pt50Reading reading = Pt50SensorModel::convert(config, 8U);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_reader_uses_configured_channel_and_conversion_model() {
  FakeAnalogInput analog;
  CalibrationConfig config = calibration();
  analog.nextAdc = adcForResistanceFixedHigh(config, config.pt50NominalMilliOhms);
  Pt50Reader reader(analog, 7U, config);

  const Pt50Reading reading = reader.read();

  TEST_ASSERT_EQUAL_UINT8(7U, analog.lastChannel);
  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 0, reading.tempC);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_nominal_pt50_resistance_converts_to_zero_celsius);
  RUN_TEST(test_higher_pt50_resistance_converts_to_about_100_celsius);
  RUN_TEST(test_offset_calibration_adjusts_temperature);
  RUN_TEST(test_scale_calibration_adjusts_temperature_span);
  RUN_TEST(test_zero_adc_is_invalid);
  RUN_TEST(test_max_adc_is_invalid);
  RUN_TEST(test_near_max_adc_is_invalid_without_temperature_wraparound);
  RUN_TEST(test_out_of_plausible_temperature_range_is_invalid);
  RUN_TEST(test_pt50_high_side_orientation_is_supported);
  RUN_TEST(test_pt50_high_side_orientation_high_temperature_direction);
  RUN_TEST(test_pt50_high_side_orientation_near_zero_adc_is_invalid);
  RUN_TEST(test_reader_uses_configured_channel_and_conversion_model);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
