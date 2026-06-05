#include <unity.h>

#include "dehydrator/interfaces/ByteStorage.h"
#include "dehydrator/persistence/NtcCalibrationStore.h"

using dehydrator::ByteStorage;
using dehydrator::NtcCalibrationStore;
using dehydrator::config::CalibrationConfig;

class FakeByteStorage : public ByteStorage {
 public:
  FakeByteStorage() {
    for (uint16_t index = 0U; index < sizeof(bytes); index++) {
      bytes[index] = 0xFFU;
    }
  }

  uint16_t length() const override { return sizeof(bytes); }

  uint8_t read(uint16_t address) const override { return bytes[address]; }

  void update(uint16_t address, uint8_t value) override { bytes[address] = value; }

  uint8_t bytes[256] = {};
};

void test_save_and_load_round_trip_calibration() {
  FakeByteStorage storage;
  NtcCalibrationStore store(storage);
  CalibrationConfig calibration = dehydrator::config::CALIBRATION;

  TEST_ASSERT_TRUE(store.save(130, 1030000));
  TEST_ASSERT_TRUE(store.load(calibration));
  TEST_ASSERT_EQUAL_INT16(130, calibration.ntcOffsetCentiC);
  TEST_ASSERT_EQUAL_INT32(1030000, calibration.ntcScalePpm);
}

void test_invalid_checksum_falls_back_to_defaults() {
  FakeByteStorage storage;
  NtcCalibrationStore store(storage);
  CalibrationConfig calibration = dehydrator::config::CALIBRATION;

  TEST_ASSERT_TRUE(store.save(-70, 980000));
  storage.bytes[NtcCalibrationStore::BASE_ADDRESS + 3U] ^= 0x01U;

  calibration.ntcOffsetCentiC = dehydrator::config::CALIBRATION.ntcOffsetCentiC;
  calibration.ntcScalePpm = dehydrator::config::CALIBRATION.ntcScalePpm;
  TEST_ASSERT_FALSE(store.load(calibration));
  TEST_ASSERT_EQUAL_INT16(dehydrator::config::CALIBRATION.ntcOffsetCentiC,
                          calibration.ntcOffsetCentiC);
  TEST_ASSERT_EQUAL_INT32(dehydrator::config::CALIBRATION.ntcScalePpm,
                          calibration.ntcScalePpm);
}

void test_restore_defaults_persists_default_values() {
  FakeByteStorage storage;
  NtcCalibrationStore store(storage);
  CalibrationConfig calibration = dehydrator::config::CALIBRATION;

  TEST_ASSERT_TRUE(store.save(200, 1100000));
  TEST_ASSERT_TRUE(store.restoreDefaults());
  TEST_ASSERT_TRUE(store.load(calibration));
  TEST_ASSERT_EQUAL_INT16(dehydrator::config::CALIBRATION.ntcOffsetCentiC,
                          calibration.ntcOffsetCentiC);
  TEST_ASSERT_EQUAL_INT32(dehydrator::config::CALIBRATION.ntcScalePpm,
                          calibration.ntcScalePpm);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_save_and_load_round_trip_calibration);
  RUN_TEST(test_invalid_checksum_falls_back_to_defaults);
  RUN_TEST(test_restore_defaults_persists_default_values);
  return UNITY_END();
}
