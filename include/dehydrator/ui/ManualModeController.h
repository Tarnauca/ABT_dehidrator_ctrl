#pragma once

#include <stdint.h>

#include "dehydrator/interfaces/OutputController.h"

namespace dehydrator {

/**
 * @brief Editable fields on the manual mode screen.
 */
enum class ManualField {
  /** Fan ON/OFF selection field. */
  Fan,
  /** Heater ON/OFF selection field. */
  Heater,
};

/**
 * @brief Result of one manual mode UI action.
 */
struct ManualUiResult {
  /** True when the selected field changed. */
  bool selectionChanged = false;
  /** True when logical outputs changed. */
  bool outputChanged = false;
  /** True when the screen should close back to menu. */
  bool exitToMenu = false;
};

/**
 * @brief Minimal manual-mode editor for bring-up.
 *
 * The controller owns a logical `OutputCommand` and applies the existing
 * heater/fan safety invariant through `sanitizeOutputCommand()`. It does not
 * write hardware directly; it only updates the desired logical outputs for the
 * UI shell and future integration work.
 */
class ManualModeController {
 public:
  /**
   * @brief Returns the currently selected field.
   *
   * @return Active editable field on the manual screen.
   */
  ManualField selectedField() const { return selectedField_; }

  /**
   * @brief Returns the current logical manual output command.
   *
   * @return Manual logical output command.
   */
  OutputCommand command() const { return command_; }

  /**
   * @brief Handles one encoder step.
   *
   * @param delta Positive for clockwise, negative for counter-clockwise.
   * @return Result indicating whether selection changed.
   */
  ManualUiResult onRotate(int8_t delta) {
    if (delta == 0) {
      return {};
    }

    selectedField_ = selectedField_ == ManualField::Fan ? ManualField::Heater
                                                        : ManualField::Fan;
    ManualUiResult result;
    result.selectionChanged = true;
    return result;
  }

  /**
   * @brief Handles a short press by toggling the selected logical output.
   *
   * @return Result indicating whether outputs changed.
   */
  ManualUiResult onShortPress() {
    const OutputCommand previous = command_;

    if (selectedField_ == ManualField::Fan) {
      command_.fanOn = !command_.fanOn;
    } else {
      command_.heaterOn = !command_.heaterOn;
      if (command_.heaterOn) {
        command_.fanOn = true;
      }
    }

    command_ = sanitizeOutputCommand(command_);

    ManualUiResult result;
    result.outputChanged = previous.heaterOn != command_.heaterOn ||
                           previous.fanOn != command_.fanOn;
    return result;
  }

  /**
   * @brief Handles a long press by leaving manual mode.
   *
   * @return Result requesting a return to the menu.
   */
  ManualUiResult onLongPress() {
    ManualUiResult result;
    result.exitToMenu = true;
    return result;
  }

 private:
  ManualField selectedField_ = ManualField::Fan;
  OutputCommand command_;
};

}  // namespace dehydrator
