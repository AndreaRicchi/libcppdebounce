#include <iostream>

#include "libcppdebounce/throttle.hpp"

// NOLINTBEGIN(readability-magic-numbers)

auto main() -> int {
  auto on_execute = []() { std::cout << "Action Executed!\n"; };
  auto on_after = []() { std::cout << "Throttle finished (on_after).\n"; };

  std::cout << "--- 1. Normal Flow ---\n";
  Throttle::throttle("tag1", std::chrono::milliseconds(2000), on_execute,
                     on_after);

  Throttle::throttle("tag1", std::chrono::milliseconds(2000), on_execute,
                     on_after);

  Throttle::throttle("tag1", std::chrono::milliseconds(2000), on_execute,
                     on_after);

  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  std::cout << "\n--- 2. Cancel Flow ---\n";
  Throttle::throttle("tag1", std::chrono::milliseconds(3000), on_execute,
                     on_after);

  std::cout << "Wait 1 second...\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::cout << "Cancelling tag1...\n";
  Throttle::cancel("tag1");

  bool throttled =
      Throttle::throttle("tag1", std::chrono::milliseconds(100),
                         [] { std::cout << "Immediate New Action!\n"; });
  if (!throttled) {
    std::cout
        << "Success: Was able to run new action immediately after cancel.\n";
  }

  // Keep main alive briefly to prove on_after from the *cancelled* action never
  // runs
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  return 0;
}

// NOLINTEND(readability-magic-numbers)
