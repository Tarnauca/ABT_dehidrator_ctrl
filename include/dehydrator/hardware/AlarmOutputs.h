#pragma once

#include <stdint.h>

#include "dehydrator/config/HardwareConfig.h"
#include "dehydrator/interfaces/DigitalOutput.h"
#include "dehydrator/interfaces/OutputController.h"

namespace dehydrator {

/**
 * @brief Hardware adapter for buzzer and LCD backlight alarm outputs.
 *
 * `AlarmOutputs` translates logical buzzer/backlight commands into MCU pin
 * levels using configured active polarity. It intentionally ignores
 * heater/fan fields from `OutputCommand`; relay outputs are owned by
 * `RelayOutputs`.
 */
class AlarmOutputs final : public OutputController {
 public:
  /**
   * @brief Pin value used for unassigned hardware placeholders.
   */
  static constexpr uint8_t UNASSIGNED_PIN = 255U;

  /**
   * @brief Creates an alarm output adapter.
   *
   * @param digital Digital output writer.
   * @param hardware Pin and polarity configuration.
   */
  AlarmOutputs(DigitalOutput& digital, const config::HardwareConfig& hardware)
      : digital_(digital), hardware_(hardware) {}

  /**
   * @brief Configures assigned alarm pins as outputs and drives them OFF.
   *
   * Inactive pin levels are supplied during output configuration to avoid
   * startup pulses on active-low hardware.
   */
  void begin() {
    configureIfAssigned(
        hardware_.pins.lcdBacklight,
        activeLevelToPinHigh(hardware_.lcdBacklightActiveLevel, false));
    configureIfAssigned(hardware_.pins.buzzer,
                        activeLevelToPinHigh(hardware_.buzzerActiveLevel, false));
  }

  /**
   * @brief Applies logical buzzer and backlight commands.
   *
   * @param command Desired logical output command. Heater/fan fields are ignored
   * by this adapter.
   */
  void apply(OutputCommand command) override {
    writeBacklight(command.backlightOn);
    writeBuzzer(command.buzzerOn);
  }

  /**
   * @brief Immediately commands buzzer and backlight OFF.
   */
  void forceOff() {
    writeBacklight(false);
    writeBuzzer(false);
  }

 private:
  static constexpr bool activeLevelToPinHigh(config::ActiveLevel activeLevel,
                                             bool active) {
    if (activeLevel == config::ActiveLevel::ActiveHigh) {
      return active;
    }

    return !active;
  }

  static constexpr bool isAssigned(uint8_t pin) {
    return pin != UNASSIGNED_PIN;
  }

  void configureIfAssigned(uint8_t pin, bool initialHigh) {
    if (isAssigned(pin)) {
      digital_.configureOutput(pin, initialHigh);
    }
  }

  void writeBacklight(bool on) {
    writeIfAssigned(
        hardware_.pins.lcdBacklight,
        activeLevelToPinHigh(hardware_.lcdBacklightActiveLevel, on));
  }

  void writeBuzzer(bool on) {
    writeIfAssigned(hardware_.pins.buzzer,
                    activeLevelToPinHigh(hardware_.buzzerActiveLevel, on));
  }

  void writeIfAssigned(uint8_t pin, bool high) {
    if (isAssigned(pin)) {
      digital_.write(pin, high);
    }
  }

  DigitalOutput& digital_;
  const config::HardwareConfig& hardware_;
};

}  // namespace dehydrator
