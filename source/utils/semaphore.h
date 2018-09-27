#pragma once

#include <condition_variable>
#include <mutex>

class Semaphore {
public:
  Semaphore(int count_ = 0)
    : count(count_) {}

  inline void notify()
  {
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    cv.notify_one();
  }

  inline void wait()
  {
    std::unique_lock<std::mutex> lock(mtx);

    while (count == 0) {
      cv.wait(lock);
    }
    count--;
  }

  bool try_wait() {
    std::lock_guard<decltype(mtx)> lock(mtx);
    if (count) {
      --count;
      return true;
    }
    return false;
  }

private:
  std::mutex mtx;
  std::condition_variable cv;
  int count;
}; 

