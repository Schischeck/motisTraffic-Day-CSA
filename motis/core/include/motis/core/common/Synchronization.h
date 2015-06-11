#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace td {

/// This class provides synchronization and distinguishes two kinds of access:
///
///  - Write (changing data)
///  - Read (reading without changing)
///
///
/// Synchronization Properties:
///
///  - There may be an arbitrary number of parallel readers as long as there is
///    no write access.
///  - Write access is exclusive. There cannot be any reads or other writes
///    at the same time.
///  - Write accesses are prioritized. This means that no new read
///    access will be granted as soon as there is at least one write access
///    waiting.
///  - Write accesses are queued in their request order.
///  - Active read accesses cannot be stopped. Thus, queued write requests
///    need to wait for all read accesses to finish.
struct Synchronization {
  struct Lock {
    Lock(Lock const&) = delete;
    Lock& operator=(Lock&) = delete;

    Lock(Synchronization& s, bool write) : s_(s), write_(write) {
      std::unique_lock<std::mutex> lock(s_.write_queue_mutex_);

      if (write_) {
        auto my_id = ++s_.next_id_;
        s_.write_queue_.push(my_id);
        s_.write_queue_cv_.wait(lock, [this, my_id] {
          return s_.usage_count_ == 0 && s_.write_queue_.front() == my_id;
        });
      } else {
        s_.write_queue_cv_.wait(lock,
                                [this] { return s_.write_queue_.empty(); });
      }

      ++s_.usage_count_;
    }

    ~Lock() {
      std::lock_guard<std::mutex> lock(s_.write_queue_mutex_);

      {
        if (write_) {
          s_.write_queue_.pop();
        }

        --s_.usage_count_;
      }

      s_.write_queue_cv_.notify_all();
    }

    Synchronization& s_;
    bool write_;
  };

  Synchronization(Synchronization const&) = delete;
  Synchronization& operator=(Synchronization&) = delete;

  Synchronization() : usage_count_(0), next_id_(0) {}

  std::atomic<unsigned> usage_count_;
  std::atomic<unsigned> next_id_;
  std::queue<unsigned> write_queue_;
  std::mutex write_queue_mutex_;
  std::condition_variable write_queue_cv_;
};

}  // namespace td