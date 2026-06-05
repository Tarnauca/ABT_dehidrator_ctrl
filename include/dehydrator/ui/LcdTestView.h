#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/interfaces/OutputController.h"
#include "dehydrator/sensors/NtcReader.h"
#include "dehydrator/sensors/TempRhReader.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/TestModeController.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by the direct-output test screen.
 */
struct LcdTestSnapshot {
  /** Selected editable field. */
  TestField selectedField = TestField::NtcTemp;
  /** Latest primary NTC reading. */
  NtcReading ntc;
  /** Latest secondary temp/RH reading. */
  TempRhReading tempRh;
  /** Current logical test output command. */
  OutputCommand command;
};

/**
 * @brief Renders the 4x20 Romanian direct-output test screen.
 */
class LcdTestView {
 public:
  /**
   * @brief Creates a test view for the provided character display.
   *
   * @param display LCD character display interface.
   */
  explicit LcdTestView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current test mode state.
   *
   * @param snapshot Current selection and logical outputs.
   */
  void render(const LcdTestSnapshot& snapshot) {
    static constexpr TestField kVisibleFields[] = {
        TestField::NtcTemp, TestField::TempRhTemp, TestField::TempRhRh,
        TestField::Fan,     TestField::Heater,     TestField::Back,
    };

    const uint8_t selectedIndex = fieldIndex(snapshot.selectedField);
    const uint8_t windowStart = selectedIndex > 2U ? selectedIndex - 2U : 0U;
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    writeToken(line, "Testare", 0U);
    writeLine(0U, line);

    for (uint8_t row = 0U; row < 3U; row++) {
      fillLine(line);
      const uint8_t itemIndex = windowStart + row;
      if (itemIndex < sizeof(kVisibleFields) / sizeof(kVisibleFields[0])) {
        const TestField field = kVisibleFields[itemIndex];
        line[0] = field == snapshot.selectedField ? '>' : ' ';
        renderField(line, field, snapshot);
      }
      writeLine(static_cast<uint8_t>(row + 1U), line);
    }
  }

 private:
  static void fillLine(char* line) {
    for (uint8_t index = 0U; index < LcdStatusView::COLUMNS; index++) {
      line[index] = ' ';
    }
    line[LcdStatusView::COLUMNS] = '\0';
  }

  static void writeToken(char* line, const char* token, uint8_t column) {
    if (token == nullptr) {
      return;
    }

    uint8_t writeColumn = column;
    for (size_t index = 0U;
         token[index] != '\0' && writeColumn < LcdStatusView::COLUMNS; index++) {
      line[writeColumn] = token[index];
      writeColumn++;
    }
  }

  static uint8_t fieldIndex(TestField field) {
    switch (field) {
      case TestField::NtcTemp:
        return 0U;
      case TestField::TempRhTemp:
        return 1U;
      case TestField::TempRhRh:
        return 2U;
      case TestField::Fan:
        return 3U;
      case TestField::Heater:
        return 4U;
      case TestField::Back:
      default:
        return 5U;
    }
  }

  static void writeSignedTemperature(char* line, uint8_t column,
                                     int16_t tempDeciC) {
    char value[12] = {};
    const bool negative = tempDeciC < 0;
    const int16_t magnitude =
        static_cast<int16_t>(negative ? -tempDeciC : tempDeciC);
    const int printed =
        snprintf(value, sizeof(value), "%s%d.%d", negative ? "-" : "",
                 static_cast<int>(magnitude / 10),
                 static_cast<int>(magnitude % 10));
    if (printed <= 0) {
      return;
    }

    writeToken(line, value, column);
    uint8_t endColumn = column;
    while (endColumn < LcdStatusView::COLUMNS && line[endColumn] != ' ') {
      endColumn++;
    }

    if (endColumn + 1U < LcdStatusView::COLUMNS) {
      line[endColumn] = static_cast<char>(223);
      line[endColumn + 1U] = 'C';
    }
  }

  static void renderField(char* line, TestField field,
                          const LcdTestSnapshot& snapshot) {
    switch (field) {
      case TestField::NtcTemp:
        writeToken(line, "NTC:", 1U);
        if (snapshot.ntc.valid) {
          writeSignedTemperature(line, 6U, snapshot.ntc.tempDeciC);
        } else {
          writeToken(line, "Eroare", 6U);
        }
        return;
      case TestField::TempRhTemp:
        writeToken(line, "AM2302 T:", 1U);
        if (snapshot.tempRh.valid) {
          writeSignedTemperature(line, 11U, snapshot.tempRh.tempDeciC);
        } else {
          writeToken(line, "Eroare", 11U);
        }
        return;
      case TestField::TempRhRh:
        writeToken(line, "AM2302 RH:", 1U);
        if (snapshot.tempRh.valid) {
          char value[8] = {};
          const int printed =
              snprintf(value, sizeof(value), "%u%%", snapshot.tempRh.rhPercent);
          if (printed > 0) {
            writeToken(line, value, 12U);
          }
        } else {
          writeToken(line, "Eroare", 12U);
        }
        return;
      case TestField::Fan:
        writeToken(line, "Fan:", 1U);
        writeToken(line, snapshot.command.fanOn ? "ON" : "OFF", 6U);
        return;
      case TestField::Heater:
        writeToken(line, "Heat:", 1U);
        writeToken(line, snapshot.command.heaterOn ? "ON" : "OFF", 7U);
        return;
      case TestField::Back:
      default:
        writeToken(line, "Inapoi", 1U);
        return;
    }
  }

  void writeLine(uint8_t row, const char* line) {
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
      display_.writeChar(line[column]);
    }
  }

  CharacterDisplay& display_;
};

}  // namespace dehydrator
