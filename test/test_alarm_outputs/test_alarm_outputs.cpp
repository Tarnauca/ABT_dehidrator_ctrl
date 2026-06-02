#include <unity.h>

#include "dehydrator/config/HardwareConfig.h"
#include "dehydrator/hardware/AlarmOutputs.h"

using dehydrator::AlarmOutputs;
using dehydrator::DigitalOutput;
using dehydrator::OutputCommand;
using dehydrator::config::ActiveLevel;
using dehydrator::config::HardwareConfig;

struct PinWrite {
  uint8_t pin = 0;
  bool high = false;
};

class FakeDigitalOutput : public DigitalOutput {
 public:
  static constexpr uint8_t MAX_CONFIGURED = 8U;
  static constexpr uint8_t MAX_WRITES = 16U;

  uint8_t configuredPins[MAX_CONFIGURED] = {};
  bool configuredInitialHigh[MAX_CONFIGURED] = {};
  uint8_t configuredCount = 0;
  PinWrite writes[MAX_WRITES] = {};
  uint8_t writeCount = 0;

  void configureOutput(uint8_t pin, bool initialHigh) override {
    if (configuredCount < MAX_CONFIGURED) {
      configuredPins[configuredCount] = pin;
      configuredInitialHigh[configuredCount] = initialHigh;
      configuredCount++;
    }
  }

  void write(uint8_t pin, bool high) override {
    if (writeCount < MAX_WRITES) {
      writes[writeCount].pin = pin;
      writes[writeCount].high = high;
      writeCount++;
    }
  }
};

HardwareConfig alarmConfig(ActiveLevel backlightLevel = ActiveLevel::ActiveHigh,
                           ActiveLevel buzzerLevel = ActiveLevel::ActiveHigh) {
  HardwareConfig config = dehydrator::config::HARDWARE;
  config.pins.lcdBacklight = 24U;
  config.pins.buzzer = 25U;
  config.lcdBacklightActiveLevel = backlightLevel;
  config.buzzerActiveLevel = buzzerLevel;
  return config;
}

void test_begin_configures_assigned_alarm_pins_inactive() {
  FakeDigitalOutput digital;
  const HardwareConfig config = alarmConfig();
  AlarmOutputs alarms(digital, config);

  alarms.begin();

  TEST_ASSERT_EQUAL_UINT8(2U, digital.configuredCount);
  TEST_ASSERT_EQUAL_UINT8(24U, digital.configuredPins[0]);
  TEST_ASSERT_FALSE(digital.configuredInitialHigh[0]);
  TEST_ASSERT_EQUAL_UINT8(25U, digital.configuredPins[1]);
  TEST_ASSERT_FALSE(digital.configuredInitialHigh[1]);
}

void test_active_low_begin_uses_inactive_high_initial_levels() {
  FakeDigitalOutput digital;
  const HardwareConfig config =
      alarmConfig(ActiveLevel::ActiveLow, ActiveLevel::ActiveLow);
  AlarmOutputs alarms(digital, config);

  alarms.begin();

  TEST_ASSERT_TRUE(digital.configuredInitialHigh[0]);
  TEST_ASSERT_TRUE(digital.configuredInitialHigh[1]);
}

void test_apply_turns_buzzer_and_backlight_on() {
  FakeDigitalOutput digital;
  const HardwareConfig config = alarmConfig();
  AlarmOutputs alarms(digital, config);
  OutputCommand command;
  command.backlightOn = true;
  command.buzzerOn = true;

  alarms.apply(command);

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_EQUAL_UINT8(24U, digital.writes[0].pin);
  TEST_ASSERT_TRUE(digital.writes[0].high);
  TEST_ASSERT_EQUAL_UINT8(25U, digital.writes[1].pin);
  TEST_ASSERT_TRUE(digital.writes[1].high);
}

void test_active_low_on_writes_low_levels() {
  FakeDigitalOutput digital;
  const HardwareConfig config =
      alarmConfig(ActiveLevel::ActiveLow, ActiveLevel::ActiveLow);
  AlarmOutputs alarms(digital, config);
  OutputCommand command;
  command.backlightOn = true;
  command.buzzerOn = true;

  alarms.apply(command);

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_FALSE(digital.writes[0].high);
  TEST_ASSERT_FALSE(digital.writes[1].high);
}

void test_force_off_drives_alarm_outputs_inactive() {
  FakeDigitalOutput digital;
  const HardwareConfig config = alarmConfig();
  AlarmOutputs alarms(digital, config);

  alarms.forceOff();

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_EQUAL_UINT8(24U, digital.writes[0].pin);
  TEST_ASSERT_FALSE(digital.writes[0].high);
  TEST_ASSERT_EQUAL_UINT8(25U, digital.writes[1].pin);
  TEST_ASSERT_FALSE(digital.writes[1].high);
}

void test_unassigned_alarm_pins_are_not_configured_or_written() {
  FakeDigitalOutput digital;
  HardwareConfig config = alarmConfig();
  config.pins.lcdBacklight = AlarmOutputs::UNASSIGNED_PIN;
  config.pins.buzzer = AlarmOutputs::UNASSIGNED_PIN;
  AlarmOutputs alarms(digital, config);
  OutputCommand command;
  command.backlightOn = true;
  command.buzzerOn = true;

  alarms.begin();
  alarms.apply(command);

  TEST_ASSERT_EQUAL_UINT8(0U, digital.configuredCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digital.writeCount);
}

void test_heater_and_fan_fields_are_ignored_by_alarm_adapter() {
  FakeDigitalOutput digital;
  const HardwareConfig config = alarmConfig();
  AlarmOutputs alarms(digital, config);
  OutputCommand command;
  command.fanOn = true;
  command.heaterOn = true;

  alarms.apply(command);

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_FALSE(digital.writes[0].high);
  TEST_ASSERT_FALSE(digital.writes[1].high);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_begin_configures_assigned_alarm_pins_inactive);
  RUN_TEST(test_active_low_begin_uses_inactive_high_initial_levels);
  RUN_TEST(test_apply_turns_buzzer_and_backlight_on);
  RUN_TEST(test_active_low_on_writes_low_levels);
  RUN_TEST(test_force_off_drives_alarm_outputs_inactive);
  RUN_TEST(test_unassigned_alarm_pins_are_not_configured_or_written);
  RUN_TEST(test_heater_and_fan_fields_are_ignored_by_alarm_adapter);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
