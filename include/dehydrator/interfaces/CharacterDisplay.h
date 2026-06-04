#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Minimal character LCD interface used by UI renderers.
 *
 * The interface keeps display rendering independent from a specific HD44780
 * library while still mapping directly to common 4x20 LCD operations.
 */
class CharacterDisplay {
 public:
  virtual ~CharacterDisplay() = default;

  /**
   * @brief Sets the cursor position before writing characters.
   *
   * @param column Zero-based display column.
   * @param row Zero-based display row.
   */
  virtual void setCursor(uint8_t column, uint8_t row) = 0;

  /**
   * @brief Writes one printable character at the current cursor position.
   *
   * @param value Character byte to write.
   */
  virtual void writeChar(char value) = 0;

  /**
   * @brief Writes one custom character code at the current cursor position.
   *
   * @param code Custom character slot code.
   */
  virtual void writeCustom(uint8_t code) = 0;
};

}  // namespace dehydrator
