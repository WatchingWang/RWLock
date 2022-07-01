#include <condition_variable>
#include <mutex>

template <typename T = std::mutex>
class RWLock {
 public:
  enum Option { READ_FIRST, WRITE_FIRST };

  void SetOption(Option option) {
    std::lock_guard<decltype(lck_)> lk(lck_);
    option_ = option;
  }

  int AcquireRLck() {
    {
      std::unique_lock<decltype(lck_)> lk(lck_);
      if (w_count_ != 0) {
        cv_.wait(lk, [=] {
          return (w_count_ == 0) &&
                 !(option_ == WRITE_FIRST && is_acquiring_w_);
        })
      }
    }

    ++r_count_;
    return 0;
  }

  int ReleaseRLck() {
    std::lock_guard<decltype(lck_)> lk(lck_);
    --r_count_;
    cv_.notify_all();
    return 0;
  }

  int AcquireWLck() {
    {
      std::unique_lock<decltype(lck_)> lk(lck_);
      is_acquiring_w_ = true;
      if (w_count_ != 0 || r_count_ != 0) {
        cv_.wait(lk, [=] { return w_count_ == 0 && r_count_ == 0; })
      }
    }

    ++w_count_;
  }

  int ReleaseWLck() {
    std::lock_guard<decltype(lck_)> lk(lck_);
    --w_count_;
    is_acquiring_w_ = false;
    cv_.notify_all();
    return 0;
  }

 private:
  T lck_;
  std::condition_variable cv_;

  Option option_;
  bool is_acquiring_w_ = false;

  size_t r_count_ = 0;
  size_t w_count_ = 0;
};