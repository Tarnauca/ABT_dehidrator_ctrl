#include <unity.h>

#include "dehydrator/interfaces/ByteStorage.h"
#include "dehydrator/persistence/UserProfileStore.h"

using dehydrator::ByteStorage;
using dehydrator::ProfileConfig;
using dehydrator::ProfileMode;
using dehydrator::UserProfileSlotRecord;
using dehydrator::UserProfileStore;

class FakeByteStorage : public ByteStorage {
 public:
  static constexpr uint16_t SIZE = 512U;
  uint8_t bytes[SIZE] = {};

  FakeByteStorage() {
    for (uint16_t index = 0U; index < SIZE; index++) {
      bytes[index] = 0xFFU;
    }
  }

  uint16_t length() const override { return SIZE; }

  uint8_t read(uint16_t address) const override { return bytes[address]; }

  void update(uint16_t address, uint8_t value) override { bytes[address] = value; }
};

void test_save_and_load_round_trip_profile() {
  FakeByteStorage storage;
  UserProfileStore store(storage);
  const ProfileConfig profile{ProfileMode::Boost, 55, 0, 65, 360, 30, 0};

  TEST_ASSERT_TRUE(store.save(2U, profile));

  UserProfileSlotRecord loaded;
  TEST_ASSERT_TRUE(store.load(2U, loaded));
  TEST_ASSERT_TRUE(loaded.occupied);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ProfileMode::Boost),
                        static_cast<int>(loaded.profile.mode));
  TEST_ASSERT_EQUAL_INT(55, loaded.profile.targetTempC);
  TEST_ASSERT_EQUAL_INT(65, loaded.profile.highTempC);
}

void test_clear_makes_slot_vacant() {
  FakeByteStorage storage;
  UserProfileStore store(storage);

  TEST_ASSERT_TRUE(store.save(0U, {ProfileMode::Fixed, 57, 0, 0, 480, 0, 0}));
  TEST_ASSERT_TRUE(store.clear(0U));

  UserProfileSlotRecord loaded;
  TEST_ASSERT_TRUE(store.load(0U, loaded));
  TEST_ASSERT_FALSE(loaded.occupied);
}

void test_corrupt_checksum_is_treated_as_vacant() {
  FakeByteStorage storage;
  UserProfileStore store(storage);

  TEST_ASSERT_TRUE(store.save(1U, {ProfileMode::Fixed, 57, 0, 0, 480, 0, 0}));
  storage.bytes[UserProfileStore::BASE_ADDRESS + 1U * 17U + 3U] ^= 0x01U;

  UserProfileSlotRecord loaded;
  TEST_ASSERT_TRUE(store.load(1U, loaded));
  TEST_ASSERT_FALSE(loaded.occupied);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_save_and_load_round_trip_profile);
  RUN_TEST(test_clear_makes_slot_vacant);
  RUN_TEST(test_corrupt_checksum_is_treated_as_vacant);
  return UNITY_END();
}
