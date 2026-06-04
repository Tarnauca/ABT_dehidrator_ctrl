#include <unity.h>

#include "dehydrator/app/EncoderStepFilter.h"

using dehydrator::EncoderStepFilter;

void test_requires_full_positive_detent_before_emitting_step() {
  EncoderStepFilter filter(4);

  TEST_ASSERT_EQUAL_INT8(0, filter.update(0));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(1));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(2));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(3));
  TEST_ASSERT_EQUAL_INT8(1, filter.update(4));
}

void test_requires_full_negative_detent_before_emitting_step() {
  EncoderStepFilter filter(4);

  TEST_ASSERT_EQUAL_INT8(0, filter.update(0));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(-1));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(-2));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(-3));
  TEST_ASSERT_EQUAL_INT8(-1, filter.update(-4));
}

void test_small_reverse_bounce_is_ignored_until_direction_is_stable() {
  EncoderStepFilter filter(4);

  TEST_ASSERT_EQUAL_INT8(0, filter.update(0));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(1));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(2));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(1));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(2));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(3));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(4));
  TEST_ASSERT_EQUAL_INT8(1, filter.update(5));
}

void test_reset_clears_pending_partial_step() {
  EncoderStepFilter filter(4);

  TEST_ASSERT_EQUAL_INT8(0, filter.update(0));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(1));
  filter.reset(10);
  TEST_ASSERT_EQUAL_INT8(0, filter.update(11));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(12));
  TEST_ASSERT_EQUAL_INT8(0, filter.update(13));
  TEST_ASSERT_EQUAL_INT8(1, filter.update(14));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_requires_full_positive_detent_before_emitting_step);
  RUN_TEST(test_requires_full_negative_detent_before_emitting_step);
  RUN_TEST(test_small_reverse_bounce_is_ignored_until_direction_is_stable);
  RUN_TEST(test_reset_clears_pending_partial_step);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
