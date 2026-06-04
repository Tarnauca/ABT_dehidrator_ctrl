#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Supported drying profile modes.
 */
enum class ProfileMode {
  /** One target temperature for the whole run. */
  Fixed,
  /** Alternates between high and low target phases. */
  Fluctuating,
};

/**
 * @brief Configuration for one drying profile.
 *
 * Temperatures are integer Celsius values. Durations are stored in minutes to
 * match the user-facing `HH:MM` requirement and avoid floating point.
 */
struct ProfileConfig {
  /** Profile mode to evaluate. */
  ProfileMode mode = ProfileMode::Fixed;
  /** Fixed-mode target temperature and fluctuating-mode user average metadata. */
  int16_t targetTempC = 0;
  /** Fluctuating-mode low phase target temperature. */
  int16_t lowTempC = 0;
  /** Fluctuating-mode high phase target temperature. */
  int16_t highTempC = 0;
  /** Total active profile duration in minutes. */
  uint16_t durationMinutes = 0;
  /** Fluctuating-mode high phase duration in minutes. */
  uint16_t highPhaseMinutes = 0;
  /** Fluctuating-mode low phase duration in minutes. */
  uint16_t lowPhaseMinutes = 0;

  /**
   * @brief Creates a profile configuration.
   *
   * @param modeValue Profile mode to evaluate.
   * @param targetTempCValue Fixed target temperature or fluctuating average metadata.
   * @param lowTempCValue Fluctuating-mode low phase target temperature.
   * @param highTempCValue Fluctuating-mode high phase target temperature.
   * @param durationMinutesValue Total active profile duration in minutes.
   * @param highPhaseMinutesValue Fluctuating-mode high phase duration.
   * @param lowPhaseMinutesValue Fluctuating-mode low phase duration.
   */
  constexpr ProfileConfig(ProfileMode modeValue = ProfileMode::Fixed,
                          int16_t targetTempCValue = 0,
                          int16_t lowTempCValue = 0,
                          int16_t highTempCValue = 0,
                          uint16_t durationMinutesValue = 0,
                          uint16_t highPhaseMinutesValue = 0,
                          uint16_t lowPhaseMinutesValue = 0)
      : mode(modeValue),
        targetTempC(targetTempCValue),
        lowTempC(lowTempCValue),
        highTempC(highTempCValue),
        durationMinutes(durationMinutesValue),
        highPhaseMinutes(highPhaseMinutesValue),
        lowPhaseMinutes(lowPhaseMinutesValue) {}
};

/**
 * @brief Result of evaluating a profile at a specific active elapsed time.
 */
struct ProfileTarget {
  /** Target temperature for the current profile moment. */
  int16_t targetTempC = 0;
  /** True when elapsed active time is at or beyond configured duration. */
  bool complete = false;
  /** True when the profile configuration was valid enough to evaluate. */
  bool valid = false;
};

/**
 * @brief Pure profile target evaluator.
 *
 * `ProfileEngine` does not read clocks and does not track pause/resume state.
 * Callers pass active elapsed time, so paused time can be excluded by the future
 * run timer/state machine.
 */
class ProfileEngine {
 public:
  /**
   * @brief Maximum allowed profile setpoint in Celsius.
   */
  static constexpr int16_t MAX_TARGET_TEMP_C = 75;

  /**
   * @brief Maximum allowed duration in minutes (`99:00`).
   */
  static constexpr uint16_t MAX_DURATION_MINUTES = 99U * 60U;

  /**
   * @brief Evaluates a profile at a given active elapsed time.
   *
   * Fluctuating profiles start with the high-temperature phase, then alternate
   * high/low phases using the configured phase durations.
   *
   * @param config Profile configuration to evaluate.
   * @param elapsedSeconds Active elapsed profile time in seconds.
   * @return Current target, completion flag, and validity flag. The target is
   * still populated when complete is true; callers should use `complete` to
   * decide when to stop control.
   */
  static ProfileTarget evaluate(const ProfileConfig& config,
                                uint32_t elapsedSeconds) {
    ProfileTarget result;
    result.valid = isValid(config);

    if (!result.valid) {
      return result;
    }

    result.complete = elapsedSeconds >= durationSeconds(config);

    if (config.mode == ProfileMode::Fixed) {
      result.targetTempC = config.targetTempC;
      return result;
    }

    result.targetTempC = fluctuatingTarget(config, elapsedSeconds);
    return result;
  }

  /**
   * @brief Checks whether a profile is valid for evaluation.
   *
   * @param config Profile configuration to validate.
   * @return true when duration and temperature settings are usable.
   */
  static constexpr bool isValid(const ProfileConfig& config) {
    return config.durationMinutes > 0U &&
           config.durationMinutes <= MAX_DURATION_MINUTES &&
           ((config.mode == ProfileMode::Fixed &&
             isAllowedTemp(config.targetTempC)) ||
            (config.mode == ProfileMode::Fluctuating &&
             isAllowedTemp(config.targetTempC) &&
             isAllowedTemp(config.lowTempC) &&
             isAllowedTemp(config.highTempC) &&
             config.lowTempC <= config.highTempC &&
             config.highPhaseMinutes > 0U &&
             config.lowPhaseMinutes > 0U));
  }

 private:
  static constexpr bool isAllowedTemp(int16_t tempC) {
    return tempC >= 0 && tempC <= MAX_TARGET_TEMP_C;
  }

  static constexpr uint32_t durationSeconds(const ProfileConfig& config) {
    return static_cast<uint32_t>(config.durationMinutes) * 60UL;
  }

  static uint32_t phaseSeconds(uint16_t minutes) {
    return static_cast<uint32_t>(minutes) * 60UL;
  }

  static int16_t fluctuatingTarget(const ProfileConfig& config,
                                   uint32_t elapsedSeconds) {
    const uint32_t highSeconds = phaseSeconds(config.highPhaseMinutes);
    const uint32_t lowSeconds = phaseSeconds(config.lowPhaseMinutes);
    const uint32_t cycleSeconds = highSeconds + lowSeconds;
    const uint32_t cyclePosition = elapsedSeconds % cycleSeconds;

    return cyclePosition < highSeconds ? config.highTempC : config.lowTempC;
  }
};

}  // namespace dehydrator
