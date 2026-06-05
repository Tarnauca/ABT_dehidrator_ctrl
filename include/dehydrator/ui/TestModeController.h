#pragma once

#include <stdint.h>

#include "dehydrator/interfaces/OutputController.h"

namespace dehydrator {

/**
 * @brief Selectable fields on the direct-output test screen.
 */
enum class TestField {
  /** Primary NTC temperature read-only row. */
  NtcTemp,
  /** Secondary AM2302 temperature read-only row. */
  TempRhTemp,
  /** Secondary AM2302 humidity read-only row. */
  TempRhRh,
  /** Fan ON/OFF selection field. */
  Fan,
  /** Heater ON/OFF selection field. */
  Heater,
  /** Back entry that returns to the previous menu level. */
  Back,
};

/**
 * @brief Result of one direct-output test UI action.
 */
struct TestUiResult {
  /** True when the selected field changed. */
  bool selectionChanged = false;
  /** True when logical outputs changed. */
  bool outputChanged = false;
  /** True when the screen should close back to the previous menu level. */
  bool exitToMenu = false;
};

/**
 * @brief Minimal direct-output test controller for bring-up.
 *
 * The controller owns a logical `OutputCommand` and applies the existing
 * heater/fan safety invariant through `sanitizeOutputCommand()`. It does not
 * write hardware directly; it only updates the desired logical outputs for the
 * UI shell and future integration work.
 */
class TestModeController {
 public:
  /**
   * @brief Returns the currently selected field.
   *
   * @return Active selectable field on the test screen.
   */
  TestField selectedField() const { return selectedField_; }

  /**
   * @brief Returns the current logical test output command.
   *
   * @return Test logical output command.
   */
  OutputCommand command() const { return command_; }

  /**
   * @brief Handles one encoder step.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  TestUiResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    if (delta > 0) {
      if (selectedField_ == TestField::NtcTemp) {
        selectedField_ = TestField::TempRhTemp;
      } else if (selectedField_ == TestField::TempRhTemp) {
        selectedField_ = TestField::TempRhRh;
      } else if (selectedField_ == TestField::TempRhRh) {
        selectedField_ = TestField::Fan;
      } else if (selectedField_ == TestField::Fan) {
        selectedField_ = TestField::Heater;
      } else if (selectedField_ == TestField::Heater) {
        selectedField_ = TestField::Back;
      }
    } else {
      if (selectedField_ == TestField::Back) {
        selectedField_ = TestField::Heater;
      } else if (selectedField_ == TestField::Heater) {
        selectedField_ = TestField::Fan;
      } else if (selectedField_ == TestField::Fan) {
        selectedField_ = TestField::TempRhRh;
      } else if (selectedField_ == TestField::TempRhRh) {
        selectedField_ = TestField::TempRhTemp;
      } else if (selectedField_ == TestField::TempRhTemp) {
        selectedField_ = TestField::NtcTemp;
      }
    }

    TestUiResult result;
    result.selectionChanged = true;
    return result;
  }

  /**
   * @brief Handles a short press by toggling the selected logical output.
   *
   * @return Result indicating whether outputs changed.
   */
  TestUiResult onShortPress() {
    if (selectedField_ == TestField::Back) {
      selectedField_ = TestField::NtcTemp;
      TestUiResult result;
      result.exitToMenu = true;
      return result;
    }

    if (selectedField_ == TestField::NtcTemp ||
        selectedField_ == TestField::TempRhTemp ||
        selectedField_ == TestField::TempRhRh) {
      return {};
    }

    const OutputCommand previous = command_;

    if (selectedField_ == TestField::Fan) {
      command_.fanOn = !command_.fanOn;
    } else {
      command_.heaterOn = !command_.heaterOn;
      if (command_.heaterOn) {
        command_.fanOn = true;
      }
    }

    command_ = sanitizeOutputCommand(command_);

    TestUiResult result;
    result.outputChanged = previous.heaterOn != command_.heaterOn ||
                           previous.fanOn != command_.fanOn;
    return result;
  }

  /**
   * @brief Handles a long press with no assigned action in the current scope.
   *
   * @return Result with no externally visible action.
   */
  TestUiResult onLongPress() const { return {}; }

 private:
  TestField selectedField_ = TestField::NtcTemp;
  OutputCommand command_;
};

}  // namespace dehydrator
