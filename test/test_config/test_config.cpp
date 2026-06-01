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

void test_hardware_uses_status_led_placeholder() {
  TEST_ASSERT_EQUAL_UINT8(13U, dehydrator::config::HARDWARE.pins.statusLed);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_serial_defaults_to_required_baud_rate);
  RUN_TEST(test_logging_supports_two_initial_sinks);
  RUN_TEST(test_scheduler_state_log_interval_matches_requirement);
  RUN_TEST(test_hardware_uses_status_led_placeholder);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
