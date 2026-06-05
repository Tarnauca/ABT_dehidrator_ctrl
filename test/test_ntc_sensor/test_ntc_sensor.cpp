#include <math.h>
#include <unity.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/NtcSensorModel.h"
#include "dehydrator/interfaces/AnalogInput.h"
#include "dehydrator/sensors/NtcReader.h"

using dehydrator::AnalogInput;
using dehydrator::NtcReader;
using dehydrator::NtcReading;
using dehydrator::NtcSensorModel;
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
      static_cast<int64_t>(config.ntcFixedResistorMilliOhms) +
      resistanceMilliOhms;
  return static_cast<uint16_t>((numerator + (denominator / 2)) / denominator);
}

int32_t resistanceAtTempC(const CalibrationConfig& config, int16_t tempC) {
  const float nominalResistance =
      static_cast<float>(config.ntcNominalMilliOhms);
  const float nominalTempK =
      static_cast<float>(config.ntcNominalTempCentiC) / 100.0f + 273.15f;
  const float tempK = static_cast<float>(tempC) + 273.15f;
  const float resistance =
      nominalResistance *
      expf(static_cast<float>(config.ntcBetaK) *
           ((1.0f / tempK) - (1.0f / nominalTempK)));
  return static_cast<int32_t>(resistance + 0.5f);
}

void test_nominal_resistance_converts_to_nominal_temperature() {
  const CalibrationConfig config = calibration();
  const uint16_t adc =
      adcForResistanceFixedHigh(config, config.ntcNominalMilliOhms);

  const NtcReading reading = NtcSensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 250, reading.tempDeciC);
  TEST_ASSERT_INT32_WITHIN(250000, config.ntcNominalMilliOhms,
                           reading.resistanceMilliOhms);
}

void test_lower_resistance_converts_to_higher_temperature() {
  const CalibrationConfig config = calibration();
  const int32_t resistanceAt100C = resistanceAtTempC(config, 100);
  const uint16_t adc = adcForResistanceFixedHigh(config, resistanceAt100C);

  const NtcReading reading = NtcSensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(2, 1000, reading.tempDeciC);
}

void test_offset_calibration_adjusts_temperature() {
  CalibrationConfig config = calibration();
  config.ntcOffsetCentiC = 250;
  const uint16_t adc =
      adcForResistanceFixedHigh(config, config.ntcNominalMilliOhms);

  const NtcReading reading = NtcSensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 275, reading.tempDeciC);
}

void test_scale_calibration_adjusts_temperature_span() {
  CalibrationConfig config = calibration();
  config.ntcScalePpm = 1100000;
  const int32_t resistanceAt75C = resistanceAtTempC(config, 75);
  const uint16_t adc = adcForResistanceFixedHigh(config, resistanceAt75C);

  const NtcReading reading = NtcSensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 825, reading.tempDeciC);
}

void test_zero_adc_is_invalid() {
  const NtcReading reading = NtcSensorModel::convert(calibration(), 0);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_max_adc_is_invalid() {
  const CalibrationConfig config = calibration();

  const NtcReading reading =
      NtcSensorModel::convert(config, config.adcMaxCount);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_near_max_adc_is_invalid_without_temperature_wraparound() {
  CalibrationConfig config = calibration();

  const NtcReading reading =
      NtcSensorModel::convert(config, config.adcMaxCount - 8U);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_out_of_plausible_temperature_range_is_invalid() {
  CalibrationConfig config = calibration();
  config.ntcMaxValidTempC = 60;
  const int32_t resistanceAt100C = resistanceAtTempC(config, 100);
  const uint16_t adc = adcForResistanceFixedHigh(config, resistanceAt100C);

  const NtcReading reading = NtcSensorModel::convert(config, adc);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_ntc_high_side_orientation_is_supported() {
  CalibrationConfig config = calibration();
  config.ntcDividerOrientation = DividerOrientation::NtcHighFixedLow;
  const uint16_t adc = static_cast<uint16_t>(
      (static_cast<int64_t>(config.adcMaxCount) *
       config.ntcFixedResistorMilliOhms) /
      (config.ntcFixedResistorMilliOhms + config.ntcNominalMilliOhms));

  const NtcReading reading = NtcSensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 250, reading.tempDeciC);
}

void test_ntc_high_side_orientation_high_temperature_direction() {
  CalibrationConfig config = calibration();
  config.ntcDividerOrientation = DividerOrientation::NtcHighFixedLow;
  const int32_t resistanceAt100C = resistanceAtTempC(config, 100);
  const uint16_t adc = static_cast<uint16_t>(
      (static_cast<int64_t>(config.adcMaxCount) *
       config.ntcFixedResistorMilliOhms) /
      (config.ntcFixedResistorMilliOhms + resistanceAt100C));

  const NtcReading reading = NtcSensorModel::convert(config, adc);

  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(2, 1000, reading.tempDeciC);
}

void test_ntc_high_side_orientation_near_zero_adc_is_invalid() {
  CalibrationConfig config = calibration();
  config.ntcDividerOrientation = DividerOrientation::NtcHighFixedLow;

  const NtcReading reading = NtcSensorModel::convert(config, 8U);

  TEST_ASSERT_FALSE(reading.valid);
}

void test_reader_uses_configured_channel_and_conversion_model() {
  FakeAnalogInput analog;
  CalibrationConfig config = calibration();
  analog.nextAdc = adcForResistanceFixedHigh(config, config.ntcNominalMilliOhms);
  NtcReader reader(analog, 7U, config);

  const NtcReading reading = reader.read();

  TEST_ASSERT_EQUAL_UINT8(7U, analog.lastChannel);
  TEST_ASSERT_TRUE(reading.valid);
  TEST_ASSERT_INT_WITHIN(1, 250, reading.tempDeciC);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_nominal_resistance_converts_to_nominal_temperature);
  RUN_TEST(test_lower_resistance_converts_to_higher_temperature);
  RUN_TEST(test_offset_calibration_adjusts_temperature);
  RUN_TEST(test_scale_calibration_adjusts_temperature_span);
  RUN_TEST(test_zero_adc_is_invalid);
  RUN_TEST(test_max_adc_is_invalid);
  RUN_TEST(test_near_max_adc_is_invalid_without_temperature_wraparound);
  RUN_TEST(test_out_of_plausible_temperature_range_is_invalid);
  RUN_TEST(test_ntc_high_side_orientation_is_supported);
  RUN_TEST(test_ntc_high_side_orientation_high_temperature_direction);
  RUN_TEST(test_ntc_high_side_orientation_near_zero_adc_is_invalid);
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
