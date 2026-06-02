#include <unity.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/FaultDetector.h"

using dehydrator::FaultCode;
using dehydrator::FaultDetector;
using dehydrator::FaultDetectorInput;
using dehydrator::FaultDetectorResult;
using dehydrator::config::SafetyConfig;

SafetyConfig safetyConfig() { return dehydrator::config::SAFETY; }

FaultDetectorInput normalInput() {
  FaultDetectorInput input;
  input.pt50Valid = true;
  input.pt50TempC = 50;
  input.deltaSeconds = 1;
  return input;
}

void assertFault(FaultDetectorResult result, FaultCode expectedCode) {
  TEST_ASSERT_TRUE(result.hardFault);
  TEST_ASSERT_EQUAL(static_cast<int>(expectedCode),
                    static_cast<int>(result.code));
}

void assertNoFault(FaultDetectorResult result) {
  TEST_ASSERT_FALSE(result.hardFault);
  TEST_ASSERT_EQUAL(static_cast<int>(FaultCode::None),
                    static_cast<int>(result.code));
}

void test_normal_input_has_no_fault() {
  FaultDetector detector;

  assertNoFault(detector.update(safetyConfig(), normalInput()));
}

void test_invalid_pt50_triggers_hard_fault() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.pt50Valid = false;

  assertFault(detector.update(safetyConfig(), input), FaultCode::Pt50Invalid);
}

void test_pt50_below_plausible_range_triggers_hard_fault() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.pt50TempC = safetyConfig().pt50MinValidTempC - 1;

  assertFault(detector.update(safetyConfig(), input), FaultCode::Pt50Invalid);
}

void test_pt50_above_plausible_range_triggers_hard_fault() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.pt50TempC = safetyConfig().pt50MaxValidTempC + 1;

  assertFault(detector.update(safetyConfig(), input), FaultCode::Pt50Invalid);
}

void test_over_temperature_faults_at_80_degrees() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.pt50TempC = safetyConfig().hardFaultTempC;

  assertFault(detector.update(safetyConfig(), input), FaultCode::OverTemperature);
}

void test_temperature_below_80_does_not_over_temperature_fault() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.pt50TempC = safetyConfig().hardFaultTempC - 1;

  assertNoFault(detector.update(safetyConfig(), input));
}

void test_no_rise_fault_after_five_minutes_accumulated_heater_on() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  input.pt50TempC = 40;
  input.deltaSeconds = safetyConfig().noRiseWindowSeconds;

  assertFault(detector.update(safetyConfig(), input),
              FaultCode::TemperatureNotRising);
}

void test_no_rise_does_not_fault_before_window() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  input.pt50TempC = 40;
  input.deltaSeconds = safetyConfig().noRiseWindowSeconds - 1U;

  assertNoFault(detector.update(safetyConfig(), input));
}

void test_no_rise_timer_resets_after_required_temperature_rise() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  input.pt50TempC = 40;
  input.deltaSeconds = safetyConfig().noRiseWindowSeconds - 1U;
  assertNoFault(detector.update(safetyConfig(), input));

  input.pt50TempC = 42;
  input.deltaSeconds = 1;
  assertNoFault(detector.update(safetyConfig(), input));

  input.deltaSeconds = safetyConfig().noRiseWindowSeconds - 1U;
  assertNoFault(detector.update(safetyConfig(), input));
}

void test_no_rise_accumulates_heater_on_time_across_relay_cycles() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  input.pt50TempC = 40;
  input.deltaSeconds = 150;
  assertNoFault(detector.update(safetyConfig(), input));

  input.heaterCommandOn = false;
  input.deltaSeconds = 100;
  assertNoFault(detector.update(safetyConfig(), input));

  input.heaterCommandOn = true;
  input.deltaSeconds = 150;

  assertFault(detector.update(safetyConfig(), input),
              FaultCode::TemperatureNotRising);
}

void test_stuck_heater_does_not_monitor_before_heater_was_on() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = false;
  input.pt50TempC = 50;
  input.deltaSeconds = safetyConfig().stuckHeaterGraceSeconds;
  assertNoFault(detector.update(safetyConfig(), input));

  input.pt50TempC = 60;
  input.deltaSeconds = safetyConfig().stuckHeaterWindowSeconds;
  assertNoFault(detector.update(safetyConfig(), input));
}

void test_stuck_heater_does_not_fault_during_grace_period() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  assertNoFault(detector.update(safetyConfig(), input));

  input.heaterCommandOn = false;
  input.pt50TempC = 50;
  input.deltaSeconds = safetyConfig().stuckHeaterGraceSeconds - 1U;

  assertNoFault(detector.update(safetyConfig(), input));
}

void test_stuck_heater_faults_after_grace_when_temperature_rises() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  assertNoFault(detector.update(safetyConfig(), input));

  input.heaterCommandOn = false;
  input.pt50TempC = 50;
  input.deltaSeconds = safetyConfig().stuckHeaterGraceSeconds;
  assertNoFault(detector.update(safetyConfig(), input));

  input.pt50TempC = 53;
  input.deltaSeconds = safetyConfig().stuckHeaterWindowSeconds;

  assertFault(detector.update(safetyConfig(), input),
              FaultCode::HeaterStuckOnSuspected);
}

void test_stuck_heater_monitoring_resets_when_heater_turns_on() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  assertNoFault(detector.update(safetyConfig(), input));

  input.heaterCommandOn = false;
  input.pt50TempC = 50;
  input.deltaSeconds = safetyConfig().stuckHeaterGraceSeconds;
  assertNoFault(detector.update(safetyConfig(), input));

  input.heaterCommandOn = true;
  input.pt50TempC = 53;
  input.deltaSeconds = 1;

  assertNoFault(detector.update(safetyConfig(), input));
}

void test_stuck_heater_monitoring_rebaselines_after_window_without_fault() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.heaterCommandOn = true;
  assertNoFault(detector.update(safetyConfig(), input));

  input.heaterCommandOn = false;
  input.pt50TempC = 50;
  input.deltaSeconds = safetyConfig().stuckHeaterGraceSeconds;
  assertNoFault(detector.update(safetyConfig(), input));

  input.pt50TempC = 52;
  input.deltaSeconds = safetyConfig().stuckHeaterWindowSeconds;
  assertNoFault(detector.update(safetyConfig(), input));

  input.pt50TempC = 55;
  input.deltaSeconds = safetyConfig().stuckHeaterWindowSeconds;
  assertFault(detector.update(safetyConfig(), input),
              FaultCode::HeaterStuckOnSuspected);
}

void test_button_stuck_faults_after_30_seconds() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.buttonActive = true;
  input.deltaSeconds = safetyConfig().buttonStuckSeconds;

  assertFault(detector.update(safetyConfig(), input), FaultCode::ButtonStuck);
}

void test_button_timer_resets_when_button_released() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.buttonActive = true;
  input.deltaSeconds = safetyConfig().buttonStuckSeconds - 1U;
  assertNoFault(detector.update(safetyConfig(), input));

  input.buttonActive = false;
  input.deltaSeconds = 1;
  assertNoFault(detector.update(safetyConfig(), input));

  input.buttonActive = true;
  input.deltaSeconds = 1;
  assertNoFault(detector.update(safetyConfig(), input));
}

void test_watchdog_reset_during_run_faults() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.watchdogResetDuringRun = true;

  assertFault(detector.update(safetyConfig(), input),
              FaultCode::WatchdogResetDuringRun);
}

void test_first_fault_is_latched_until_reset() {
  FaultDetector detector;
  FaultDetectorInput input = normalInput();
  input.pt50Valid = false;
  assertFault(detector.update(safetyConfig(), input), FaultCode::Pt50Invalid);

  input = normalInput();
  input.pt50TempC = safetyConfig().hardFaultTempC;
  assertFault(detector.update(safetyConfig(), input), FaultCode::Pt50Invalid);

  detector.reset();
  assertFault(detector.update(safetyConfig(), input), FaultCode::OverTemperature);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_normal_input_has_no_fault);
  RUN_TEST(test_invalid_pt50_triggers_hard_fault);
  RUN_TEST(test_pt50_below_plausible_range_triggers_hard_fault);
  RUN_TEST(test_pt50_above_plausible_range_triggers_hard_fault);
  RUN_TEST(test_over_temperature_faults_at_80_degrees);
  RUN_TEST(test_temperature_below_80_does_not_over_temperature_fault);
  RUN_TEST(test_no_rise_fault_after_five_minutes_accumulated_heater_on);
  RUN_TEST(test_no_rise_does_not_fault_before_window);
  RUN_TEST(test_no_rise_timer_resets_after_required_temperature_rise);
  RUN_TEST(test_no_rise_accumulates_heater_on_time_across_relay_cycles);
  RUN_TEST(test_stuck_heater_does_not_monitor_before_heater_was_on);
  RUN_TEST(test_stuck_heater_does_not_fault_during_grace_period);
  RUN_TEST(test_stuck_heater_faults_after_grace_when_temperature_rises);
  RUN_TEST(test_stuck_heater_monitoring_resets_when_heater_turns_on);
  RUN_TEST(test_stuck_heater_monitoring_rebaselines_after_window_without_fault);
  RUN_TEST(test_button_stuck_faults_after_30_seconds);
  RUN_TEST(test_button_timer_resets_when_button_released);
  RUN_TEST(test_watchdog_reset_during_run_faults);
  RUN_TEST(test_first_fault_is_latched_until_reset);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
