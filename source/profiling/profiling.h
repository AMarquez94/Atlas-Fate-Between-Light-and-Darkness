#ifndef INC_PROFILING_H_
#define INC_PROFILING_H_

#define PROFILING_ENABLED

#include <cinttypes>
#include <thread>   
#include <atomic>   

#ifdef PROFILING_ENABLED

class CProfiler {

  struct TEntry {
    uint64_t     time_stamp;
    const char*  name;
    uint32_t     thread_id;
    bool isBegin() const;
  };

  TEntry*  entries;
  std::atomic<uint32_t> nentries;
  uint32_t max_entries;
  bool     is_capturing;
  uint32_t nframes_to_capture;

  void create(uint32_t amax_entries) {
    max_entries = amax_entries;
    nentries = 0;
    entries = new TEntry[max_entries];
  }

  void destroy() {
    delete[] entries;
    max_entries = 0;
  }

  void saveResults();

public:

  CProfiler() 
    : entries( nullptr )
    , nentries(0)
    , max_entries(0)
  { }

  __forceinline uint32_t enter(const char* name) {
    uint32_t my_entry = nentries++;
    assert(my_entry < max_entries);
    auto* e = entries + my_entry;
    e->time_stamp = __rdtsc() & (~1ULL);
    e->name = name;
    e->thread_id = (uint32_t)::GetCurrentThreadId(); //std::this_thread::get_id().hash();
    return my_entry;
  }
  __forceinline void exit(uint32_t entrance_idx) {
    uint32_t my_entry = nentries++;
    assert(my_entry < max_entries);
    auto* e = entries + my_entry;
    e->time_stamp = __rdtsc() | (1ULL);
    e->name = entries[entrance_idx].name;
    e->thread_id = (uint32_t)::GetCurrentThreadId(); //std::this_thread::get_id().hash();
  }

  void setNFramesToCapture(uint32_t new_n);
  void beginFrame();

};

extern CProfiler profiler;

// ------------------------------------
struct TCPUScoped {
  uint32_t n;
  TCPUScoped(const char* txt) {
    n = profiler.enter(txt);
  }
  ~TCPUScoped() {
    profiler.exit(n);
  }
};

#define PROFILE_FUNCTION(txt)   TCPUScoped profiled_scoped(txt)
#define PROFILE_FRAME_BEGINS()  profiler.beginFrame()
#define PROFILE_SET_NFRAMES(n)   profiler.setNFramesToCapture(n)

#else

#define PROFILE_FUNCTION(txt)  
#define PROFILE_FRAME_BEGINS()   
#define PROFILE_SET_NFRAMES(n)

#endif




#endif


