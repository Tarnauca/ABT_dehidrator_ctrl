#include <unity.h>

#include "dehydrator/config/HardwareConfig.h"
#include "dehydrator/config/RuntimeConfig.h"

void test_serial_defaults_to_required_baud_rate() {
  TEST_ASSERT_EQUAL_UINT32(115200UL,
                           dehydrator::config::SERIAL_PORTS.baudRate);
}

void test_logging_supports_two_initial_sinks() {
  TEST_ASSERT_EQUAL_UINT(2U, dehydrator::config::LOGGING.sinkCapacity);
}

void test_scheduler_state_log_interval_matches_requirement() {
  TEST_ASSERT_EQUAL_UINT32(5000UL,
                           dehydrator::config::SCHEDULER.stateLogIntervalMs);
}

void test_scheduler_sensor_and_lcd_intervals_match_bringup_needs() {
  TEST_ASSERT_EQUAL_UINT32(1000UL,
                           dehydrator::config::SCHEDULER.sensorSampleIntervalMs);
  TEST_ASSERT_EQUAL_UINT32(2000UL,
                           dehydrator::config::SCHEDULER.tempRhSampleIntervalMs);
  TEST_ASSERT_EQUAL_UINT32(1000UL,
                           dehydrator::config::SCHEDULER.lcdRefreshIntervalMs);
  TEST_ASSERT_EQUAL_UINT32(20UL,
                           dehydrator::config::SCHEDULER.inputScanIntervalMs);
}

void test_hardware_uses_status_led_placeholder() {
  TEST_ASSERT_EQUAL_UINT8(13U, dehydrator::config::HARDWARE.pins.statusLed);
}

void test_control_force_off_threshold_matches_requirement() {
  TEST_ASSERT_EQUAL_INT16(
      75, dehydrator::config::CONTROL.heaterForceOffAboveTempC);
}

void test_control_minimum_relay_timing_matches_baseline() {
  TEST_ASSERT_EQUAL_UINT16(10U,
                           dehydrator::config::CONTROL.minHeaterOnSeconds);
  TEST_ASSERT_EQUAL_UINT16(10U,
                           dehydrator::config::CONTROL.minHeaterOffSeconds);
}

void test_safety_fault_thresholds_match_requirements() {
  TEST_ASSERT_EQUAL_INT16(80, dehydrator::config::SAFETY.hardFaultTempC);
  TEST_ASSERT_EQUAL_INT16(2, dehydrator::config::SAFETY.noRiseMinIncreaseC);
  TEST_ASSERT_EQUAL_UINT16(5U * 60U,
                           dehydrator::config::SAFETY.noRiseWindowSeconds);
  TEST_ASSERT_EQUAL_UINT16(
      2U * 60U, dehydrator::config::SAFETY.stuckHeaterGraceSeconds);
  TEST_ASSERT_EQUAL_INT16(3, dehydrator::config::SAFETY.stuckHeaterRiseC);
  TEST_ASSERT_EQUAL_UINT16(
      5U * 60U, dehydrator::config::SAFETY.stuckHeaterWindowSeconds);
  TEST_ASSERT_EQUAL_UINT16(30U,
                           dehydrator::config::SAFETY.buttonStuckSeconds);
}

void test_pt50_calibration_defaults_are_explicit() {
  TEST_ASSERT_EQUAL_INT32(100000,
                          dehydrator::config::CALIBRATION
                              .pt50FixedResistorMilliOhms);
  TEST_ASSERT_EQUAL_INT32(50000,
                          dehydrator::config::CALIBRATION.pt50NominalMilliOhms);
  TEST_ASSERT_EQUAL_INT32(3850,
                          dehydrator::config::CALIBRATION.pt50AlphaPpmPerC);
  TEST_ASSERT_EQUAL_UINT16(1023U,
                           dehydrator::config::CALIBRATION.adcMaxCount);
  TEST_ASSERT_EQUAL(
      static_cast<int>(dehydrator::config::DividerOrientation::FixedHighPt50Low),
      static_cast<int>(
          dehydrator::config::CALIBRATION.pt50DividerOrientation));
}

void test_alarm_output_polarity_defaults_are_explicit() {
  TEST_ASSERT_EQUAL(
      static_cast<int>(dehydrator::config::ActiveLevel::ActiveHigh),
      static_cast<int>(
          dehydrator::config::HARDWARE.lcdBacklightActiveLevel));
  TEST_ASSERT_EQUAL(
      static_cast<int>(dehydrator::config::ActiveLevel::ActiveHigh),
      static_cast<int>(dehydrator::config::HARDWARE.buzzerActiveLevel));
}

void test_temp_rh_calibration_defaults_are_explicit() {
  TEST_ASSERT_EQUAL_INT16(0,
                          dehydrator::config::CALIBRATION.tempRhTempOffsetCentiC);
  TEST_ASSERT_EQUAL_INT16(
      0, dehydrator::config::CALIBRATION.tempRhRhOffsetCentiPercent);
  TEST_ASSERT_EQUAL_INT16(-40,
                          dehydrator::config::CALIBRATION.tempRhMinValidTempC);
  TEST_ASSERT_EQUAL_INT16(85,
                          dehydrator::config::CALIBRATION.tempRhMaxValidTempC);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_serial_defaults_to_required_baud_rate);
  RUN_TEST(test_logging_supports_two_initial_sinks);
  RUN_TEST(test_scheduler_state_log_interval_matches_requirement);
  RUN_TEST(test_scheduler_sensor_and_lcd_intervals_match_bringup_needs);
  RUN_TEST(test_hardware_uses_status_led_placeholder);
  RUN_TEST(test_control_force_off_threshold_matches_requirement);
  RUN_TEST(test_control_minimum_relay_timing_matches_baseline);
  RUN_TEST(test_safety_fault_thresholds_match_requirements);
  RUN_TEST(test_pt50_calibration_defaults_are_explicit);
  RUN_TEST(test_alarm_output_polarity_defaults_are_explicit);
  RUN_TEST(test_temp_rh_calibration_defaults_are_explicit);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
