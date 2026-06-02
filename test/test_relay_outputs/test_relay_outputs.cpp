#include <unity.h>

#include "dehydrator/config/HardwareConfig.h"
#include "dehydrator/hardware/RelayOutputs.h"

using dehydrator::DigitalOutput;
using dehydrator::OutputCommand;
using dehydrator::RelayOutputs;
using dehydrator::config::ActiveLevel;
using dehydrator::config::HardwareConfig;

struct PinWrite {
  uint8_t pin = 0;
  bool high = false;
};

enum class OperationType {
  Configure,
  Write,
};

struct DigitalOperation {
  OperationType type = OperationType::Write;
  uint8_t pin = 0;
  bool high = false;
};

class FakeDigitalOutput : public DigitalOutput {
 public:
  static constexpr uint8_t MAX_CONFIGURED = 8U;
  static constexpr uint8_t MAX_WRITES = 16U;
  static constexpr uint8_t MAX_OPERATIONS = 24U;

  uint8_t configuredPins[MAX_CONFIGURED] = {};
  bool configuredInitialHigh[MAX_CONFIGURED] = {};
  uint8_t configuredCount = 0;
  PinWrite writes[MAX_WRITES] = {};
  uint8_t writeCount = 0;
  DigitalOperation operations[MAX_OPERATIONS] = {};
  uint8_t operationCount = 0;

  void configureOutput(uint8_t pin, bool initialHigh) override {
    if (configuredCount < MAX_CONFIGURED) {
      configuredPins[configuredCount] = pin;
      configuredInitialHigh[configuredCount] = initialHigh;
      configuredCount++;
    }
    record(OperationType::Configure, pin, initialHigh);
  }

  void write(uint8_t pin, bool high) override {
    if (writeCount < MAX_WRITES) {
      writes[writeCount].pin = pin;
      writes[writeCount].high = high;
      writeCount++;
    }
    record(OperationType::Write, pin, high);
  }

 private:
  void record(OperationType type, uint8_t pin, bool high) {
    if (operationCount < MAX_OPERATIONS) {
      operations[operationCount].type = type;
      operations[operationCount].pin = pin;
      operations[operationCount].high = high;
      operationCount++;
    }
  }
};

HardwareConfig relayConfig(ActiveLevel heaterLevel = ActiveLevel::ActiveHigh,
                           ActiveLevel fanLevel = ActiveLevel::ActiveHigh) {
  HardwareConfig config = dehydrator::config::HARDWARE;
  config.pins.heaterRelay = 22U;
  config.pins.fanRelay = 23U;
  config.heaterRelayActiveLevel = heaterLevel;
  config.fanRelayActiveLevel = fanLevel;
  return config;
}

void test_begin_configures_assigned_relays_with_inactive_initial_levels() {
  FakeDigitalOutput digital;
  const HardwareConfig config = relayConfig();
  RelayOutputs relays(digital, config);

  relays.begin();

  TEST_ASSERT_EQUAL_UINT8(2U, digital.configuredCount);
  TEST_ASSERT_EQUAL_UINT8(22U, digital.configuredPins[0]);
  TEST_ASSERT_FALSE(digital.configuredInitialHigh[0]);
  TEST_ASSERT_EQUAL_UINT8(23U, digital.configuredPins[1]);
  TEST_ASSERT_FALSE(digital.configuredInitialHigh[1]);
  TEST_ASSERT_EQUAL_UINT8(0U, digital.writeCount);
  TEST_ASSERT_EQUAL(static_cast<int>(OperationType::Configure),
                    static_cast<int>(digital.operations[0].type));
  TEST_ASSERT_EQUAL_UINT8(22U, digital.operations[0].pin);
  TEST_ASSERT_FALSE(digital.operations[0].high);
}

void test_active_low_begin_uses_inactive_high_initial_levels() {
  FakeDigitalOutput digital;
  const HardwareConfig config =
      relayConfig(ActiveLevel::ActiveLow, ActiveLevel::ActiveLow);
  RelayOutputs relays(digital, config);

  relays.begin();

  TEST_ASSERT_EQUAL_UINT8(2U, digital.configuredCount);
  TEST_ASSERT_TRUE(digital.configuredInitialHigh[0]);
  TEST_ASSERT_TRUE(digital.configuredInitialHigh[1]);
}

void test_active_low_polarity_inverts_off_levels() {
  FakeDigitalOutput digital;
  const HardwareConfig config =
      relayConfig(ActiveLevel::ActiveLow, ActiveLevel::ActiveLow);
  RelayOutputs relays(digital, config);

  relays.forceOff();

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_TRUE(digital.writes[0].high);
  TEST_ASSERT_TRUE(digital.writes[1].high);
}

void test_active_low_heater_on_writes_fan_low_before_heater_low() {
  FakeDigitalOutput digital;
  const HardwareConfig config =
      relayConfig(ActiveLevel::ActiveLow, ActiveLevel::ActiveLow);
  RelayOutputs relays(digital, config);
  OutputCommand command;
  command.fanOn = true;
  command.heaterOn = true;

  relays.apply(command);

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_EQUAL_UINT8(23U, digital.writes[0].pin);
  TEST_ASSERT_FALSE(digital.writes[0].high);
  TEST_ASSERT_EQUAL_UINT8(22U, digital.writes[1].pin);
  TEST_ASSERT_FALSE(digital.writes[1].high);
}

void test_heater_on_command_writes_fan_on_before_heater_on() {
  FakeDigitalOutput digital;
  const HardwareConfig config = relayConfig();
  RelayOutputs relays(digital, config);
  OutputCommand command;
  command.fanOn = true;
  command.heaterOn = true;

  relays.apply(command);

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_EQUAL_UINT8(23U, digital.writes[0].pin);
  TEST_ASSERT_TRUE(digital.writes[0].high);
  TEST_ASSERT_EQUAL_UINT8(22U, digital.writes[1].pin);
  TEST_ASSERT_TRUE(digital.writes[1].high);
}

void test_unsafe_heater_without_fan_is_sanitized_off() {
  FakeDigitalOutput digital;
  const HardwareConfig config = relayConfig();
  RelayOutputs relays(digital, config);
  OutputCommand command;
  command.fanOn = false;
  command.heaterOn = true;

  relays.apply(command);

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_EQUAL_UINT8(22U, digital.writes[0].pin);
  TEST_ASSERT_FALSE(digital.writes[0].high);
  TEST_ASSERT_EQUAL_UINT8(23U, digital.writes[1].pin);
  TEST_ASSERT_FALSE(digital.writes[1].high);
}

void test_fan_only_command_keeps_heater_off_and_fan_on() {
  FakeDigitalOutput digital;
  const HardwareConfig config = relayConfig();
  RelayOutputs relays(digital, config);
  OutputCommand command;
  command.fanOn = true;

  relays.apply(command);

  TEST_ASSERT_EQUAL_UINT8(2U, digital.writeCount);
  TEST_ASSERT_EQUAL_UINT8(22U, digital.writes[0].pin);
  TEST_ASSERT_FALSE(digital.writes[0].high);
  TEST_ASSERT_EQUAL_UINT8(23U, digital.writes[1].pin);
  TEST_ASSERT_TRUE(digital.writes[1].high);
}

void test_force_off_writes_heater_off_before_fan_off() {
  FakeDigitalOutput digital;
  const HardwareConfig config = relayConfig();
  RelayOutputs relays(digital, config);

  relays.forceOff();

  TEST_ASSERT_EQUAL_UINT8(22U, digital.writes[0].pin);
  TEST_ASSERT_FALSE(digital.writes[0].high);
  TEST_ASSERT_EQUAL_UINT8(23U, digital.writes[1].pin);
  TEST_ASSERT_FALSE(digital.writes[1].high);
}

void test_unassigned_relay_pins_are_not_written() {
  FakeDigitalOutput digital;
  HardwareConfig config = relayConfig();
  config.pins.heaterRelay = RelayOutputs::UNASSIGNED_PIN;
  config.pins.fanRelay = RelayOutputs::UNASSIGNED_PIN;
  RelayOutputs relays(digital, config);
  OutputCommand command;
  command.fanOn = true;
  command.heaterOn = true;

  relays.begin();
  relays.apply(command);

  TEST_ASSERT_EQUAL_UINT8(0U, digital.configuredCount);
  TEST_ASSERT_EQUAL_UINT8(0U, digital.writeCount);
}

void test_assigned_heater_is_not_energized_when_fan_pin_is_unassigned() {
  FakeDigitalOutput digital;
  HardwareConfig config = relayConfig();
  config.pins.fanRelay = RelayOutputs::UNASSIGNED_PIN;
  RelayOutputs relays(digital, config);
  OutputCommand command;
  command.fanOn = true;
  command.heaterOn = true;

  relays.apply(command);

  TEST_ASSERT_EQUAL_UINT8(1U, digital.writeCount);
  TEST_ASSERT_EQUAL_UINT8(22U, digital.writes[0].pin);
  TEST_ASSERT_FALSE(digital.writes[0].high);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_begin_configures_assigned_relays_with_inactive_initial_levels);
  RUN_TEST(test_active_low_begin_uses_inactive_high_initial_levels);
  RUN_TEST(test_active_low_polarity_inverts_off_levels);
  RUN_TEST(test_active_low_heater_on_writes_fan_low_before_heater_low);
  RUN_TEST(test_heater_on_command_writes_fan_on_before_heater_on);
  RUN_TEST(test_unsafe_heater_without_fan_is_sanitized_off);
  RUN_TEST(test_fan_only_command_keeps_heater_off_and_fan_on);
  RUN_TEST(test_force_off_writes_heater_off_before_fan_off);
  RUN_TEST(test_unassigned_relay_pins_are_not_written);
  RUN_TEST(test_assigned_heater_is_not_energized_when_fan_pin_is_unassigned);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
