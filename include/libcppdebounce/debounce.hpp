#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace cppdebounce {

/**
 * @brief Debounce
 * Execution is deferred until calls stop,
 * repeated calls reset the delay.
 */
class Debounce {
 public:
  using DebounceCallback = std::function<void()>;

  struct DebounceOperation {
    std::mutex mtx;
    std::condition_variable cv;
    bool is_cancelled = false;
    DebounceCallback callback;

    explicit DebounceOperation(DebounceCallback cb) : callback(std::move(cb)) {}
  };

  /**
   * @brief Debounces execution.
   * If called again with the same tag before duration expires, the previous
   * call is cancelled and the timer restarts. The callback runs at the END of
   * the duration.
   * * @param tag Unique identifier.
   * @param duration Time to wait before executing.
   * @param on_execute The function to execute after the silence period.
   */
  static void debounce(const std::string& tag,
                       std::chrono::milliseconds duration,
                       const DebounceCallback& on_execute) {
    std::scoped_lock lock(_global_mutex);

    auto it = _operations.find(tag);
    if (it != _operations.end()) {
      auto old_op = it->second;
      {
        std::scoped_lock op_lock(old_op->mtx);
        old_op->is_cancelled = true;
      }
      old_op->cv.notify_one();
    }

    auto op_context = std::make_shared<DebounceOperation>(on_execute);

    _operations[tag] = op_context;

    std::thread([tag, duration, op_context]() {
      std::unique_lock<std::mutex> op_lock(op_context->mtx);

      bool cancelled = op_context->cv.wait_for(
          op_lock, duration,
          [&op_context] { return op_context->is_cancelled; });

      if (cancelled) {
        return;
      }

      op_lock.unlock();

      {
        std::unique_lock<std::mutex> global_lock(_global_mutex);

        auto it = _operations.find(tag);
        if (it != _operations.end() && it->second == op_context) {
          _operations.erase(it);

          if (op_context->callback) {
            global_lock.unlock();
            op_context->callback();
          }
        }
      }
    }).detach();
  }

  /**
   * @brief Cancels any pending debounce operation for the tag.
   * The callback will never run.
   */
  static void cancel(const std::string& tag) {
    std::scoped_lock lock(_global_mutex);

    auto it = _operations.find(tag);
    if (it != _operations.end()) {
      auto op_context = it->second;
      _operations.erase(it);

      // Signal thread to exit
      {
        std::scoped_lock op_lock(op_context->mtx);
        op_context->is_cancelled = true;
      }
      op_context->cv.notify_one();
    }
  }

  static auto is_pending(const std::string& tag) -> bool {
    std::scoped_lock lock(_global_mutex);
    return _operations.find(tag) != _operations.end();
  }

  /**
   * @brief Cancels every pending operation.
   * Test-only helper for isolating test cases; not part of the supported API.
   */
  static void reset_for_testing() {
    std::scoped_lock lock(_global_mutex);
    for (auto& [tag, op] : _operations) {
      {
        std::scoped_lock op_lock(op->mtx);
        op->is_cancelled = true;
      }
      op->cv.notify_one();
    }
    _operations.clear();
  }

 private:
  static std::unordered_map<std::string, std::shared_ptr<DebounceOperation>>
      _operations;
  static std::mutex _global_mutex;
};

inline std::unordered_map<std::string,
                          std::shared_ptr<Debounce::DebounceOperation>>
    Debounce::_operations;
inline std::mutex Debounce::_global_mutex;

}  // namespace cppdebounce
