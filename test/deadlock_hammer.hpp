#pragma once

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <thread>
#include <vector>

constexpr auto K_HAMMER_RUN_FOR = std::chrono::seconds(3);
constexpr auto K_HAMMER_STALL_LIMIT = std::chrono::seconds(1);
constexpr auto K_HAMMER_POLL = std::chrono::milliseconds(50);
constexpr int K_HAMMER_THREADS = 4;

inline void hammer_without_deadlock(
    const std::function<void()>& body,
    std::chrono::milliseconds run_for = K_HAMMER_RUN_FOR,
    std::chrono::milliseconds stall_limit = K_HAMMER_STALL_LIMIT,
    int thread_count = K_HAMMER_THREADS) {
  std::atomic<bool> stop{false};
  std::atomic<std::uint64_t> iterations{0};
  std::vector<std::thread> hammers;

  hammers.reserve(thread_count);
  for (int t = 0; t < thread_count; ++t) {
    hammers.emplace_back([&]() -> void {
      while (!stop) {
        body();
        iterations++;
      }
    });
  }

  const auto deadline = std::chrono::steady_clock::now() + run_for;
  auto last_progress = std::chrono::steady_clock::now();
  std::uint64_t last_seen = iterations;

  while (std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(K_HAMMER_POLL);
    const std::uint64_t now_seen = iterations;
    if (now_seen != last_seen) {
      last_seen = now_seen;
      last_progress = std::chrono::steady_clock::now();
    } else if (std::chrono::steady_clock::now() - last_progress > stall_limit) {
      ADD_FAILURE() << "Deadlock: no hammer thread made progress for "
                    << stall_limit.count() << "ms after " << now_seen
                    << " iterations";
      std::_Exit(EXIT_FAILURE);
    }
  }

  stop = true;
  for (auto& h : hammers) {
    h.join();
  }
}
