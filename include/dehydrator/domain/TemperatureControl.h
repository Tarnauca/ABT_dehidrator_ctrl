#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/domain/RunStateMachine.h"

namespace dehydrator {

/**
 * @brief Inputs needed for one temperature-control evaluation.
 */
struct TemperatureControlInput {
  /** Primary thermistor temperature in integer Celsius. */
  int16_t currentTempC = 0;
  /** Current profile target temperature in integer Celsius. */
  int16_t targetTempC = 0;
  /** Lifecycle-level output policy from the run state machine. */
  RunOutputPolicy runPolicy;
  /** Scheduler-provided elapsed time since the previous control update. */
  uint16_t deltaSeconds = 0;
};

/**
 * @brief Result of one temperature-control evaluation.
 */
struct TemperatureControlOutput {
  /** Logical heater command before hardware relay polarity is applied. */
  bool heaterOn = false;
  /** True when heater was forced off because primary thermistor temperature exceeded limit. */
  bool forcedOffByTemperature = false;
  /** True when heater was blocked because fan/control permission was missing. */
  bool blockedByRunPolicy = false;
  /** True when an OFF request is waiting for minimum ON time to expire. */
  bool waitingForMinOnTime = false;
  /** True when an ON request is waiting for minimum OFF time to expire. */
  bool waitingForMinOffTime = false;
};

/**
 * @brief Stateful hysteresis controller for relay-based heater control.
 *
 * The controller is intentionally independent from Arduino APIs. It remembers
 * the last heater command so a simple hysteresis band can avoid rapid relay
 * toggling. Hard safety limits and run-state permissions override normal
 * hysteresis behavior.
 */
class TemperatureControl {
 public:
  /**
   * @brief Creates a controller with heater OFF and switch timer reset.
   */
  TemperatureControl() = default;

  /**
   * @brief Resets the remembered heater state.
   *
   * @param heaterOn Initial logical heater command after reset.
   */
  void reset(bool heaterOn = false) {
    heaterOn_ = heaterOn;
    secondsSinceSwitch_ = 0;
  }

  /**
   * @brief Evaluates hysteresis and safety policy for one control tick.
   *
   * The heater turns ON when temperature is at or below
   * `targetTempC - hysteresisC`, and turns OFF when temperature reaches or
   * exceeds `targetTempC`. Temperatures above `heaterForceOffAboveTempC` force
   * heater OFF immediately, even if minimum relay timing would normally hold
   * the current command.
   *
   * @param config Control thresholds and optional minimum relay timings.
   * @param input Current temperature, target, run policy, and elapsed time.
   * @return Heater command plus reason flags useful for logs and tests.
   */
  TemperatureControlOutput update(const config::ControlConfig& config,
                                  const TemperatureControlInput& input) {
    advanceTimer(input.deltaSeconds);

    TemperatureControlOutput output;
    output.forcedOffByTemperature =
        input.currentTempC > config.heaterForceOffAboveTempC;

    if (!input.runPolicy.fanOn || !input.runPolicy.heaterControlAllowed) {
      forceHeaterOff();
      output.blockedByRunPolicy = true;
      return output;
    }

    if (output.forcedOffByTemperature) {
      forceHeaterOff();
      return output;
    }

    if (heaterOn_) {
      if (input.currentTempC >= input.targetTempC) {
        if (secondsSinceSwitch_ >= config.minHeaterOnSeconds) {
          switchHeater(false);
        } else {
          output.waitingForMinOnTime = true;
        }
      }
    } else if (input.currentTempC <= input.targetTempC - config.hysteresisC) {
      if (secondsSinceSwitch_ >= config.minHeaterOffSeconds) {
        switchHeater(true);
      } else {
        output.waitingForMinOffTime = true;
      }
    }

    output.heaterOn = heaterOn_;
    return output;
  }

  /**
   * @brief Returns the currently remembered logical heater command.
   *
   * @return true when the controller currently requests heater ON.
   */
  constexpr bool heaterOn() const { return heaterOn_; }

 private:
  void advanceTimer(uint16_t deltaSeconds) {
    const uint32_t next =
        static_cast<uint32_t>(secondsSinceSwitch_) + deltaSeconds;
    secondsSinceSwitch_ =
        next > UINT16_MAX ? UINT16_MAX : static_cast<uint16_t>(next);
  }

  void switchHeater(bool heaterOn) {
    if (heaterOn_ != heaterOn) {
      heaterOn_ = heaterOn;
      secondsSinceSwitch_ = 0;
    }
  }

  void forceHeaterOff() {
    if (heaterOn_) {
      switchHeater(false);
    }
  }

  bool heaterOn_ = false;
  uint16_t secondsSinceSwitch_ = 0;
};

}  // namespace dehydrator
