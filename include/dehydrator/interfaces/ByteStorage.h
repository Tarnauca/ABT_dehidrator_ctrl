#pragma once

#include <stdint.h>

namespace dehydrator {

/**
 * @brief Byte-addressable non-volatile storage abstraction.
 *
 * This interface keeps EEPROM persistence testable in native builds while the
 * Arduino target can provide a thin adapter around the real EEPROM API.
 */
class ByteStorage {
 public:
  virtual ~ByteStorage() = default;

  /**
   * @brief Returns the total storage size in bytes.
   */
  virtual uint16_t length() const = 0;

  /**
   * @brief Reads one byte from the given address.
   *
   * @param address Zero-based storage byte offset.
   * @return Stored byte value.
   */
  virtual uint8_t read(uint16_t address) const = 0;

  /**
   * @brief Updates one byte only when the value has changed.
   *
   * @param address Zero-based storage byte offset.
   * @param value New byte value to persist.
   */
  virtual void update(uint16_t address, uint8_t value) = 0;
};

}  // namespace dehydrator
