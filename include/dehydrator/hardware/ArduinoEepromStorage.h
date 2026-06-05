#pragma once

#include <EEPROM.h>
#include <stdint.h>

#include "dehydrator/interfaces/ByteStorage.h"

namespace dehydrator {

/**
 * @brief Arduino EEPROM adapter for the project's byte-storage interface.
 */
class ArduinoEepromStorage final : public ByteStorage {
 public:
  /**
   * @brief Returns the EEPROM size reported by the Arduino core.
   */
  uint16_t length() const override {
    return static_cast<uint16_t>(EEPROM.length());
  }

  /**
   * @brief Reads one EEPROM byte.
   *
   * @param address Zero-based EEPROM address.
   * @return Stored byte value.
   */
  uint8_t read(uint16_t address) const override { return EEPROM.read(address); }

  /**
   * @brief Updates one EEPROM byte only when the value changed.
   *
   * @param address Zero-based EEPROM address.
   * @param value New byte value.
   */
  void update(uint16_t address, uint8_t value) override {
    EEPROM.update(address, value);
  }
};

}  // namespace dehydrator
