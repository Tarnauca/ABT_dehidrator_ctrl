#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"

namespace dehydrator {

/**
 * @brief Selectable fields on the NTC calibration screen.
 */
enum class NtcCalibrationField {
  /** Editable NTC offset in 0.1 C steps. */
  Offset,
  /** Editable NTC scale in 0.01 steps. */
  Scale,
  /** Persists the current calibration values. */
  Save,
  /** Restores editor values to firmware defaults. */
  Restore,
  /** Returns to the previous menu level. */
  Back,
};

/**
 * @brief Result of one NTC calibration UI action.
 */
struct NtcCalibrationResult {
  /** True when the selected field changed. */
  bool selectionChanged = false;
  /** True when one calibration value changed. */
  bool valueChanged = false;
  /** True when the current calibration should be persisted. */
  bool saveRequested = false;
  /** True when the screen should close back to settings. */
  bool exitToSettings = false;
};

/**
 * @brief Small editor for persisted user NTC offset/scale calibration.
 */
class NtcCalibrationController {
 public:
  /** Offset step in centi-Celsius, equivalent to 0.1 C. */
  static constexpr int16_t OFFSET_STEP_CENTI_C = 10;
  /** Minimum editable offset in centi-Celsius. */
  static constexpr int16_t MIN_OFFSET_CENTI_C = -2000;
  /** Maximum editable offset in centi-Celsius. */
  static constexpr int16_t MAX_OFFSET_CENTI_C = 2000;
  /** Scale step in ppm, equivalent to 0.01. */
  static constexpr int32_t SCALE_STEP_PPM = 10000;
  /** Minimum editable scale in ppm, equivalent to 0.80. */
  static constexpr int32_t MIN_SCALE_PPM = 800000;
  /** Maximum editable scale in ppm, equivalent to 1.20. */
  static constexpr int32_t MAX_SCALE_PPM = 1200000;
  /** Number of selectable fields on the screen. */
  static constexpr uint8_t FIELD_COUNT = 5U;

  /**
   * @brief Loads the editor state from one active calibration config.
   *
   * @param calibration Current active calibration values.
   */
  void loadFromCalibration(const config::CalibrationConfig& calibration) {
    offsetCentiC_ = calibration.ntcOffsetCentiC;
    scalePpm_ = calibration.ntcScalePpm;
    savedOffsetCentiC_ = offsetCentiC_;
    savedScalePpm_ = scalePpm_;
    selectedIndex_ = 0U;
    selectedField_ = NtcCalibrationField::Offset;
    editing_ = false;
    dirty_ = false;
  }

  /**
   * @brief Returns the selected field.
   */
  NtcCalibrationField selectedField() const { return selectedField_; }

  /**
   * @brief Returns the selected row index.
   */
  uint8_t selectedIndex() const { return selectedIndex_; }

  /**
   * @brief Returns whether the selected field is being edited.
   */
  bool editing() const { return editing_; }

  /**
   * @brief Returns whether the editor differs from the last loaded/saved state.
   */
  bool dirty() const { return dirty_; }

  /**
   * @brief Returns the editable NTC offset in centi-Celsius.
   */
  int16_t offsetCentiC() const { return offsetCentiC_; }

  /**
   * @brief Returns the editable NTC scale in ppm.
   */
  int32_t scalePpm() const { return scalePpm_; }

  /**
   * @brief Marks the current values as the newly persisted baseline.
   */
  void markSaved() {
    savedOffsetCentiC_ = offsetCentiC_;
    savedScalePpm_ = scalePpm_;
    dirty_ = false;
  }

  /**
   * @brief Handles one encoder step.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result describing selection or value changes.
   */
  NtcCalibrationResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    if (!editing_) {
      return onNavigate(delta);
    }

    return editSelectedField(delta);
  }

  /**
   * @brief Handles a short press on the current field.
   *
   * @return Result describing the requested action.
   */
  NtcCalibrationResult onShortPress() {
    const NtcCalibrationField field = selectedField_;
    if (field == NtcCalibrationField::Offset ||
        field == NtcCalibrationField::Scale) {
      editing_ = !editing_;
      return {};
    }

    if (field == NtcCalibrationField::Save) {
      NtcCalibrationResult result;
      result.saveRequested = true;
      return result;
    }

    if (field == NtcCalibrationField::Restore) {
      offsetCentiC_ = config::CALIBRATION.ntcOffsetCentiC;
      scalePpm_ = config::CALIBRATION.ntcScalePpm;
      dirty_ = offsetCentiC_ != savedOffsetCentiC_ ||
               scalePpm_ != savedScalePpm_;
      NtcCalibrationResult result;
      result.valueChanged = true;
      return result;
    }

    selectedIndex_ = 0U;
    selectedField_ = NtcCalibrationField::Offset;
    editing_ = false;
    offsetCentiC_ = savedOffsetCentiC_;
    scalePpm_ = savedScalePpm_;
    dirty_ = false;
    NtcCalibrationResult result;
    result.exitToSettings = true;
    return result;
  }

 private:
  static NtcCalibrationField fieldAt(uint8_t index) {
    static constexpr NtcCalibrationField kFields[FIELD_COUNT] = {
        NtcCalibrationField::Offset, NtcCalibrationField::Scale,
        NtcCalibrationField::Save,   NtcCalibrationField::Restore,
        NtcCalibrationField::Back};
    return index < FIELD_COUNT ? kFields[index] : NtcCalibrationField::Back;
  }

  NtcCalibrationResult onNavigate(int8_t delta) {
    const uint8_t previous = selectedIndex_;
    if (delta > 0 && selectedIndex_ + 1U < FIELD_COUNT) {
      selectedIndex_++;
    } else if (delta < 0 && selectedIndex_ > 0U) {
      selectedIndex_--;
    }

    selectedField_ = fieldAt(selectedIndex_);
    if (selectedIndex_ == previous) {
      return {};
    }

    NtcCalibrationResult result;
    result.selectionChanged = true;
    return result;
  }

  NtcCalibrationResult editSelectedField(int8_t delta) {
    if (selectedField_ == NtcCalibrationField::Offset) {
      const int32_t next = static_cast<int32_t>(offsetCentiC_) +
                           static_cast<int32_t>(delta) * OFFSET_STEP_CENTI_C;
      if (next < MIN_OFFSET_CENTI_C || next > MAX_OFFSET_CENTI_C) {
        return {};
      }
      offsetCentiC_ = static_cast<int16_t>(next);
    } else if (selectedField_ == NtcCalibrationField::Scale) {
      const int32_t next =
          scalePpm_ + static_cast<int32_t>(delta) * SCALE_STEP_PPM;
      if (next < MIN_SCALE_PPM || next > MAX_SCALE_PPM) {
        return {};
      }
      scalePpm_ = next;
    } else {
      return {};
    }

    dirty_ = offsetCentiC_ != savedOffsetCentiC_ || scalePpm_ != savedScalePpm_;
    NtcCalibrationResult result;
    result.valueChanged = true;
    return result;
  }

  uint8_t selectedIndex_ = 0U;
  NtcCalibrationField selectedField_ = NtcCalibrationField::Offset;
  bool editing_ = false;
  bool dirty_ = false;
  int16_t offsetCentiC_ = config::CALIBRATION.ntcOffsetCentiC;
  int32_t scalePpm_ = config::CALIBRATION.ntcScalePpm;
  int16_t savedOffsetCentiC_ = config::CALIBRATION.ntcOffsetCentiC;
  int32_t savedScalePpm_ = config::CALIBRATION.ntcScalePpm;
};

}  // namespace dehydrator
