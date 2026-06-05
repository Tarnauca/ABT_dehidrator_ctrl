#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dehydrator/domain/ProfileEngine.h"
#include "dehydrator/interfaces/CharacterDisplay.h"

namespace dehydrator {

/**
 * @brief Logical pages available on the main Romanian status screen.
 */
enum class StatusPage {
  /** Summary page with program, sensors, and elapsed/remaining time. */
  Summary,
  /** First parameter page for the active program. */
  ParametersPrimary,
  /** Second parameter page when the program has more than four parameters. */
  ParametersSecondary,
  /** Output-state page. */
  Outputs,
};

/**
 * @brief Blinking activity indicator shown on the main status screen.
 */
enum class StatusActivityIndicator {
  /** No activity indicator is shown. */
  None,
  /** Blinking play indicator while a run is actively executing. */
  Running,
  /** Blinking pause indicator while a run is paused. */
  Paused,
};

/**
 * @brief Snapshot rendered by the Romanian 4x20 status screen.
 */
struct LcdStatusSnapshot {
  /** Current logical page selected by the user. */
  StatusPage page = StatusPage::Summary;
  /** Current active program label, for example `Mere` or `Inactiv`. */
  const char* programLabel = nullptr;
  /** Primary thermistor temperature in Celsius. Ignored when `ntcValid` is false. */
  int16_t ntcTempC = 0;
  /** Whether primary thermistor temperature is available for display. */
  bool ntcValid = false;
  /** Relative humidity in percent. Ignored when `rhValid` is false. */
  uint8_t rhPercent = 0U;
  /** Whether RH is available for display. */
  bool rhValid = false;
  /** Elapsed active program time in minutes. */
  uint16_t elapsedMinutes = 0U;
  /** Remaining active program time in minutes. */
  uint16_t remainingMinutes = 0U;
  /** True when an active program profile is available for parameter pages. */
  bool profileValid = false;
  /** Current active program profile for parameter rendering. */
  ProfileConfig profile;
  /** Logical heater command shown on the status screen. */
  bool heaterOn = false;
  /** Logical fan command shown on the status screen. */
  bool fanOn = false;
  /** Blinking run/pause indicator shown in the summary top-right corner. */
  StatusActivityIndicator activityIndicator = StatusActivityIndicator::None;
  /** Whether the current activity indicator should be visible on this refresh. */
  bool activityIndicatorOn = false;
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
  /** HD44780 custom character slot used for the blinking play symbol. */
  static constexpr uint8_t PLAY_CHAR = 0U;
  /** HD44780 custom character slot used for the blinking pause symbol. */
  static constexpr uint8_t PAUSE_CHAR = 1U;

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
    lastSnapshot_ = snapshot;
    char line[COLUMNS + 1U] = {};

    switch (snapshot.page) {
      case StatusPage::ParametersPrimary:
        renderPrimaryParameters(snapshot, line);
        break;
      case StatusPage::ParametersSecondary:
        renderSecondaryParameters(snapshot, line);
        break;
      case StatusPage::Outputs:
        renderOutputs(snapshot, line);
        break;
      case StatusPage::Summary:
      default:
        renderSummary(snapshot, line);
        break;
    }
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

  /**
   * @brief Formats one whole-number duration as `8h 15m`.
   *
   * @param minutesValue Duration in minutes.
   * @param buffer Destination string buffer.
   * @param bufferSize Destination buffer size.
   */
  static void formatDuration(uint16_t minutesValue, char* buffer,
                             size_t bufferSize) {
    snprintf(buffer, bufferSize, "%uh %um",
             static_cast<unsigned int>(minutesValue / 60U),
             static_cast<unsigned int>(minutesValue % 60U));
  }

  /**
   * @brief Writes `Label: duration` using the project display format.
   *
   * @param line Destination LCD line.
   * @param label Prefix label, including colon.
   * @param minutesValue Duration in minutes.
   */
  static void writeDurationLine(char* line, const char* label,
                                uint16_t minutesValue) {
    char value[12] = {};
    writeToken(line, label, 0U);
    formatDuration(minutesValue, value, sizeof(value));
    writeToken(line, value, static_cast<uint8_t>(strlen(label)));
  }

  /**
   * @brief Writes `Label: temp°C` using the shared LCD temperature format.
   *
   * @param line Destination LCD line.
   * @param label Prefix label, including colon.
   * @param tempC Integer Celsius value.
   */
  static void writeTemperatureLine(char* line, const char* label, int16_t tempC) {
    char value[16] = {};
    writeToken(line, label, 0U);
    snprintf(value, sizeof(value), "%d\xDF""C", static_cast<int>(tempC));
    writeToken(line, value, static_cast<uint8_t>(strlen(label)));
  }

  /**
   * @brief Writes `Label: temp` / `Label: --` for the summary sensor row.
   *
   * @param line Destination LCD line.
   * @param snapshot Current sensor snapshot.
   */
  static void writeTemperatureAndHumidity(char* line,
                                          const LcdStatusSnapshot& snapshot) {
    char value[16] = {};
    writeToken(line, "Temp:", 0U);
    if (snapshot.ntcValid) {
      snprintf(value, sizeof(value), "%d\xDF""C",
               static_cast<int>(snapshot.ntcTempC));
      writeToken(line, value, 6U);
    } else {
      writeToken(line, "--\xDF""C", 6U);
    }

    writeToken(line, "RH: ", 11U);
    if (snapshot.rhValid) {
      snprintf(value, sizeof(value), "%u%%",
               static_cast<unsigned int>(snapshot.rhPercent));
      writeToken(line, value, 15U);
    } else {
      writeToken(line, "--%", 15U);
    }
  }

  /**
   * @brief Writes the current program line on the summary page.
   *
   * The LCD width cannot always fit the full long-form label after the
   * `Program:` prefix, so callers should already provide a compact status label.
   *
   * @param line Destination LCD line.
   * @param programLabel Compact program label to show.
   */
  static void writeProgramLine(char* line, const char* programLabel) {
    writeToken(line, "Program:", 0U);
    writeToken(line, safeText(programLabel), 9U);
  }

  /**
   * @brief Returns the custom-character code for the current activity indicator.
   *
   * @param snapshot Current status snapshot.
   * @return Custom-character code, or 0xFF when no indicator should be drawn.
   */
  static uint8_t activityIndicatorCode(const LcdStatusSnapshot& snapshot) {
    if (snapshot.page != StatusPage::Summary) {
      return 0xFFU;
    }
    if (!snapshot.activityIndicatorOn) {
      return 0xFFU;
    }

    if (snapshot.activityIndicator == StatusActivityIndicator::Running) {
      return PLAY_CHAR;
    }
    if (snapshot.activityIndicator == StatusActivityIndicator::Paused) {
      return PAUSE_CHAR;
    }
    return 0xFFU;
  }

  /**
   * @brief Writes one output-state line.
   *
   * @param line Destination LCD line.
   * @param label Romanian output label.
   * @param on Whether the output is currently active.
   */
  static void writeOutputStateLine(char* line, const char* label, bool on) {
    writeToken(line, label, 0U);
    writeToken(line, on ? "Pornit" : "Oprit",
               static_cast<uint8_t>(strlen(label)));
  }

  /**
   * @brief Renders the summary page.
   *
   * @param snapshot Current status data.
   * @param line Reusable line buffer.
   */
  void renderSummary(const LcdStatusSnapshot& snapshot, char* line) {
    fillLine(line);
    writeProgramLine(line, snapshot.programLabel);
    writeLine(0U, line);

    fillLine(line);
    writeTemperatureAndHumidity(line, snapshot);
    writeLine(1U, line);

    fillLine(line);
    writeDurationLine(line, "Timp scurs: ", snapshot.elapsedMinutes);
    writeLine(2U, line);

    fillLine(line);
    writeDurationLine(line, "Timp ramas: ", snapshot.remainingMinutes);
    writeLine(3U, line);
  }

  /**
   * @brief Renders the first parameter page for the active profile.
   *
   * @param snapshot Current status/profile data.
   * @param line Reusable line buffer.
   */
  void renderPrimaryParameters(const LcdStatusSnapshot& snapshot, char* line) {
    fillLine(line);
    if (!snapshot.profileValid) {
      writeToken(line, "Fara program activ", 0U);
      writeLine(0U, line);
      fillLine(line);
      writeLine(1U, line);
      fillLine(line);
      writeLine(2U, line);
      fillLine(line);
      writeLine(3U, line);
      return;
    }

    if (snapshot.profile.mode == ProfileMode::Fixed) {
      writeTemperatureLine(line, "Temp: ", snapshot.profile.targetTempC);
      writeLine(0U, line);
      fillLine(line);
      writeDurationLine(line, "Durata: ", snapshot.profile.durationMinutes);
      writeLine(1U, line);
      fillLine(line);
      writeLine(2U, line);
      fillLine(line);
      writeLine(3U, line);
      return;
    }

    if (snapshot.profile.mode == ProfileMode::Boost) {
      writeTemperatureLine(line, "Temp: ", snapshot.profile.targetTempC);
      writeLine(0U, line);
      fillLine(line);
      writeDurationLine(line, "Durata: ", snapshot.profile.durationMinutes);
      writeLine(1U, line);
      fillLine(line);
      writeTemperatureLine(line, "Boost: +", snapshot.profile.highTempC -
                                                snapshot.profile.targetTempC);
      writeLine(2U, line);
      fillLine(line);
      writeDurationLine(line, "Dur.boost: ", snapshot.profile.highPhaseMinutes);
      writeLine(3U, line);
      return;
    }

    writeTemperatureLine(line, "T.ref: ", snapshot.profile.targetTempC);
    writeLine(0U, line);
    fillLine(line);
    writeDurationLine(line, "Durata: ", snapshot.profile.durationMinutes);
    writeLine(1U, line);
    fillLine(line);
    writeTemperatureLine(line, "Tsup: ", snapshot.profile.highTempC);
    writeLine(2U, line);
    fillLine(line);
    writeTemperatureLine(line, "Tinf: ", snapshot.profile.lowTempC);
    writeLine(3U, line);
  }

  /**
   * @brief Renders the secondary fluctuating-parameter page when needed.
   *
   * @param snapshot Current status/profile data.
   * @param line Reusable line buffer.
   */
  void renderSecondaryParameters(const LcdStatusSnapshot& snapshot, char* line) {
    fillLine(line);
    if (!(snapshot.profileValid &&
          snapshot.profile.mode == ProfileMode::Fluctuating)) {
      writeLine(0U, line);
      fillLine(line);
      writeLine(1U, line);
      fillLine(line);
      writeLine(2U, line);
      fillLine(line);
      writeLine(3U, line);
      return;
    }

    writeDurationLine(line, "Dur. Tsup: ", snapshot.profile.highPhaseMinutes);
    writeLine(0U, line);
    fillLine(line);
    writeDurationLine(line, "Dur. Tinf: ", snapshot.profile.lowPhaseMinutes);
    writeLine(1U, line);
    fillLine(line);
    writeLine(2U, line);
    fillLine(line);
    writeLine(3U, line);
  }

  /**
   * @brief Renders the heater/fan output page.
   *
   * @param snapshot Current output state data.
   * @param line Reusable line buffer.
   */
  void renderOutputs(const LcdStatusSnapshot& snapshot, char* line) {
    fillLine(line);
    writeOutputStateLine(line, "Incalzitor: ", snapshot.heaterOn);
    writeLine(0U, line);

    fillLine(line);
    writeOutputStateLine(line, "Ventilator: ", snapshot.fanOn);
    writeLine(1U, line);

    fillLine(line);
    writeLine(2U, line);

    fillLine(line);
    writeLine(3U, line);
  }

  void writeLine(uint8_t row, const char* line) {
    const uint8_t indicatorCode =
        row == 0U ? activityIndicatorCode(lastSnapshot_) : 0xFFU;
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < COLUMNS; column++) {
      if (row == 0U && column == COLUMNS - 1U && indicatorCode != 0xFFU) {
        display_.writeCustom(indicatorCode);
      } else {
        display_.writeChar(line[column]);
      }
    }
  }

  CharacterDisplay& display_;
  LcdStatusSnapshot lastSnapshot_;
};

}  // namespace dehydrator
