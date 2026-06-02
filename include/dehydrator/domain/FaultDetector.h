#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"

namespace dehydrator {

/**
 * @brief Stable hard-fault reason codes.
 */
enum class FaultCode {
  /** No hard fault is active. */
  None,
  /** PT50 is missing, invalid, or outside configured plausible range. */
  Pt50Invalid,
  /** PT50 temperature reached the hard over-temperature threshold. */
  OverTemperature,
  /** Temperature did not rise enough while heater was commanded ON. */
  TemperatureNotRising,
  /** Temperature rose suspiciously while heater command was OFF. */
  HeaterStuckOnSuspected,
  /** Encoder/button input was active continuously for too long. */
  ButtonStuck,
  /** Watchdog reset occurred during an active run. */
  WatchdogResetDuringRun,
};

/**
 * @brief Inputs needed for one fault-detection update.
 */
struct FaultDetectorInput {
  /** True when the primary PT50 measurement is valid and present. */
  bool pt50Valid = false;
  /** Primary PT50 temperature in integer Celsius. */
  int16_t pt50TempC = 0;
  /** Logical heater command after control policy but before relay polarity. */
  bool heaterCommandOn = false;
  /** True while the encoder pushbutton is debounced active. */
  bool buttonActive = false;
  /** True when boot/reset analysis detected watchdog reset during active run. */
  bool watchdogResetDuringRun = false;
  /** Scheduler-provided elapsed time since the previous detector update. */
  uint16_t deltaSeconds = 0;
};

/**
 * @brief Result of one fault-detection update.
 */
struct FaultDetectorResult {
  /** True when a hard fault is latched. */
  bool hardFault = false;
  /** Latched hard-fault reason code. */
  FaultCode code = FaultCode::None;
};

/**
 * @brief Pure hard-fault detector for safety-relevant runtime conditions.
 *
 * The detector latches the first hard fault until `reset()` is called. This
 * mirrors the required user acknowledgement flow at the application level and
 * prevents later secondary symptoms from hiding the original reason.
 */
class FaultDetector {
 public:
  /**
   * @brief Creates a detector with no active fault and cleared timing state.
   */
  FaultDetector() = default;

  /**
   * @brief Clears latched fault and timing state.
   */
  void reset() {
    latchedCode_ = FaultCode::None;
    noRiseTracking_ = false;
    noRiseAccumulatedSeconds_ = 0;
    noRiseBaselineTempC_ = 0;
    heaterWasCommandedOn_ = false;
    heaterOffSeconds_ = 0;
    stuckMonitoring_ = false;
    stuckMonitorSeconds_ = 0;
    stuckBaselineTempC_ = 0;
    buttonActiveSeconds_ = 0;
  }

  /**
   * @brief Updates fault detection using one scheduler tick of input data.
   *
   * PT50 validity and hard over-temperature are checked immediately.
   * Temperature-not-rising accumulates heater ON command time. Suspected
   * heater-stuck-ON monitoring starts only after the configured heater-OFF
   * grace time.
   *
   * @param config Safety thresholds and timing windows.
   * @param input Current sensor/control/input state.
   * @return Current latched fault status.
   */
  FaultDetectorResult update(const config::SafetyConfig& config,
                             const FaultDetectorInput& input) {
    if (latchedCode_ != FaultCode::None) {
      return result();
    }

    if (input.watchdogResetDuringRun) {
      latch(FaultCode::WatchdogResetDuringRun);
      return result();
    }

    if (!isPt50Usable(config, input)) {
      latch(FaultCode::Pt50Invalid);
      return result();
    }

    if (input.pt50TempC >= config.hardFaultTempC) {
      latch(FaultCode::OverTemperature);
      return result();
    }

    updateButton(config, input);
    if (latchedCode_ != FaultCode::None) {
      return result();
    }

    updateNoRise(config, input);
    if (latchedCode_ != FaultCode::None) {
      return result();
    }

    updateStuckHeater(config, input);
    return result();
  }

 private:
  static bool isPt50Usable(const config::SafetyConfig& config,
                           const FaultDetectorInput& input) {
    return input.pt50Valid && input.pt50TempC >= config.pt50MinValidTempC &&
           input.pt50TempC <= config.pt50MaxValidTempC;
  }

  void updateButton(const config::SafetyConfig& config,
                    const FaultDetectorInput& input) {
    if (!input.buttonActive) {
      buttonActiveSeconds_ = 0;
      return;
    }

    buttonActiveSeconds_ = saturatingAdd(buttonActiveSeconds_, input.deltaSeconds);
    if (buttonActiveSeconds_ >= config.buttonStuckSeconds) {
      latch(FaultCode::ButtonStuck);
    }
  }

  void updateNoRise(const config::SafetyConfig& config,
                    const FaultDetectorInput& input) {
    if (!input.heaterCommandOn) {
      return;
    }

    if (!noRiseTracking_) {
      noRiseTracking_ = true;
      noRiseBaselineTempC_ = input.pt50TempC;
      noRiseAccumulatedSeconds_ = 0;
    }

    noRiseAccumulatedSeconds_ =
        saturatingAdd(noRiseAccumulatedSeconds_, input.deltaSeconds);

    if (input.pt50TempC >= noRiseBaselineTempC_ + config.noRiseMinIncreaseC) {
      noRiseBaselineTempC_ = input.pt50TempC;
      noRiseAccumulatedSeconds_ = 0;
      return;
    }

    if (noRiseAccumulatedSeconds_ >= config.noRiseWindowSeconds) {
      latch(FaultCode::TemperatureNotRising);
    }
  }

  void updateStuckHeater(const config::SafetyConfig& config,
                         const FaultDetectorInput& input) {
    if (input.heaterCommandOn) {
      heaterWasCommandedOn_ = true;
      heaterOffSeconds_ = 0;
      stuckMonitoring_ = false;
      stuckMonitorSeconds_ = 0;
      return;
    }

    if (!heaterWasCommandedOn_) {
      return;
    }

    heaterOffSeconds_ = saturatingAdd(heaterOffSeconds_, input.deltaSeconds);
    if (heaterOffSeconds_ < config.stuckHeaterGraceSeconds) {
      return;
    }

    if (!stuckMonitoring_) {
      stuckMonitoring_ = true;
      stuckMonitorSeconds_ = 0;
      stuckBaselineTempC_ = input.pt50TempC;
      return;
    }

    stuckMonitorSeconds_ =
        saturatingAdd(stuckMonitorSeconds_, input.deltaSeconds);

    if (input.pt50TempC >= stuckBaselineTempC_ + config.stuckHeaterRiseC &&
        stuckMonitorSeconds_ <= config.stuckHeaterWindowSeconds) {
      latch(FaultCode::HeaterStuckOnSuspected);
      return;
    }

    if (stuckMonitorSeconds_ >= config.stuckHeaterWindowSeconds) {
      stuckBaselineTempC_ = input.pt50TempC;
      stuckMonitorSeconds_ = 0;
    }
  }

  static uint16_t saturatingAdd(uint16_t value, uint16_t increment) {
    const uint32_t next = static_cast<uint32_t>(value) + increment;
    return next > UINT16_MAX ? UINT16_MAX : static_cast<uint16_t>(next);
  }

  void latch(FaultCode code) { latchedCode_ = code; }

  constexpr FaultDetectorResult result() const {
    FaultDetectorResult detectorResult;
    detectorResult.code = latchedCode_;
    detectorResult.hardFault = latchedCode_ != FaultCode::None;
    return detectorResult;
  }

  FaultCode latchedCode_ = FaultCode::None;
  bool noRiseTracking_ = false;
  uint16_t noRiseAccumulatedSeconds_ = 0;
  int16_t noRiseBaselineTempC_ = 0;
  bool heaterWasCommandedOn_ = false;
  uint16_t heaterOffSeconds_ = 0;
  bool stuckMonitoring_ = false;
  uint16_t stuckMonitorSeconds_ = 0;
  int16_t stuckBaselineTempC_ = 0;
  uint16_t buttonActiveSeconds_ = 0;
};

}  // namespace dehydrator
