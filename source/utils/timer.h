#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <Windows.h>

class CTimer {
  uint64_t time_stamp;    
public:
  CTimer() {
    reset();
  }

  // Ticks!
  uint64_t getTimeStamp() const {
    uint64_t now;
    QueryPerformanceCounter((LARGE_INTEGER*)&now);
    return now;
  }

  float elapsed() {
    uint64_t now = getTimeStamp();
    uint64_t delta_ticks = now - time_stamp;

    LARGE_INTEGER freq;
    if (QueryPerformanceFrequency(&freq)) {
      float delta_secs = (float) (delta_ticks) / (float)freq.QuadPart;
      return delta_secs;
    }
    fatal("QueryPerformanceFrequency returned false!!!\n");
    return 0.f;
  }

  // Reset counter to current timestamp
  void reset() {
    time_stamp = getTimeStamp();
  }

  float elapsedAndReset() {
    float delta = elapsed();
    reset();
    return delta;
  }

};


#endif


