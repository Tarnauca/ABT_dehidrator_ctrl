#pragma once

#include <stdint.h>

#include "dehydrator/config/HardwareConfig.h"
#include "dehydrator/interfaces/DigitalOutput.h"
#include "dehydrator/interfaces/OutputController.h"

namespace dehydrator {

/**
 * @brief Hardware adapter for heater and fan relay outputs.
 *
 * `RelayOutputs` translates logical heater/fan commands into MCU pin levels
 * using configured relay polarity. It defensively sanitizes commands so heater
 * cannot remain ON when fan is OFF, even if a higher layer provides an unsafe
 * command.
 */
class RelayOutputs final : public OutputController {
 public:
  /**
   * @brief Pin value used for unassigned hardware placeholders.
   */
  static constexpr uint8_t UNASSIGNED_PIN = 255U;

  /**
   * @brief Creates a relay output adapter.
   *
   * @param digital Digital output writer.
   * @param hardware Pin and polarity configuration.
   */
  RelayOutputs(DigitalOutput& digital, const config::HardwareConfig& hardware)
      : digital_(digital), hardware_(hardware) {}

  /**
   * @brief Configures assigned relay pins as outputs and drives them OFF.
   *
   * This should be called during startup before normal control is allowed.
   */
  void begin() {
    configureIfAssigned(
        hardware_.pins.heaterRelay,
        activeLevelToPinHigh(hardware_.heaterRelayActiveLevel, false));
    configureIfAssigned(
        hardware_.pins.fanRelay,
        activeLevelToPinHigh(hardware_.fanRelayActiveLevel, false));
  }

  /**
   * @brief Applies a logical heater/fan command using safe write ordering.
   *
   * Heater OFF is written before fan OFF. Fan ON is written before heater ON.
   * This prevents transient writes from energizing the heater without fan.
   *
   * @param command Desired logical command before defensive sanitization.
   */
  void apply(OutputCommand command) override {
    const OutputCommand sanitized = sanitizeOutputCommand(command);
    const bool fanRelayAvailable = isAssigned(hardware_.pins.fanRelay);

    if (sanitized.heaterOn && fanRelayAvailable) {
      writeFan(true);
      writeHeater(true);
      return;
    }

    writeHeater(false);
    writeFan(sanitized.fanOn);
  }

  /**
   * @brief Immediately commands heater and fan relays OFF.
   */
  void forceOff() {
    writeHeater(false);
    writeFan(false);
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

  void writeHeater(bool on) {
    writeIfAssigned(hardware_.pins.heaterRelay,
                    activeLevelToPinHigh(hardware_.heaterRelayActiveLevel, on));
  }

  void writeFan(bool on) {
    writeIfAssigned(hardware_.pins.fanRelay,
                    activeLevelToPinHigh(hardware_.fanRelayActiveLevel, on));
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
