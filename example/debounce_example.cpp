#include <iostream>

#include "libcppdebounce/debounce.hpp"

// NOLINTBEGIN(readability-magic-numbers)

void onSearchInput(const std::string& text) {
  Debounce::debounce("search_api", std::chrono::milliseconds(500), [text]() {
    std::cout << "Searching API for: " << text << "\n";
  });
}

auto main() -> int {
  onSearchInput("Hello one");
  onSearchInput("Hello two");
  onSearchInput("Hello three");
  onSearchInput("Hello four");

  // Keep main alive briefly to prove on_after from the *cancelled* action never
  // runs
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  return 0;
}

// NOLINTEND(readability-magic-numbers)
