#include "libcppdebounce/debounce.hpp"

#include <gtest/gtest.h>

using namespace std::chrono_literals;

class DebounceTest : public ::testing::Test {
 protected:
  void SetUp() override { Debounce::reset_for_testing(); }

  void TearDown() override { Debounce::reset_for_testing(); }
};

TEST_F(DebounceTest, ExecutesAfterDelay) {
  std::atomic<bool> executed{false};

  Debounce::debounce("basic_tag", 100ms, [&] { executed = true; });

  EXPECT_FALSE(executed);

  std::this_thread::sleep_for(150ms);

  EXPECT_TRUE(executed) << "Callback should execute after the duration expires";
}

TEST_F(DebounceTest, SubsequentCallsResetTimer) {
  std::atomic<int> run_count{0};

  Debounce::debounce("reset_tag", 200ms, [&] { run_count++; });

  std::this_thread::sleep_for(100ms);
  EXPECT_EQ(run_count, 0);

  Debounce::debounce("reset_tag", 200ms, [&] { run_count++; });

  std::this_thread::sleep_for(150ms);
  EXPECT_EQ(run_count, 0) << "First call should have been cancelled, second "
                             "call shouldn't be done yet";

  std::this_thread::sleep_for(100ms);
  EXPECT_EQ(run_count, 1) << "Only the last call should execute";
}

TEST_F(DebounceTest, ManualCancelPreventsExecution) {
  std::atomic<bool> executed{false};

  Debounce::debounce("cancel_tag", 100ms, [&] { executed = true; });

  Debounce::cancel("cancel_tag");

  std::this_thread::sleep_for(150ms);

  EXPECT_FALSE(executed) << "Cancelled operation should never run";
}

TEST_F(DebounceTest, CancelRemovesPendingStatus) {
  Debounce::debounce("status_tag", 500ms, [] {});

  EXPECT_TRUE(Debounce::is_pending("status_tag"));

  Debounce::cancel("status_tag");

  EXPECT_FALSE(Debounce::is_pending("status_tag"));
}

TEST_F(DebounceTest, DifferentTagsRunIndependently) {
  std::atomic<int> count_a{0};
  std::atomic<int> count_b{0};

  Debounce::debounce("TAG_A", 100ms, [&] { count_a++; });

  Debounce::debounce("TAG_B", 300ms, [&] { count_b++; });

  std::this_thread::sleep_for(150ms);
  EXPECT_EQ(count_a, 1);
  EXPECT_EQ(count_b, 0);

  std::this_thread::sleep_for(200ms);
  EXPECT_EQ(count_b, 1);
}

TEST_F(DebounceTest, RapidFireOnlyRunsLast) {
  std::atomic<int> run_count{0};
  constexpr int K_SPAM_COUNT = 100;

  for (int i = 0; i < K_SPAM_COUNT; ++i) {
    Debounce::debounce("spam_tag", 200ms, [&] { run_count++; });
    std::this_thread::sleep_for(1ms);
    EXPECT_EQ(run_count, 0) << "Intermediate calls should not execute";
  }

  std::this_thread::sleep_for(250ms);

  EXPECT_EQ(run_count, 1)
      << "Even after 100 rapid calls, only the final one should execute";
}

auto main(int argc, char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
