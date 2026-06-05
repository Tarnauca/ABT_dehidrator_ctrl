#pragma once

#include <stddef.h>
#include <stdint.h>
#include "dehydrator/domain/ProfileEngine.h"
#include "dehydrator/interfaces/ByteStorage.h"

namespace dehydrator {

/**
 * @brief Occupancy and profile payload for one saved user slot.
 */
struct UserProfileSlotRecord {
  /** True when the slot contains one valid saved profile. */
  bool occupied = false;
  /** Saved drying profile for the slot when occupied. */
  ProfileConfig profile;
};

/**
 * @brief Fixed-layout EEPROM repository for 10 user-defined manual profiles.
 *
 * The record format is intentionally simple: version, occupied flag, profile
 * payload, and checksum. Invalid or erased records are treated as vacant.
 */
class UserProfileStore {
 public:
  /** Number of user profile slots exposed in the UI. */
  static constexpr uint8_t SLOT_COUNT = 10U;
  /** Stable storage-layout version. */
  static constexpr uint8_t STORAGE_VERSION = 1U;
  /** Base EEPROM offset reserved for user profiles. */
  static constexpr uint16_t BASE_ADDRESS = 0U;

  /**
   * @brief Creates a repository over one byte-addressable storage backend.
   *
   * @param storage EEPROM-like backend.
   */
  explicit UserProfileStore(ByteStorage& storage) : storage_(storage) {}

  /**
   * @brief Returns whether the configured storage is large enough.
   */
  bool available() const {
    return storage_.length() >=
           static_cast<uint16_t>(BASE_ADDRESS + SLOT_COUNT * recordSize());
  }

  /**
   * @brief Loads one slot record.
   *
   * Invalid/corrupt records are treated as vacant to keep the UI resilient.
   *
   * @param slotIndex Zero-based slot index.
   * @param record Output record for the slot.
   * @return true when the slot bytes were readable from available storage.
   */
  bool load(uint8_t slotIndex, UserProfileSlotRecord& record) const {
    record = {};
    if (!isValidSlot(slotIndex) || !available()) {
      return false;
    }

    StoredSlot stored = {};
    readRecord(slotIndex, stored);
    if (!isStoredSlotValid(stored)) {
      return true;
    }

    record.occupied = stored.occupied != 0U;
    if (record.occupied) {
      record.profile = toProfile(stored);
    }
    return true;
  }

  /**
   * @brief Saves one profile into the given slot.
   *
   * @param slotIndex Zero-based slot index.
   * @param profile Profile to persist.
   * @return true when the profile was written.
   */
  bool save(uint8_t slotIndex, const ProfileConfig& profile) {
    if (!isValidSlot(slotIndex) || !available() ||
        !ProfileEngine::isValid(profile)) {
      return false;
    }

    StoredSlot stored = {};
    stored.version = STORAGE_VERSION;
    stored.occupied = 1U;
    stored.mode = static_cast<uint8_t>(profile.mode);
    stored.targetTempC = profile.targetTempC;
    stored.lowTempC = profile.lowTempC;
    stored.highTempC = profile.highTempC;
    stored.durationMinutes = profile.durationMinutes;
    stored.highPhaseMinutes = profile.highPhaseMinutes;
    stored.lowPhaseMinutes = profile.lowPhaseMinutes;
    stored.checksum = checksum(stored);
    writeRecord(slotIndex, stored);
    return true;
  }

  /**
   * @brief Clears one slot so it appears vacant to the UI.
   *
   * @param slotIndex Zero-based slot index.
   * @return true when the slot was cleared.
   */
  bool clear(uint8_t slotIndex) {
    if (!isValidSlot(slotIndex) || !available()) {
      return false;
    }

    StoredSlot stored = {};
    stored.version = STORAGE_VERSION;
    stored.occupied = 0U;
    stored.checksum = checksum(stored);
    writeRecord(slotIndex, stored);
    return true;
  }

 private:
  static constexpr uint16_t RECORD_SIZE = 17U;

  struct StoredSlot {
    uint8_t version;
    uint8_t occupied;
    uint8_t mode;
    int16_t targetTempC;
    int16_t lowTempC;
    int16_t highTempC;
    uint16_t durationMinutes;
    uint16_t highPhaseMinutes;
    uint16_t lowPhaseMinutes;
    uint16_t checksum;
  };

  static constexpr uint16_t recordSize() { return RECORD_SIZE; }

  static bool isValidSlot(uint8_t slotIndex) { return slotIndex < SLOT_COUNT; }

  static ProfileConfig toProfile(const StoredSlot& stored) {
    return {static_cast<ProfileMode>(stored.mode), stored.targetTempC,
            stored.lowTempC, stored.highTempC, stored.durationMinutes,
            stored.highPhaseMinutes, stored.lowPhaseMinutes};
  }

  static uint16_t checksum(const StoredSlot& stored) {
    return static_cast<uint16_t>(
        stored.version + stored.occupied + stored.mode +
        static_cast<uint16_t>(stored.targetTempC) +
        static_cast<uint16_t>(stored.lowTempC) +
        static_cast<uint16_t>(stored.highTempC) + stored.durationMinutes +
        stored.highPhaseMinutes + stored.lowPhaseMinutes);
  }

  static bool isStoredSlotValid(const StoredSlot& stored) {
    if (stored.version != STORAGE_VERSION) {
      return false;
    }
    if (stored.checksum != checksum(stored)) {
      return false;
    }
    if (stored.occupied == 0U) {
      return true;
    }
    return ProfileEngine::isValid(toProfile(stored));
  }

  uint16_t recordAddress(uint8_t slotIndex) const {
    return static_cast<uint16_t>(BASE_ADDRESS +
                                 slotIndex * static_cast<uint16_t>(recordSize()));
  }

  void readRecord(uint8_t slotIndex, StoredSlot& stored) const {
    const uint16_t address = recordAddress(slotIndex);
    stored.version = readU8(address + 0U);
    stored.occupied = readU8(address + 1U);
    stored.mode = readU8(address + 2U);
    stored.targetTempC = readI16(address + 3U);
    stored.lowTempC = readI16(address + 5U);
    stored.highTempC = readI16(address + 7U);
    stored.durationMinutes = readU16(address + 9U);
    stored.highPhaseMinutes = readU16(address + 11U);
    stored.lowPhaseMinutes = readU16(address + 13U);
    stored.checksum = readU16(address + 15U);
  }

  void writeRecord(uint8_t slotIndex, const StoredSlot& stored) {
    const uint16_t address = recordAddress(slotIndex);
    writeU8(address + 0U, stored.version);
    writeU8(address + 1U, stored.occupied);
    writeU8(address + 2U, stored.mode);
    writeI16(address + 3U, stored.targetTempC);
    writeI16(address + 5U, stored.lowTempC);
    writeI16(address + 7U, stored.highTempC);
    writeU16(address + 9U, stored.durationMinutes);
    writeU16(address + 11U, stored.highPhaseMinutes);
    writeU16(address + 13U, stored.lowPhaseMinutes);
    writeU16(address + 15U, stored.checksum);
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

  void writeU8(uint16_t address, uint8_t value) { storage_.update(address, value); }

  void writeI16(uint16_t address, int16_t value) {
    writeU16(address, static_cast<uint16_t>(value));
  }

  void writeU16(uint16_t address, uint16_t value) {
    storage_.update(address, static_cast<uint8_t>(value & 0xFFU));
    storage_.update(static_cast<uint16_t>(address + 1U),
                    static_cast<uint8_t>((value >> 8U) & 0xFFU));
  }

  ByteStorage& storage_;
};

}  // namespace dehydrator
