#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

/**
 * @brief Throttle
 * First call is executed immediately,
 * all subsequent calls are ignored for a fixed time interval.
 */
class Throttle {
 public:
  using ThrottleCallback = std::function<void()>;

  struct ThrottleOperation {
    std::mutex mtx;
    std::condition_variable cv;
    bool is_cancelled = false;
    ThrottleCallback on_after;

    explicit ThrottleOperation(ThrottleCallback callback)
        : on_after(std::move(callback)) {}
  };

  /**
   * @brief Throttles execution based on a tag and duration.
   *  @param tag Unique identifier for the operation.
   * @param duration Time to ignore subsequent calls.
   * @param on_execute Function to run immediately.
   * @param on_after (Optional) Function to run after duration expires.
   * @return true if throttled (ignored), false if executed.
   */
  static auto throttle(const std::string& tag,
                       std::chrono::milliseconds duration,
                       const ThrottleCallback& on_execute,
                       const ThrottleCallback& on_after = nullptr) -> bool {
    std::unique_lock<std::mutex> lock(_global_mutex);

    if (_operations.find(tag) != _operations.end()) {
      return true;
    }

    auto op_context = std::make_shared<ThrottleOperation>(on_after);
    _operations[tag] = op_context;

    lock.unlock();

    auto execute_callback = on_execute;
    if (execute_callback) {
      execute_callback();
    }

    lock.lock();

    std::thread([tag, duration, op_context]() {
      std::unique_lock<std::mutex> op_lock(op_context->mtx);

      bool cancelled = op_context->cv.wait_for(
          op_lock, duration,
          [&op_context] { return op_context->is_cancelled; });

      if (cancelled) {
        return;
      }

      {
        std::lock_guard<std::mutex> global_lock(_global_mutex);

        auto it = _operations.find(tag);
        if (it != _operations.end() && it->second == op_context) {
          _operations.erase(it);

        } else {
          return;
        }
      }

      if (op_context->on_after) {
        try {
          op_context->on_after();
        } catch (...) {  // NOLINT(bugprone-empty-catch)
        }
      }
    }).detach();

    return false;
  }

  /**
   * @brief Manually cancels a throttle operation.
   *  @param tag Unique identifier for the operation.
   * The on_after callback will NOT be executed.
   */
  static auto cancel(const std::string& tag) -> void {
    std::lock_guard<std::mutex> lock(_global_mutex);

    auto it = _operations.find(tag);
    if (it != _operations.end()) {
      auto op_context = it->second;

      _operations.erase(it);

      {
        std::lock_guard<std::mutex> op_lock(op_context->mtx);
        op_context->is_cancelled = true;
      }
      op_context->cv.notify_one();
    }
  }

  static auto is_active(const std::string& tag) -> bool {
    std::lock_guard<std::mutex> lock(_global_mutex);
    return _operations.find(tag) != _operations.end();
  }

  // Test helper: Clear all state (Not thread safe, use only between tests)
  static void reset_for_testing() {
    std::lock_guard<std::mutex> lock(_global_mutex);
    for (auto& [tag, op] : _operations) {
      {
        std::lock_guard<std::mutex> op_lock(op->mtx);
        op->is_cancelled = true;
      }
      op->cv.notify_one();
    }
    _operations.clear();
  }

 private:
  static std::unordered_map<std::string, std::shared_ptr<ThrottleOperation>>
      _operations;
  static std::mutex _global_mutex;
};

inline std::unordered_map<std::string,
                          std::shared_ptr<Throttle::ThrottleOperation>>
    Throttle::_operations;
inline std::mutex Throttle::_global_mutex;
