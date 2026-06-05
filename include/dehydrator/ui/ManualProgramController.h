#pragma once

#include <stdint.h>

#include "dehydrator/domain/ManualProgramDraft.h"
#include "dehydrator/domain/ProfileEngine.h"

namespace dehydrator {

/**
 * @brief Editable fields on the manual program screen.
 */
enum class ManualProgramField {
  /** Manual mode selector. */
  Mode,
  /** Base/reference temperature field. */
  Temperature,
  /** Program duration field. */
  Duration,
  /** Boost temperature delta field. */
  BoostDelta,
  /** Boost phase duration field. */
  BoostDuration,
  /** Fluctuating upper temperature field. */
  UpperTemp,
  /** Fluctuating lower temperature field. */
  LowerTemp,
  /** Fluctuating upper phase duration field. */
  UpperDuration,
  /** Fluctuating lower phase duration field. */
  LowerDuration,
  /** Starts the configured manual program. */
  Start,
  /** Saves the configured profile into one user slot. */
  Save,
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
  /** True when the current profile should be saved by the UI shell. */
  bool saveRequested = false;
  /** True when the screen should close back to menu. */
  bool exitToMenu = false;
};

/**
 * @brief Manual program editor for constant, boost, and fluctuating runs.
 */
class ManualProgramController {
 public:
  /** Default editable target/reference temperature in Celsius. */
  static constexpr int16_t DEFAULT_TEMP_C = 57;
  /** Default editable duration in minutes. */
  static constexpr uint16_t DEFAULT_DURATION_MINUTES = 8U * 60U;
  /** Duration edit step in minutes. */
  static constexpr uint16_t DURATION_STEP_MINUTES = 15U;
  /** Boost delta edit step in Celsius. */
  static constexpr int16_t BOOST_DELTA_STEP_C = 5;
  /** Maximum boost delta above the base temperature. */
  static constexpr int16_t MAX_BOOST_DELTA_C = 20;
  /** Boost duration edit step in minutes. */
  static constexpr uint16_t BOOST_DURATION_STEP_MINUTES = 5U;
  /** Default boost delta in Celsius. */
  static constexpr int16_t DEFAULT_BOOST_DELTA_C = 10;
  /** Default boost duration in minutes. */
  static constexpr uint16_t DEFAULT_BOOST_DURATION_MINUTES = 30U;
  /** Maximum absolute deviation from reference in fluctuating mode. */
  static constexpr int16_t MAX_FLUCT_OFFSET_C = 10;
  /** Fluctuating phase duration edit step in minutes. */
  static constexpr uint16_t FLUCT_DURATION_STEP_MINUTES = 1U;
  /** Minimum fluctuating phase duration in minutes. */
  static constexpr uint16_t MIN_FLUCT_DURATION_MINUTES = 5U;
  /** Maximum fluctuating phase duration in minutes. */
  static constexpr uint16_t MAX_FLUCT_DURATION_MINUTES = 20U;
  /** Default upper target offset from reference. */
  static constexpr int16_t DEFAULT_UPPER_OFFSET_C = 5;
  /** Default lower target offset from reference. */
  static constexpr int16_t DEFAULT_LOWER_OFFSET_C = 5;
  /** Default upper/lower phase duration in minutes. */
  static constexpr uint16_t DEFAULT_FLUCT_PHASE_MINUTES = 10U;

  /**
   * @brief Creates the controller with default manual-program values.
   */
  ManualProgramController() { resetToDefaults(); }

  /**
   * @brief Returns the currently selected field.
   */
  ManualProgramField selectedField() const { return selectedField_; }

  /**
   * @brief Returns the current zero-based selection index.
   */
  uint8_t selectedIndex() const { return selectedIndex_; }

  /**
   * @brief Returns the selected manual program mode.
   */
  ManualProgramMode mode() const { return mode_; }

  /**
   * @brief Returns the editable base/reference temperature in Celsius.
   */
  int16_t targetTempC() const { return targetTempC_; }

  /**
   * @brief Returns the editable duration in minutes.
   */
  uint16_t durationMinutes() const { return durationMinutes_; }

  /**
   * @brief Returns the boost delta above the base temperature.
   */
  int16_t boostDeltaC() const { return boostDeltaC_; }

  /**
   * @brief Returns the boost phase duration in minutes.
   */
  uint16_t boostDurationMinutes() const { return boostDurationMinutes_; }

  /**
   * @brief Returns the fluctuating upper target temperature.
   */
  int16_t upperTempC() const { return upperTempC_; }

  /**
   * @brief Returns the fluctuating lower target temperature.
   */
  int16_t lowerTempC() const { return lowerTempC_; }

  /**
   * @brief Returns the upper phase duration in minutes.
   */
  uint16_t upperDurationMinutes() const { return upperDurationMinutes_; }

  /**
   * @brief Returns the lower phase duration in minutes.
   */
  uint16_t lowerDurationMinutes() const { return lowerDurationMinutes_; }

  /**
   * @brief Returns whether the current field is being edited.
   */
  bool editing() const { return editing_; }

  /**
   * @brief Returns whether the current editor differs from its last saved state.
   */
  bool dirty() const { return dirty_; }

  /**
   * @brief Returns whether the editor is associated with one saved profile slot.
   */
  bool hasAssociatedSlot() const { return associatedSlotValid_; }

  /**
   * @brief Returns the associated saved slot index when available.
   */
  uint8_t associatedSlot() const { return associatedSlot_; }

  /**
   * @brief Returns the number of selectable fields for the current mode.
   */
  uint8_t fieldCount() const { return fieldCountForMode(mode_); }

  /**
   * @brief Returns the field at a visible/selection index for the current mode.
   *
   * @param index Zero-based selection index.
   * @return Field for that index, or `Back` if the index is outside range.
   */
  ManualProgramField fieldAt(uint8_t index) const {
    return fieldAtIndex(mode_, index);
  }

  /**
   * @brief Returns the number of selectable fields for one mode.
   *
   * @param mode Manual mode whose dynamic field list is requested.
   * @return Number of fields including `Start` and `Back`.
   */
  static uint8_t fieldCountForMode(ManualProgramMode mode) {
    if (mode == ManualProgramMode::Boost) {
      return BOOST_FIELD_COUNT;
    }
    if (mode == ManualProgramMode::Fluctuating) {
      return FLUCT_FIELD_COUNT;
    }
    return CONSTANT_FIELD_COUNT;
  }

  /**
   * @brief Returns one field from a mode-specific dynamic field list.
   *
   * @param mode Manual mode whose field list is requested.
   * @param index Zero-based field index.
   * @return Field for that position, or `Back` if index is outside range.
   */
  static ManualProgramField fieldAtIndex(ManualProgramMode mode,
                                         uint8_t index) {
    if (mode == ManualProgramMode::Boost) {
      static constexpr ManualProgramField fields[BOOST_FIELD_COUNT] = {
          ManualProgramField::Mode, ManualProgramField::Temperature,
          ManualProgramField::Duration, ManualProgramField::BoostDelta,
          ManualProgramField::BoostDuration, ManualProgramField::Start,
          ManualProgramField::Save, ManualProgramField::Back};
      return index < BOOST_FIELD_COUNT ? fields[index] : ManualProgramField::Back;
    }

    if (mode == ManualProgramMode::Fluctuating) {
      static constexpr ManualProgramField fields[FLUCT_FIELD_COUNT] = {
          ManualProgramField::Mode, ManualProgramField::Temperature,
          ManualProgramField::Duration, ManualProgramField::UpperTemp,
          ManualProgramField::LowerTemp, ManualProgramField::UpperDuration,
          ManualProgramField::LowerDuration, ManualProgramField::Start,
          ManualProgramField::Save, ManualProgramField::Back};
      return index < FLUCT_FIELD_COUNT ? fields[index] : ManualProgramField::Back;
    }

    static constexpr ManualProgramField fields[CONSTANT_FIELD_COUNT] = {
        ManualProgramField::Mode, ManualProgramField::Temperature,
        ManualProgramField::Duration, ManualProgramField::Start,
        ManualProgramField::Save, ManualProgramField::Back};
    return index < CONSTANT_FIELD_COUNT ? fields[index] : ManualProgramField::Back;
  }

  /**
   * @brief Handles one encoder step.
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

    return editSelectedField(delta);
  }

  /**
   * @brief Moves the active field selection within the dynamic mode list.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  ManualProgramUiResult onNavigate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    const uint8_t previous = selectedIndex_;
    const uint8_t count = fieldCount();
    if (delta > 0 && selectedIndex_ + 1U < count) {
      selectedIndex_++;
    } else if (delta < 0 && selectedIndex_ > 0U) {
      selectedIndex_--;
    }

    selectedField_ = fieldAt(selectedIndex_);
    if (selectedIndex_ == previous) {
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
    if (isEditable(selectedField_)) {
      editing_ = !editing_;
      return result;
    }

    if (selectedField_ == ManualProgramField::Start) {
      result.startRequested = true;
      return result;
    }

    if (selectedField_ == ManualProgramField::Save) {
      result.saveRequested = true;
      return result;
    }

    if (selectedField_ == ManualProgramField::Back) {
      resetSelection();
      result.exitToMenu = true;
      return result;
    }

    return result;
  }

  /**
   * @brief Builds a `ProfileConfig` from the current editor values.
   */
  ProfileConfig profile() const {
    if (mode_ == ManualProgramMode::Constant) {
      return ProfileConfig{ProfileMode::Fixed, targetTempC_, 0, 0,
                           durationMinutes_, 0U, 0U};
    }

    if (mode_ == ManualProgramMode::Boost) {
      return ProfileConfig{ProfileMode::Boost, targetTempC_, 0,
                           boostTargetTempC(), durationMinutes_,
                           boostDurationMinutes_, 0U};
    }

    return ProfileConfig{ProfileMode::Fluctuating, targetTempC_, lowerTempC_,
                         upperTempC_, durationMinutes_, upperDurationMinutes_,
                         lowerDurationMinutes_};
  }

  /**
   * @brief Loads one profile into the editor and clears the dirty flag.
   *
   * @param profile Profile configuration to edit.
   * @param associatedSlotValid True when the profile came from one saved slot.
   * @param associatedSlot Slot index when `associatedSlotValid` is true.
   */
  void loadProfile(const ProfileConfig& profile,
                   bool associatedSlotValid = false,
                   uint8_t associatedSlot = 0U) {
    const ProfileConfig source = profile;
    resetToDefaults();

    if (source.mode == ProfileMode::Boost) {
      mode_ = ManualProgramMode::Boost;
      targetTempC_ = source.targetTempC;
      durationMinutes_ = source.durationMinutes;
      boostDeltaC_ =
          static_cast<int16_t>(source.highTempC - source.targetTempC);
      boostDurationMinutes_ = source.highPhaseMinutes;
    } else if (source.mode == ProfileMode::Fluctuating) {
      mode_ = ManualProgramMode::Fluctuating;
      targetTempC_ = source.targetTempC;
      durationMinutes_ = source.durationMinutes;
      upperTempC_ = source.highTempC;
      lowerTempC_ = source.lowTempC;
      upperDurationMinutes_ = source.highPhaseMinutes;
      lowerDurationMinutes_ = source.lowPhaseMinutes;
    } else {
      mode_ = ManualProgramMode::Constant;
      targetTempC_ = source.targetTempC;
      durationMinutes_ = source.durationMinutes;
    }

    associatedSlotValid_ = associatedSlotValid;
    associatedSlot_ = associatedSlot;
    dirty_ = false;
    baselineProfile_ = this->profile();
    baselineAssociatedSlotValid_ = associatedSlotValid;
    baselineAssociatedSlot_ = associatedSlot;
    resetSelection();
  }

  /**
   * @brief Marks the current editor values as saved in the given slot.
   *
   * @param slot Saved user-profile slot index.
   */
  void markSaved(uint8_t slot) {
    associatedSlotValid_ = true;
    associatedSlot_ = slot;
    dirty_ = false;
    baselineProfile_ = this->profile();
    baselineAssociatedSlotValid_ = true;
    baselineAssociatedSlot_ = slot;
  }

  /**
   * @brief Restores default manual-program values for a new unsaved profile.
   */
  void resetToDefaults() {
    mode_ = ManualProgramMode::Constant;
    targetTempC_ = DEFAULT_TEMP_C;
    durationMinutes_ = DEFAULT_DURATION_MINUTES;
    boostDeltaC_ = DEFAULT_BOOST_DELTA_C;
    boostDurationMinutes_ = DEFAULT_BOOST_DURATION_MINUTES;
    upperTempC_ = DEFAULT_TEMP_C + DEFAULT_UPPER_OFFSET_C;
    lowerTempC_ = DEFAULT_TEMP_C - DEFAULT_LOWER_OFFSET_C;
    upperDurationMinutes_ = DEFAULT_FLUCT_PHASE_MINUTES;
    lowerDurationMinutes_ = DEFAULT_FLUCT_PHASE_MINUTES;
    associatedSlotValid_ = false;
    associatedSlot_ = 0U;
    dirty_ = false;
    baselineProfile_ = this->profile();
    baselineAssociatedSlotValid_ = false;
    baselineAssociatedSlot_ = 0U;
    resetSelection();
  }

  /**
   * @brief Discards unsaved changes and restores the last baseline state.
   */
  void discardChanges() {
    loadProfile(baselineProfile_, baselineAssociatedSlotValid_,
                baselineAssociatedSlot_);
  }

 private:
  static constexpr uint8_t CONSTANT_FIELD_COUNT = 6U;
  static constexpr uint8_t BOOST_FIELD_COUNT = 8U;
  static constexpr uint8_t FLUCT_FIELD_COUNT = 10U;

  static bool isEditable(ManualProgramField field) {
    return field != ManualProgramField::Start &&
           field != ManualProgramField::Save &&
           field != ManualProgramField::Back;
  }

  ManualProgramUiResult editSelectedField(int8_t delta) {
    if (selectedField_ == ManualProgramField::Mode) {
      return editMode(delta);
    }
    if (selectedField_ == ManualProgramField::Temperature) {
      return editTemperature(delta);
    }
    if (selectedField_ == ManualProgramField::Duration) {
      return editDuration(delta);
    }
    if (selectedField_ == ManualProgramField::BoostDelta) {
      return editBoostDelta(delta);
    }
    if (selectedField_ == ManualProgramField::BoostDuration) {
      return editBoostDuration(delta);
    }
    if (selectedField_ == ManualProgramField::UpperTemp) {
      return editUpperTemp(delta);
    }
    if (selectedField_ == ManualProgramField::LowerTemp) {
      return editLowerTemp(delta);
    }
    if (selectedField_ == ManualProgramField::UpperDuration) {
      return editUpperDuration(delta);
    }
    if (selectedField_ == ManualProgramField::LowerDuration) {
      return editLowerDuration(delta);
    }
    return {};
  }

  ManualProgramUiResult editMode(int8_t delta) {
    const ManualProgramMode previous = mode_;
    if (delta > 0) {
      if (mode_ == ManualProgramMode::Constant) {
        mode_ = ManualProgramMode::Boost;
      } else if (mode_ == ManualProgramMode::Boost) {
        mode_ = ManualProgramMode::Fluctuating;
      }
    } else {
      if (mode_ == ManualProgramMode::Fluctuating) {
        mode_ = ManualProgramMode::Boost;
      } else if (mode_ == ManualProgramMode::Boost) {
        mode_ = ManualProgramMode::Constant;
      }
    }

    if (mode_ == previous) {
      return {};
    }

    resetModeDependentSelection();
    ManualProgramUiResult result;
    result.valueChanged = true;
    return result;
  }

  ManualProgramUiResult editTemperature(int8_t delta) {
    const int16_t next = static_cast<int16_t>(targetTempC_ + delta);
    if (next < 0 || next > ProfileEngine::MAX_TARGET_TEMP_C) {
      return {};
    }
    if (mode_ == ManualProgramMode::Boost &&
        next + boostDeltaC_ > ProfileEngine::MAX_TARGET_TEMP_C) {
      return {};
    }
    if (mode_ == ManualProgramMode::Fluctuating &&
        (!isUpperTempValidForReference(upperTempC_, next) ||
         !isLowerTempValidForReference(lowerTempC_, next))) {
      return {};
    }

    targetTempC_ = next;
    return changedValue();
  }

  ManualProgramUiResult editDuration(int8_t delta) {
    const int32_t next = static_cast<int32_t>(durationMinutes_) +
                         static_cast<int32_t>(delta) * DURATION_STEP_MINUTES;
    if (next <= 0 || next > ProfileEngine::MAX_DURATION_MINUTES) {
      return {};
    }
    if (mode_ == ManualProgramMode::Boost &&
        boostDurationMinutes_ * 2U > static_cast<uint16_t>(next)) {
      return {};
    }

    durationMinutes_ = static_cast<uint16_t>(next);
    return changedValue();
  }

  ManualProgramUiResult editBoostDelta(int8_t delta) {
    const int16_t next = static_cast<int16_t>(
        boostDeltaC_ + static_cast<int16_t>(delta) * BOOST_DELTA_STEP_C);
    if (next < 0 || next > MAX_BOOST_DELTA_C ||
        targetTempC_ + next > ProfileEngine::MAX_TARGET_TEMP_C) {
      return {};
    }

    boostDeltaC_ = next;
    return changedValue();
  }

  ManualProgramUiResult editBoostDuration(int8_t delta) {
    const int32_t next =
        static_cast<int32_t>(boostDurationMinutes_) +
        static_cast<int32_t>(delta) * BOOST_DURATION_STEP_MINUTES;
    if (next <= 0 || next * 2 > durationMinutes_) {
      return {};
    }

    boostDurationMinutes_ = static_cast<uint16_t>(next);
    return changedValue();
  }

  ManualProgramUiResult editUpperTemp(int8_t delta) {
    const int16_t next = static_cast<int16_t>(upperTempC_ + delta);
    if (next < lowerTempC_ || !isAllowedTemp(next) ||
        !isUpperTempValidForReference(next, targetTempC_)) {
      return {};
    }

    upperTempC_ = next;
    return changedValue();
  }

  ManualProgramUiResult editLowerTemp(int8_t delta) {
    const int16_t next = static_cast<int16_t>(lowerTempC_ + delta);
    if (next > upperTempC_ || !isAllowedTemp(next) ||
        !isLowerTempValidForReference(next, targetTempC_)) {
      return {};
    }

    lowerTempC_ = next;
    return changedValue();
  }

  ManualProgramUiResult editUpperDuration(int8_t delta) {
    return editBoundedPhaseDuration(upperDurationMinutes_, delta);
  }

  ManualProgramUiResult editLowerDuration(int8_t delta) {
    return editBoundedPhaseDuration(lowerDurationMinutes_, delta);
  }

  ManualProgramUiResult editBoundedPhaseDuration(uint16_t& value,
                                                 int8_t delta) {
    const int32_t next = static_cast<int32_t>(value) +
                         static_cast<int32_t>(delta) *
                             FLUCT_DURATION_STEP_MINUTES;
    if (next < MIN_FLUCT_DURATION_MINUTES ||
        next > MAX_FLUCT_DURATION_MINUTES) {
      return {};
    }

    value = static_cast<uint16_t>(next);
    return changedValue();
  }

  static bool isAllowedTemp(int16_t tempC) {
    return tempC >= 0 && tempC <= ProfileEngine::MAX_TARGET_TEMP_C;
  }

  static bool isUpperTempValidForReference(int16_t tempC,
                                           int16_t referenceTempC) {
    return tempC >= referenceTempC &&
           tempC - referenceTempC <= MAX_FLUCT_OFFSET_C;
  }

  static bool isLowerTempValidForReference(int16_t tempC,
                                           int16_t referenceTempC) {
    return tempC <= referenceTempC &&
           referenceTempC - tempC <= MAX_FLUCT_OFFSET_C;
  }

  ManualProgramUiResult changedValue() {
    ManualProgramUiResult result;
    result.valueChanged = true;
    dirty_ = true;
    return result;
  }

  int16_t boostTargetTempC() const {
    return static_cast<int16_t>(targetTempC_ + boostDeltaC_);
  }

  void resetSelection() {
    selectedIndex_ = 0U;
    selectedField_ = ManualProgramField::Mode;
    editing_ = false;
  }

  void resetModeDependentSelection() {
    selectedIndex_ = 0U;
    selectedField_ = ManualProgramField::Mode;
  }

  ManualProgramMode mode_ = ManualProgramMode::Constant;
  uint8_t selectedIndex_ = 0U;
  ManualProgramField selectedField_ = ManualProgramField::Mode;
  int16_t targetTempC_ = DEFAULT_TEMP_C;
  uint16_t durationMinutes_ = DEFAULT_DURATION_MINUTES;
  int16_t boostDeltaC_ = DEFAULT_BOOST_DELTA_C;
  uint16_t boostDurationMinutes_ = DEFAULT_BOOST_DURATION_MINUTES;
  int16_t upperTempC_ = DEFAULT_TEMP_C + DEFAULT_UPPER_OFFSET_C;
  int16_t lowerTempC_ = DEFAULT_TEMP_C - DEFAULT_LOWER_OFFSET_C;
  uint16_t upperDurationMinutes_ = DEFAULT_FLUCT_PHASE_MINUTES;
  uint16_t lowerDurationMinutes_ = DEFAULT_FLUCT_PHASE_MINUTES;
  bool editing_ = false;
  bool associatedSlotValid_ = false;
  uint8_t associatedSlot_ = 0U;
  bool dirty_ = false;
  ProfileConfig baselineProfile_;
  bool baselineAssociatedSlotValid_ = false;
  uint8_t baselineAssociatedSlot_ = 0U;
};

}  // namespace dehydrator
