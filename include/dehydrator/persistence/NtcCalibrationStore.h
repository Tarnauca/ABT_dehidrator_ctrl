#pragma once

#include <stdint.h>

#include "dehydrator/config/RuntimeConfig.h"
#include "dehydrator/interfaces/ByteStorage.h"
#include "dehydrator/persistence/UserProfileStore.h"

namespace dehydrator {

/**
 * @brief Fixed-layout EEPROM repository for persisted NTC calibration overrides.
 *
 * Only the user-tunable offset and scale are stored here. The rest of the NTC
 * conversion model remains defined by firmware defaults.
 */
class NtcCalibrationStore {
 public:
  /** Stable storage-layout version. */
  static constexpr uint8_t STORAGE_VERSION = 1U;
  /** Base EEPROM offset reserved for NTC calibration overrides. */
  static constexpr uint16_t BASE_ADDRESS = static_cast<uint16_t>(
      UserProfileStore::BASE_ADDRESS +
      UserProfileStore::SLOT_COUNT * static_cast<uint16_t>(17U) + 16U);

  /**
   * @brief Creates a repository over one byte-addressable storage backend.
   *
   * @param storage EEPROM-like backend.
   */
  explicit NtcCalibrationStore(ByteStorage& storage) : storage_(storage) {}

  /**
   * @brief Returns whether the configured storage is large enough.
   */
  bool available() const {
    return storage_.length() >=
           static_cast<uint16_t>(BASE_ADDRESS + recordSize());
  }

  /**
   * @brief Loads one persisted NTC calibration override into the active config.
   *
   * @param calibration Active calibration config to update.
   * @return true when a valid persisted override was loaded and applied.
   */
  bool load(config::CalibrationConfig& calibration) const {
    if (!available()) {
      return false;
    }

    StoredCalibration stored = {};
    readRecord(stored);
    if (!isStoredCalibrationValid(stored)) {
      return false;
    }

    calibration.ntcOffsetCentiC = stored.ntcOffsetCentiC;
    calibration.ntcScalePpm = stored.ntcScalePpm;
    return true;
  }

  /**
   * @brief Saves one NTC calibration override.
   *
   * @param ntcOffsetCentiC Offset in centi-Celsius.
   * @param ntcScalePpm Scale factor in parts per million.
   * @return true when the override was written.
   */
  bool save(int16_t ntcOffsetCentiC, int32_t ntcScalePpm) {
    if (!available() || ntcScalePpm <= 0) {
      return false;
    }

    StoredCalibration stored = {};
    stored.version = STORAGE_VERSION;
    stored.ntcOffsetCentiC = ntcOffsetCentiC;
    stored.ntcScalePpm = ntcScalePpm;
    stored.checksum = checksum(stored);
    writeRecord(stored);
    return true;
  }

  /**
   * @brief Persists the firmware-default NTC calibration values.
   *
   * @return true when the defaults were written.
   */
  bool restoreDefaults() {
    return save(config::CALIBRATION.ntcOffsetCentiC,
                config::CALIBRATION.ntcScalePpm);
  }

 private:
  struct StoredCalibration {
    uint8_t version;
    int16_t ntcOffsetCentiC;
    int32_t ntcScalePpm;
    uint16_t checksum;
  };

  static constexpr uint16_t recordSize() { return 9U; }

  static uint16_t checksum(const StoredCalibration& stored) {
    return static_cast<uint16_t>(
        stored.version + static_cast<uint16_t>(stored.ntcOffsetCentiC) +
        static_cast<uint16_t>(stored.ntcScalePpm & 0xFFFFU) +
        static_cast<uint16_t>((static_cast<uint32_t>(stored.ntcScalePpm) >>
                               16U) &
                              0xFFFFU));
  }

  static bool isStoredCalibrationValid(const StoredCalibration& stored) {
    return stored.version == STORAGE_VERSION &&
           stored.ntcScalePpm > 0 &&
           stored.checksum == checksum(stored);
  }

  void readRecord(StoredCalibration& stored) const {
    stored.version = readU8(BASE_ADDRESS + 0U);
    stored.ntcOffsetCentiC = readI16(BASE_ADDRESS + 1U);
    stored.ntcScalePpm = readI32(BASE_ADDRESS + 3U);
    stored.checksum = readU16(BASE_ADDRESS + 7U);
  }

  void writeRecord(const StoredCalibration& stored) {
    writeU8(BASE_ADDRESS + 0U, stored.version);
    writeI16(BASE_ADDRESS + 1U, stored.ntcOffsetCentiC);
    writeI32(BASE_ADDRESS + 3U, stored.ntcScalePpm);
    writeU16(BASE_ADDRESS + 7U, stored.checksum);
  }

  uint8_t readU8(uint16_t address) const { return storage_.read(address); }

  int16_t readI16(uint16_t address) const {
    return static_cast<int16_t>(readU16(address));
  }

  uint16_t readU16(uint16_t address) const {
    return static_cast<uint16_t>(static_cast<uint16_t>(storage_.read(address)) |
                                 (static_cast<uint16_t>(storage_.read(
                                      static_cast<uint16_t>(address + 1U)))
                                  << 8U));
  }

  int32_t readI32(uint16_t address) const {
    return static_cast<int32_t>(
        static_cast<uint32_t>(storage_.read(address)) |
        (static_cast<uint32_t>(
             storage_.read(static_cast<uint16_t>(address + 1U)))
         << 8U) |
        (static_cast<uint32_t>(
             storage_.read(static_cast<uint16_t>(address + 2U)))
         << 16U) |
        (static_cast<uint32_t>(
             storage_.read(static_cast<uint16_t>(address + 3U)))
         << 24U));
  }

  void writeU8(uint16_t address, uint8_t value) { storage_.update(address, value); }

  void writeI16(uint16_t address, int16_t value) {
    writeU16(address, static_cast<uint16_t>(value));
  }

  void writeU16(uint16_t address, uint16_t value) {
    storage_.update(address, static_cast<uint8_t>(value & 0xFFU));
    storage_.update(static_cast<uint16_t>(address + 1U),
                    static_cast<uint8_t>((value >> 8U) & 0xFFU));
  }

  void writeI32(uint16_t address, int32_t value) {
    const uint32_t raw = static_cast<uint32_t>(value);
    storage_.update(address, static_cast<uint8_t>(raw & 0xFFU));
    storage_.update(static_cast<uint16_t>(address + 1U),
                    static_cast<uint8_t>((raw >> 8U) & 0xFFU));
    storage_.update(static_cast<uint16_t>(address + 2U),
                    static_cast<uint8_t>((raw >> 16U) & 0xFFU));
    storage_.update(static_cast<uint16_t>(address + 3U),
                    static_cast<uint8_t>((raw >> 24U) & 0xFFU));
  }

  ByteStorage& storage_;
};

}  // namespace dehydrator
