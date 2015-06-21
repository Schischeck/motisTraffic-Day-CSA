#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace motis {

/// this class provides synchronization and distinguishes two kinds of access:
///
///  - write (changing data)
///  - read (reading without changing)
///
///
/// synchronization properties:
///
///  - there may be an arbitrary number of parallel readers as long as there is
///    no write access.
///  - write access is exclusive. there cannot be any reads or other writes
///    at the same time.
///  - write accesses are prioritized. this means that no new read
///    access will be granted as soon as there is at least one write access
///    waiting.
///  - write accesses are queued in their request order.
///  - active read accesses cannot be stopped. thus, queued write requests
///    need to wait for all read accesses to finish.
struct synchronization {
  struct lock {
    lock(lock const&) = delete;
    lock& operator=(lock&) = delete;

    lock(synchronization& s, bool write) : s_(s), write_(write) {
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

    ~lock() {
      std::lock_guard<std::mutex> lock(s_.write_queue_mutex_);

      {
        if (write_) {
          s_.write_queue_.pop();
        }

        --s_.usage_count_;
      }

      s_.write_queue_cv_.notify_all();
    }

    synchronization& s_;
    bool write_;
  };

  synchronization(synchronization const&) = delete;
  synchronization& operator=(synchronization&) = delete;

  synchronization() : usage_count_(0), next_id_(0) {}

  std::atomic<unsigned> usage_count_;
  std::atomic<unsigned> next_id_;
  std::queue<unsigned> write_queue_;
  std::mutex write_queue_mutex_;
  std::condition_variable write_queue_cv_;
};

}  // namespace motis
