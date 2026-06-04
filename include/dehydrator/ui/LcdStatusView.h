#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "dehydrator/interfaces/CharacterDisplay.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the Romanian 4x20 status screen.
 */
struct LcdStatusSnapshot {
  /** Romanian state label, for example `INACTIV` or `RULARE`. */
  const char* stateLabel;
  /** PT50 temperature in Celsius. Ignored when `pt50Valid` is false. */
  int16_t pt50TempC;
  /** Whether PT50 temperature is available for display. */
  bool pt50Valid;
  /** Relative humidity in percent. Ignored when `rhValid` is false. */
  uint8_t rhPercent;
  /** Whether RH is available for display. */
  bool rhValid;
  /** Logical heater command shown on the status screen. */
  bool heaterOn;
  /** Logical fan command shown on the status screen. */
  bool fanOn;
  /** Whether the heartbeat custom symbol should be visible. */
  bool heartbeatOn;
};

/**
 * @brief Renders the main Romanian status screen on a 4x20 character LCD.
 *
 * The renderer is intentionally deterministic and allocation-free. It redraws
 * complete fixed-width lines so stale characters from previous values cannot
 * remain visible after a shorter value is displayed.
 */
class LcdStatusView {
 public:
  /** Number of columns on the configured LCD. */
  static constexpr uint8_t COLUMNS = 20U;
  /** Number of rows on the configured LCD. */
  static constexpr uint8_t ROWS = 4U;
  /** HD44780 custom character slot used for the heartbeat symbol. */
  static constexpr uint8_t HEARTBEAT_CHAR = 0U;

  /**
   * @brief Creates a status view for the provided character display.
   *
   * @param display Display interface receiving the rendered characters.
   */
  explicit LcdStatusView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders a full status screen.
   *
   * @param snapshot Current device status values.
   */
  void render(const LcdStatusSnapshot& snapshot) {
    char line[COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Stare:", 0U);
    writeToken(line, safeText(snapshot.stateLabel), 7U);
    writeLine(0U, line, false);

    fillLine(line);
    writeTemperatureAndHumidity(line, snapshot);
    writeLine(1U, line, false);

    fillLine(line);
    writeOutputState(line, "H:", snapshot.heaterOn, 0U);
    writeOutputState(line, "F:", snapshot.fanOn, 10U);
    writeLine(2U, line, false);

    fillLine(line);
    writeLine(3U, line, snapshot.heartbeatOn);
  }

 private:
  static const char* safeText(const char* text) {
    return text == nullptr ? "" : text;
  }

  static void fillLine(char* line) {
    for (uint8_t index = 0U; index < COLUMNS; index++) {
      line[index] = ' ';
    }
    line[COLUMNS] = '\0';
  }

  static void writeToken(char* line, const char* token, uint8_t column) {
    uint8_t writeColumn = column;
    for (size_t index = 0U;
         token[index] != '\0' && writeColumn < COLUMNS; index++) {
      line[writeColumn] = token[index];
      writeColumn++;
    }
  }

  static void writeTemperatureAndHumidity(char* line,
                                          const LcdStatusSnapshot& snapshot) {
    char value[8] = {};
    writeToken(line, "T:", 0U);
    if (snapshot.pt50Valid) {
      snprintf(value, sizeof(value), "%dC", static_cast<int>(snapshot.pt50TempC));
      writeToken(line, value, 2U);
    } else {
      writeToken(line, "--C", 2U);
    }

    writeToken(line, "RH:", 9U);
    if (snapshot.rhValid) {
      snprintf(value, sizeof(value), "%u%%",
               static_cast<unsigned int>(snapshot.rhPercent));
      writeToken(line, value, 12U);
    } else {
      writeToken(line, "--%", 12U);
    }
  }

  static void writeOutputState(char* line, const char* label, bool on,
                               uint8_t column) {
    writeToken(line, label, column);
    writeToken(line, on ? "ON" : "OFF", static_cast<uint8_t>(column + 2U));
  }

  void writeLine(uint8_t row, const char* line, bool heartbeatOn) {
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < COLUMNS; column++) {
      if (row == ROWS - 1U && column == COLUMNS - 1U && heartbeatOn) {
        display_.writeCustom(HEARTBEAT_CHAR);
      } else {
        display_.writeChar(line[column]);
      }
    }
  }

  CharacterDisplay& display_;
};

}  // namespace dehydrator
