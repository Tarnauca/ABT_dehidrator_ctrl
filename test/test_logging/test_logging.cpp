#include <string.h>
#include <unity.h>

#include "dehydrator/logging/LogDispatcher.h"
#include "dehydrator/logging/LogFormatter.h"
#include "dehydrator/logging/LogSink.h"

using dehydrator::LogDispatcher;
using dehydrator::LogFormatter;
using dehydrator::LogSink;

class FakeLogSink final : public LogSink {
 public:
  void writeLine(const char* line) override {
    ++writeCount;
    strncpy(lastLine, line, sizeof(lastLine) - 1U);
    lastLine[sizeof(lastLine) - 1U] = '\0';
  }

  unsigned int writeCount = 0;
  char lastLine[96] = {};
};

void test_format_event_creates_parseable_line() {
  char line[64] = {};

  const bool ok = LogFormatter::formatEvent(line, sizeof(line), "boot",
                                            "scheduler_shell");

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_STRING("EVENT type=boot detail=scheduler_shell", line);
}

void test_format_scheduler_state_creates_parseable_line() {
  char line[160] = {};

  const bool ok = LogFormatter::formatSchedulerState(line, sizeof(line), 5000U,
                                                     true);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_STRING("STATE app=scheduler_shell uptime_ms=5000 led=on",
                           line);
}

void test_format_bringup_state_includes_valid_ntc_fields() {
  char line[160] = {};

  const bool ok = LogFormatter::formatBringupState(line, sizeof(line), 5000U,
                                                   true, true, 57, 512U, true,
                                                   24, 43U, "running", "mere",
                                                   false, true);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_STRING(
      "STATE app=bringup uptime_ms=5000 led=on run=running preset=mere h=off f=on ntc=57 adc=512 env_t=24 rh=43",
      line);
}

void test_format_bringup_state_marks_invalid_ntc_as_null() {
  char line[160] = {};

  const bool ok = LogFormatter::formatBringupState(line, sizeof(line), 5000U,
                                                   false, false, 0, 0U, false,
                                                   0, 0U, "idle", nullptr,
                                                   false, false);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_STRING(
      "STATE app=bringup uptime_ms=5000 led=off run=idle preset=null h=off f=off ntc=null adc=0 env_t=null rh=null",
      line);
}

void test_format_bringup_state_marks_invalid_temp_rh_as_null() {
  char line[160] = {};

  const bool ok = LogFormatter::formatBringupState(line, sizeof(line), 5000U,
                                                   true, true, 57, 512U, false,
                                                   0, 0U, "finish_cooldown",
                                                   "ierburi", false, true);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_STRING(
      "STATE app=bringup uptime_ms=5000 led=on run=finish_cooldown preset=ierburi h=off f=on ntc=57 adc=512 env_t=null rh=null",
      line);
}

void test_formatter_reports_truncated_line() {
  char line[8] = {};

  const bool ok = LogFormatter::formatEvent(line, sizeof(line), "very_long",
                                            "detail");

  TEST_ASSERT_FALSE(ok);
}

void test_formatter_uses_stable_null_token() {
  char line[64] = {};

  const bool ok = LogFormatter::formatEvent(line, sizeof(line), nullptr,
                                            nullptr);

  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_STRING("EVENT type=null detail=null", line);
}

void test_dispatcher_fans_out_to_all_sinks() {
  FakeLogSink first;
  FakeLogSink second;
  LogSink* sinks[2] = {};
  LogDispatcher dispatcher(sinks, 2);

  TEST_ASSERT_TRUE(dispatcher.addSink(first));
  TEST_ASSERT_TRUE(dispatcher.addSink(second));

  dispatcher.writeLine("EVENT type=test detail=fanout");

  TEST_ASSERT_EQUAL_UINT(1U, first.writeCount);
  TEST_ASSERT_EQUAL_STRING("EVENT type=test detail=fanout", first.lastLine);
  TEST_ASSERT_EQUAL_UINT(1U, second.writeCount);
  TEST_ASSERT_EQUAL_STRING("EVENT type=test detail=fanout", second.lastLine);
}

void test_dispatcher_ignores_null_lines() {
  FakeLogSink sink;
  LogSink* sinks[1] = {};
  LogDispatcher dispatcher(sinks, 1);

  TEST_ASSERT_TRUE(dispatcher.addSink(sink));
  dispatcher.writeLine(nullptr);

  TEST_ASSERT_EQUAL_UINT(0U, sink.writeCount);
}

void test_dispatcher_rejects_sinks_when_full() {
  FakeLogSink first;
  FakeLogSink second;
  LogSink* sinks[1] = {};
  LogDispatcher dispatcher(sinks, 1);

  TEST_ASSERT_TRUE(dispatcher.addSink(first));
  TEST_ASSERT_FALSE(dispatcher.addSink(second));
  TEST_ASSERT_EQUAL_UINT(1U, dispatcher.sinkCount());
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_format_event_creates_parseable_line);
  RUN_TEST(test_format_scheduler_state_creates_parseable_line);
  RUN_TEST(test_format_bringup_state_includes_valid_ntc_fields);
  RUN_TEST(test_format_bringup_state_marks_invalid_ntc_as_null);
  RUN_TEST(test_format_bringup_state_marks_invalid_temp_rh_as_null);
  RUN_TEST(test_formatter_reports_truncated_line);
  RUN_TEST(test_formatter_uses_stable_null_token);
  RUN_TEST(test_dispatcher_fans_out_to_all_sinks);
  RUN_TEST(test_dispatcher_ignores_null_lines);
  RUN_TEST(test_dispatcher_rejects_sinks_when_full);
  UNITY_END();
}

void loop() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  setup();
  return 0;
}
