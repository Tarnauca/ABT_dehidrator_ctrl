#pragma once

#include <stdint.h>

#include "dehydrator/domain/ProfileEngine.h"

namespace dehydrator {

/**
 * @brief Editable fields on the manual program screen.
 */
enum class ManualProgramField {
  /** Target temperature field. */
  Temperature,
  /** Program duration field. */
  Duration,
  /** Fixed/fluctuating mode field. */
  Fluctuating,
  /** Starts the configured manual program. */
  Start,
  /** Returns to the previous menu level. */
  Back,
};

/**
 * @brief Result of one manual program UI action.
 */
struct ManualProgramUiResult {
  /** True when the selected field changed. */
  bool selectionChanged = false;
  /** True when one editable value changed. */
  bool valueChanged = false;
  /** True when the configured program should be started. */
  bool startRequested = false;
  /** True when the screen should close back to menu. */
  bool exitToMenu = false;
};

/**
 * @brief Minimal manual program editor for fixed/fluctuating run setup.
 */
class ManualProgramController {
 public:
  /** Default editable target temperature in Celsius. */
  static constexpr int16_t DEFAULT_TEMP_C = 57;
  /** Default editable duration in minutes. */
  static constexpr uint16_t DEFAULT_DURATION_MINUTES = 8U * 60U;
  /** Fluctuating low-side offset from the selected average temperature. */
  static constexpr int16_t FLUCT_LOW_OFFSET_C = 5;
  /** Fluctuating high-side offset from the selected average temperature. */
  static constexpr int16_t FLUCT_HIGH_OFFSET_C = 5;
  /** Default high phase duration for fluctuating manual runs. */
  static constexpr uint16_t FLUCT_HIGH_PHASE_MINUTES = 20U;
  /** Default low phase duration for fluctuating manual runs. */
  static constexpr uint16_t FLUCT_LOW_PHASE_MINUTES = 20U;

  /**
   * @brief Returns the currently selected field.
   */
  ManualProgramField selectedField() const { return selectedField_; }

  /**
   * @brief Returns the editable target temperature in Celsius.
   */
  int16_t targetTempC() const { return targetTempC_; }

  /**
   * @brief Returns the editable duration in minutes.
   */
  uint16_t durationMinutes() const { return durationMinutes_; }

  /**
   * @brief Returns whether fluctuating mode is enabled.
   */
  bool fluctuating() const { return fluctuating_; }

  /**
   * @brief Returns whether the current field is being edited.
   */
  bool editing() const { return editing_; }

  /**
   * @brief Handles one encoder step.
   *
   * Rotation changes the selected field while on action rows, and changes the
   * current value while on editable rows.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating selection or value changes.
   */
  ManualProgramUiResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    if (!editing_) {
      return onNavigate(delta);
    }

    if (selectedField_ == ManualProgramField::Temperature) {
      const int16_t next = static_cast<int16_t>(targetTempC_ + delta);
      if (next < 0 || next > ProfileEngine::MAX_TARGET_TEMP_C) {
        return {};
      }
      targetTempC_ = next;
      ManualProgramUiResult result;
      result.valueChanged = true;
      return result;
    }

    if (selectedField_ == ManualProgramField::Duration) {
      const int32_t next =
          static_cast<int32_t>(durationMinutes_) + static_cast<int32_t>(delta) * 15;
      if (next <= 0 || next > ProfileEngine::MAX_DURATION_MINUTES) {
        return {};
      }
      durationMinutes_ = static_cast<uint16_t>(next);
      ManualProgramUiResult result;
      result.valueChanged = true;
      return result;
    }

    if (selectedField_ == ManualProgramField::Fluctuating) {
      fluctuating_ = delta > 0;
      ManualProgramUiResult result;
      result.valueChanged = true;
      return result;
    }

    return {};
  }

  /**
   * @brief Moves the active field selection.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  ManualProgramUiResult onNavigate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    const ManualProgramField previous = selectedField_;
    if (delta > 0) {
      if (selectedField_ == ManualProgramField::Temperature) {
        selectedField_ = ManualProgramField::Duration;
      } else if (selectedField_ == ManualProgramField::Duration) {
        selectedField_ = ManualProgramField::Fluctuating;
      } else if (selectedField_ == ManualProgramField::Fluctuating) {
        selectedField_ = ManualProgramField::Start;
      } else if (selectedField_ == ManualProgramField::Start) {
        selectedField_ = ManualProgramField::Back;
      }
    } else {
      if (selectedField_ == ManualProgramField::Back) {
        selectedField_ = ManualProgramField::Start;
      } else if (selectedField_ == ManualProgramField::Start) {
        selectedField_ = ManualProgramField::Fluctuating;
      } else if (selectedField_ == ManualProgramField::Fluctuating) {
        selectedField_ = ManualProgramField::Duration;
      } else if (selectedField_ == ManualProgramField::Duration) {
        selectedField_ = ManualProgramField::Temperature;
      }
    }

    if (selectedField_ == previous) {
      return {};
    }

    ManualProgramUiResult result;
    result.selectionChanged = true;
    return result;
  }

  /**
   * @brief Handles a short press on the current field.
   */
  ManualProgramUiResult onShortPress() {
    ManualProgramUiResult result;
    if (selectedField_ == ManualProgramField::Temperature ||
        selectedField_ == ManualProgramField::Duration ||
        selectedField_ == ManualProgramField::Fluctuating) {
      editing_ = !editing_;
      return result;
    }

    if (selectedField_ == ManualProgramField::Start) {
      result.startRequested = true;
      return result;
    }

    if (selectedField_ == ManualProgramField::Back) {
      selectedField_ = ManualProgramField::Temperature;
      editing_ = false;
      result.exitToMenu = true;
      return result;
    }

    return result;
  }

  /**
   * @brief Builds a `ProfileConfig` from the current editor values.
   */
  ProfileConfig profile() const {
    if (!fluctuating_) {
      return ProfileConfig{ProfileMode::Fixed, targetTempC_, 0, 0,
                           durationMinutes_, 0U, 0U};
    }

    const int16_t lowTemp =
        targetTempC_ > FLUCT_LOW_OFFSET_C ? targetTempC_ - FLUCT_LOW_OFFSET_C : 0;
    const int16_t highTemp = targetTempC_ + FLUCT_HIGH_OFFSET_C >
                                     ProfileEngine::MAX_TARGET_TEMP_C
                                 ? ProfileEngine::MAX_TARGET_TEMP_C
                                 : static_cast<int16_t>(targetTempC_ +
                                                        FLUCT_HIGH_OFFSET_C);
    return ProfileConfig{ProfileMode::Fluctuating, targetTempC_, lowTemp,
                         highTemp, durationMinutes_, FLUCT_HIGH_PHASE_MINUTES,
                         FLUCT_LOW_PHASE_MINUTES};
  }

 private:
  ManualProgramField selectedField_ = ManualProgramField::Temperature;
  int16_t targetTempC_ = DEFAULT_TEMP_C;
  uint16_t durationMinutes_ = DEFAULT_DURATION_MINUTES;
  bool fluctuating_ = false;
  bool editing_ = false;
};

}  // namespace dehydrator
