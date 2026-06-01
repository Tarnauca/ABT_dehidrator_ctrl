#include <unity.h>

#include "dehydrator/app/PeriodicTask.h"

using dehydrator::PeriodicTask;

void test_periodic_task_runs_immediately_first_time() {
  PeriodicTask task(1000);

  TEST_ASSERT_TRUE(task.shouldRun(42));
}

void test_periodic_task_waits_until_interval_elapsed() {
  PeriodicTask task(1000);

  TEST_ASSERT_TRUE(task.shouldRun(0));
  TEST_ASSERT_FALSE(task.shouldRun(999));
  TEST_ASSERT_TRUE(task.shouldRun(1000));
}

void test_periodic_task_uses_new_baseline_after_run() {
  PeriodicTask task(1000);

  TEST_ASSERT_TRUE(task.shouldRun(100));
  TEST_ASSERT_TRUE(task.shouldRun(1100));
  TEST_ASSERT_FALSE(task.shouldRun(1999));
  TEST_ASSERT_TRUE(task.shouldRun(2100));
}

void test_periodic_task_can_reset_baseline() {
  PeriodicTask task(1000);

  task.reset(5000);

  TEST_ASSERT_FALSE(task.shouldRun(5999));
  TEST_ASSERT_TRUE(task.shouldRun(6000));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_periodic_task_runs_immediately_first_time);
  RUN_TEST(test_periodic_task_waits_until_interval_elapsed);
  RUN_TEST(test_periodic_task_uses_new_baseline_after_run);
  RUN_TEST(test_periodic_task_can_reset_baseline);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
