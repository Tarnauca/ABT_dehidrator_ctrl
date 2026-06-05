#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Persistable/editable draft for one user-defined manual program.
 */
enum class ManualProgramMode {
  /** Constant target temperature for the full duration. */
  Constant,
  /** Initial higher-temperature phase followed by constant target. */
  Boost,
  /** Alternating upper/lower target temperatures. */
  Fluctuating,
};

/**
 * @brief Full editable state for one manual drying program.
 */
struct ManualProgramDraft {
  /** Selected manual drying mode. */
  ManualProgramMode mode = ManualProgramMode::Constant;
  /** Base or reference temperature in Celsius. */
  int16_t targetTempC = 0;
  /** Total run duration in minutes. */
  uint16_t durationMinutes = 0U;
  /** Boost delta above the base temperature in Celsius. */
  int16_t boostDeltaC = 0;
  /** Boost phase duration in minutes. */
  uint16_t boostDurationMinutes = 0U;
  /** Fluctuating upper target temperature in Celsius. */
  int16_t upperTempC = 0;
  /** Fluctuating lower target temperature in Celsius. */
  int16_t lowerTempC = 0;
  /** Fluctuating upper phase duration in minutes. */
  uint16_t upperDurationMinutes = 0U;
  /** Fluctuating lower phase duration in minutes. */
  uint16_t lowerDurationMinutes = 0U;
};

/**
 * @brief Returns whether two drafts contain identical editable values.
 */
constexpr bool operator==(const ManualProgramDraft& left,
                          const ManualProgramDraft& right) {
  return left.mode == right.mode &&
         left.targetTempC == right.targetTempC &&
         left.durationMinutes == right.durationMinutes &&
         left.boostDeltaC == right.boostDeltaC &&
         left.boostDurationMinutes == right.boostDurationMinutes &&
         left.upperTempC == right.upperTempC &&
         left.lowerTempC == right.lowerTempC &&
         left.upperDurationMinutes == right.upperDurationMinutes &&
         left.lowerDurationMinutes == right.lowerDurationMinutes;
}

/**
 * @brief Returns whether two drafts differ in any editable value.
 */
constexpr bool operator!=(const ManualProgramDraft& left,
                          const ManualProgramDraft& right) {
  return !(left == right);
}

}  // namespace dehydrator
