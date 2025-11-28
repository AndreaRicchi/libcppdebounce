#include "libcppdebounce/throttle.hpp"

#include <gtest/gtest.h>

using namespace std::chrono_literals;

class ThrottleTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Clear state before every test
    Throttle::reset_for_testing();
  }
};

TEST_F(ThrottleTest, ReturnsFalseOnFirstCall) {
  bool throttled = Throttle::throttle("tag1", 1000ms, [] {}, nullptr);
  EXPECT_FALSE(throttled) << "First call should not be throttled";
}

TEST_F(ThrottleTest, ReturnsTrueOnSecondCall) {
  Throttle::throttle("tag1", 1000ms, [] {}, nullptr);
  bool throttled = Throttle::throttle("tag1", 1000ms, [] {}, nullptr);
  EXPECT_TRUE(throttled) << "Second call within duration should be throttled";
}

TEST_F(ThrottleTest, ExecutesOnExecuteImmediately) {
  bool executed = false;
  Throttle::throttle("tag_exec", 1000ms, [&] { executed = true; }, nullptr);
  EXPECT_TRUE(executed) << "onExecute should run synchronously";
}

TEST_F(ThrottleTest, UnthrottlesAfterDuration) {
  Throttle::throttle("tag_time", 100ms, [] {}, nullptr);

  EXPECT_TRUE(Throttle::throttle("tag_time", 100ms, [] {}, nullptr));

  std::this_thread::sleep_for(150ms);

  bool throttled = Throttle::throttle("tag_time", 100ms, [] {}, nullptr);
  EXPECT_FALSE(throttled) << "Should be unthrottled after duration expires";
}

TEST_F(ThrottleTest, RunsOnAfterCallback) {
  std::atomic<bool> after_called{false};

  Throttle::throttle("tag_after", 100ms, [] {}, [&] { after_called = true; });

  EXPECT_FALSE(after_called);

  std::this_thread::sleep_for(150ms);

  EXPECT_TRUE(after_called) << "onAfter should be called after timeout";
}

TEST_F(ThrottleTest, CancelFreesTagImmediately) {
  Throttle::throttle("tag_cancel", 5000ms, [] {}, nullptr);

  EXPECT_TRUE(Throttle::throttle("tag_cancel", 5000ms, [] {}, nullptr));

  Throttle::cancel("tag_cancel");

  bool throttled = Throttle::throttle("tag_cancel", 100ms, [] {}, nullptr);
  EXPECT_FALSE(throttled) << "Cancel should free the tag immediately";
}

TEST_F(ThrottleTest, CancelDoesNotRunOnAfter) {
  std::atomic<bool> after_called{false};

  Throttle::throttle(
      "tag_no_after", 500ms, [] {}, [&] { after_called = true; });

  Throttle::cancel("tag_no_after");

  // Wait longer than duration to ensure thread woke up and finished
  std::this_thread::sleep_for(600ms);

  EXPECT_FALSE(after_called) << "onAfter should NOT run if cancelled manually";
}

TEST_F(ThrottleTest, DifferentTagsDoNotBlockEachOther) {
  Throttle::throttle("TAG_A", 1000ms, [] {}, nullptr);

  bool throttled_b = Throttle::throttle("TAG_B", 1000ms, [] {}, nullptr);

  EXPECT_FALSE(throttled_b) << "Tag B should run even if Tag A is active";
}

TEST_F(ThrottleTest, ThreadSafetyStressTest) {
  // Spawn 10 threads trying to hit the same tag
  // Only ONE should succeed (return false), 9 should fail (return true)

  std::atomic<int> success_count{0};
  std::vector<std::thread> threads;
  const auto spawn_count = 10;

  threads.reserve(spawn_count);
  for (int i = 0; i < spawn_count; ++i) {
    threads.emplace_back([&] {
      if (!Throttle::throttle("stress_tag", 1000ms, [] {}, nullptr)) {
        success_count++;
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  EXPECT_EQ(success_count, 1)
      << "Exactly one thread should have succeeded in acquiring the lock";
}
