#include "mcv_platform.h"
#include "profiling.h"

// use chrome with url: chrome://tracing/

CProfiler profiler;

bool CProfiler::TEntry::isBegin() const {
  return (time_stamp & 1ULL) == 0;
}

void CProfiler::setNFramesToCapture(uint32_t new_n) {
  nframes_to_capture = new_n;
  is_capturing = false;
}

void CProfiler::beginFrame() {

  if (max_entries == 0)
    create(1 << 16);

  if (!is_capturing) {
    if (nframes_to_capture) {
      is_capturing = true;
    }
    else {
      // el profiler no esta activo
    }
    nentries = 0;
  }
  else {
    // Estamos acabando el último frame de los 
    // que queria calcular
    if (nframes_to_capture == 1) {
      is_capturing = false;
      saveResults();
      nentries = 0;
    }
    --nframes_to_capture;
  }
}

void CProfiler::saveResults() {
  const char* ofilename = "aresults.cpuprofile";
  FILE* f = fopen(ofilename, "wb");
  if (!f)
    return;

  // Query the freq to convert cycles to seconds
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);

  // The minimum time of all records
  uint64_t t0 = entries[0].time_stamp;
  
  fprintf(f, "{\"traceEvents\": [");
  
  uint32_t pid = 1234;    // Invented process id
  const char* thread_name = "main_thread";
  int n = 0;
  for (uint32_t i = 0; i < nentries; ++i, ++n) {
    const TEntry* e = entries + i;
    if (n)
      fprintf(f, ",");
    uint32_t thread_id = e->thread_id;

    // Convert cycles to s
    assert(e->time_stamp >= t0);
    uint64_t ts = e->time_stamp - t0;
    ts *= 1000000;
    ts /= freq.QuadPart;

    if (e->isBegin()) {
      fprintf(f, "{\"name\":\"%s\", \"cat\":\"c++\"", e->name);
      fprintf(f, ",\"ph\":\"B\",\"ts\": %lld, \"pid\":%d, \"tid\" : %d }\n", ts, pid, thread_id);
    }
    else {
      fprintf(f, "{\"ph\":\"E\",\"ts\": %lld, \"pid\":%d, \"tid\" : %d }\n", ts, pid, thread_id);
    }
  }
  if (nentries) {
    fprintf(f, ",{\"name\": \"thread_name\", \"ph\": \"M\", \"pid\": %d, \"tid\": %d, \"args\": {\"name\":\"%s\" }}\n"
      , pid, 24, thread_name);
  }
  fprintf(f, "]}");

  fclose(f);
}


