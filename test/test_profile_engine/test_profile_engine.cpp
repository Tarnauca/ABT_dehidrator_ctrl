#include <unity.h>

#include "dehydrator/domain/ProfileEngine.h"

using dehydrator::ProfileConfig;
using dehydrator::ProfileEngine;
using dehydrator::ProfileMode;
using dehydrator::ProfileTarget;

ProfileConfig fixedProfile() {
  ProfileConfig profile;
  profile.mode = ProfileMode::Fixed;
  profile.targetTempC = 57;
  profile.durationMinutes = 10;
  return profile;
}

ProfileConfig fluctuatingProfile() {
  ProfileConfig profile;
  profile.mode = ProfileMode::Fluctuating;
  profile.targetTempC = 57;
  profile.lowTempC = 50;
  profile.highTempC = 65;
  profile.durationMinutes = 120;
  profile.highPhaseMinutes = 30;
  profile.lowPhaseMinutes = 30;
  return profile;
}

ProfileConfig boostProfile() {
  ProfileConfig profile;
  profile.mode = ProfileMode::Boost;
  profile.targetTempC = 55;
  profile.highTempC = 65;
  profile.durationMinutes = 120;
  profile.highPhaseMinutes = 30;
  return profile;
}

void test_fixed_profile_returns_fixed_target() {
  const ProfileTarget target = ProfileEngine::evaluate(fixedProfile(), 123);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_FALSE(target.complete);
  TEST_ASSERT_EQUAL_INT16(57, target.targetTempC);
}

void test_profile_is_complete_at_configured_duration() {
  const ProfileTarget target = ProfileEngine::evaluate(fixedProfile(), 10UL * 60UL);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_TRUE(target.complete);
  TEST_ASSERT_EQUAL_INT16(57, target.targetTempC);
}

void test_fluctuating_profile_starts_with_high_target() {
  const ProfileTarget target = ProfileEngine::evaluate(fluctuatingProfile(), 0);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_EQUAL_INT16(65, target.targetTempC);
}

void test_fluctuating_profile_switches_to_low_after_high_phase() {
  const ProfileTarget target =
      ProfileEngine::evaluate(fluctuatingProfile(), 30UL * 60UL);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_EQUAL_INT16(50, target.targetTempC);
}

void test_fluctuating_profile_repeats_high_low_cycle() {
  const ProfileTarget secondHigh =
      ProfileEngine::evaluate(fluctuatingProfile(), 60UL * 60UL);
  const ProfileTarget secondLow =
      ProfileEngine::evaluate(fluctuatingProfile(), 90UL * 60UL);

  TEST_ASSERT_EQUAL_INT16(65, secondHigh.targetTempC);
  TEST_ASSERT_EQUAL_INT16(50, secondLow.targetTempC);
}

void test_boost_profile_starts_with_high_target() {
  const ProfileTarget target = ProfileEngine::evaluate(boostProfile(), 0);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_EQUAL_INT16(65, target.targetTempC);
}

void test_boost_profile_continues_with_base_target_after_boost_phase() {
  const ProfileTarget target =
      ProfileEngine::evaluate(boostProfile(), 30UL * 60UL);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_EQUAL_INT16(55, target.targetTempC);
}

void test_boost_profile_is_complete_at_configured_duration() {
  const ProfileTarget target =
      ProfileEngine::evaluate(boostProfile(), 120UL * 60UL);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_TRUE(target.complete);
  TEST_ASSERT_EQUAL_INT16(55, target.targetTempC);
}

void test_rejects_setpoint_above_safety_limit() {
  ProfileConfig profile = fixedProfile();
  profile.targetTempC = 76;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_accepts_setpoint_at_safety_limit() {
  ProfileConfig profile = fixedProfile();
  profile.targetTempC = ProfileEngine::MAX_TARGET_TEMP_C;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_EQUAL_INT16(ProfileEngine::MAX_TARGET_TEMP_C, target.targetTempC);
}

void test_rejects_fluctuating_average_above_safety_limit() {
  ProfileConfig profile = fluctuatingProfile();
  profile.targetTempC = 76;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_rejects_boost_target_above_safety_limit() {
  ProfileConfig profile = boostProfile();
  profile.highTempC = 76;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_rejects_boost_duration_above_half_total_duration() {
  ProfileConfig profile = boostProfile();
  profile.highPhaseMinutes = 61;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_accepts_fluctuating_high_at_safety_limit() {
  ProfileConfig profile = fluctuatingProfile();
  profile.highTempC = ProfileEngine::MAX_TARGET_TEMP_C;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_EQUAL_INT16(ProfileEngine::MAX_TARGET_TEMP_C, target.targetTempC);
}

void test_rejects_fluctuating_high_above_safety_limit() {
  ProfileConfig profile = fluctuatingProfile();
  profile.highTempC = 76;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_rejects_duration_above_99_hours() {
  ProfileConfig profile = fixedProfile();
  profile.durationMinutes = ProfileEngine::MAX_DURATION_MINUTES + 1U;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_accepts_duration_at_99_hours() {
  ProfileConfig profile = fixedProfile();
  profile.durationMinutes = ProfileEngine::MAX_DURATION_MINUTES;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_TRUE(target.valid);
}

void test_rejects_zero_duration() {
  ProfileConfig profile = fixedProfile();
  profile.durationMinutes = 0;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_rejects_fluctuating_profile_without_phase_durations() {
  ProfileConfig profile = fluctuatingProfile();
  profile.highPhaseMinutes = 0;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_rejects_fluctuating_profile_without_low_phase_duration() {
  ProfileConfig profile = fluctuatingProfile();
  profile.lowPhaseMinutes = 0;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_rejects_fluctuating_profile_when_low_exceeds_high() {
  ProfileConfig profile = fluctuatingProfile();
  profile.lowTempC = 66;
  profile.highTempC = 65;

  const ProfileTarget target = ProfileEngine::evaluate(profile, 0);

  TEST_ASSERT_FALSE(target.valid);
}

void test_fluctuating_profile_is_complete_at_configured_duration() {
  const ProfileTarget target =
      ProfileEngine::evaluate(fluctuatingProfile(), 120UL * 60UL);

  TEST_ASSERT_TRUE(target.valid);
  TEST_ASSERT_TRUE(target.complete);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_fixed_profile_returns_fixed_target);
  RUN_TEST(test_profile_is_complete_at_configured_duration);
  RUN_TEST(test_fluctuating_profile_starts_with_high_target);
  RUN_TEST(test_fluctuating_profile_switches_to_low_after_high_phase);
  RUN_TEST(test_fluctuating_profile_repeats_high_low_cycle);
  RUN_TEST(test_boost_profile_starts_with_high_target);
  RUN_TEST(test_boost_profile_continues_with_base_target_after_boost_phase);
  RUN_TEST(test_boost_profile_is_complete_at_configured_duration);
  RUN_TEST(test_rejects_setpoint_above_safety_limit);
  RUN_TEST(test_accepts_setpoint_at_safety_limit);
  RUN_TEST(test_rejects_fluctuating_average_above_safety_limit);
  RUN_TEST(test_rejects_boost_target_above_safety_limit);
  RUN_TEST(test_rejects_boost_duration_above_half_total_duration);
  RUN_TEST(test_accepts_fluctuating_high_at_safety_limit);
  RUN_TEST(test_rejects_fluctuating_high_above_safety_limit);
  RUN_TEST(test_rejects_duration_above_99_hours);
  RUN_TEST(test_accepts_duration_at_99_hours);
  RUN_TEST(test_rejects_zero_duration);
  RUN_TEST(test_rejects_fluctuating_profile_without_phase_durations);
  RUN_TEST(test_rejects_fluctuating_profile_without_low_phase_duration);
  RUN_TEST(test_rejects_fluctuating_profile_when_low_exceeds_high);
  RUN_TEST(test_fluctuating_profile_is_complete_at_configured_duration);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
