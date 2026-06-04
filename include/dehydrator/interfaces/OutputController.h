#pragma once

namespace dehydrator {

/**
 * @brief Logical output command before hardware pin polarity is applied.
 *
 * These are logical device commands, not raw MCU pin levels. Hardware adapters
 * translate them to physical outputs using board configuration such as relay
 * polarity and backlight driver polarity.
 */
struct OutputCommand {
  /** Logical heater command. Must never remain true when `fanOn` is false. */
  bool heaterOn = false;
  /** Logical fan command. Heater operation requires this to be true. */
  bool fanOn = false;
  /** Logical buzzer command for finish/fault alarms. */
  bool buzzerOn = false;
  /** Logical LCD backlight command for normal illumination/alarm blinking. */
  bool backlightOn = false;
};

/**
 * @brief Enforces command-level output safety invariants.
 *
 * The heater/fan invariant is enforced in multiple layers. This pure helper
 * ensures a command cannot request heater ON while fan is OFF, before any
 * hardware adapter translates the command to MCU pin levels.
 *
 * @param command Logical command requested by higher-level control logic.
 * @return Sanitized command with `heaterOn` forced false when `fanOn` is false.
 */
inline OutputCommand sanitizeOutputCommand(OutputCommand command) {
  if (!command.fanOn) {
    command.heaterOn = false;
  }
  return command;
}

/**
 * @brief Interface for applying logical output commands.
 *
 * Implementations may write real MCU pins or capture commands in tests. Real
 * hardware implementations must also defensively enforce the heater/fan safety
 * invariant before writing relay outputs.
 */
class OutputController {
 public:
  /**
   * @brief Virtual destructor for safe polymorphic use.
   */
  virtual ~OutputController() = default;

  /**
   * @brief Applies a logical output command.
   *
   * @param command Desired logical output command. Implementations should
   * sanitize or reject unsafe heater/fan combinations.
   */
  virtual void apply(OutputCommand command) = 0;
};

}  // namespace dehydrator
