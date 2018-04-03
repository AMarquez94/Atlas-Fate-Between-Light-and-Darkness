#ifndef INC_RENDER_GPU_TRACES_H_
#define INC_RENDER_GPU_TRACES_H_

#if defined(_DEBUG) || defined(PROFILE)

#include <d3d9.h>         // D3DPERF_*

struct CDbgTrace {
  wchar_t wname[64];
  CDbgTrace(const char *name) {
    setName(name);
  }
  inline void setName(const char *name) {
    mbstowcs(wname, name, 64);
  }
  inline void begin() {
    D3DPERF_BeginEvent(D3DCOLOR_XRGB(255, 0, 255), wname);
  }
  inline void end() {
    D3DPERF_EndEvent();
  }
};

#else

struct CDbgTrace {
  CDbgTrace(const char *name) {}
  void setName(const char *name) {}
  void begin() {}
  void end() {}
};

#endif

// ------------------------------------
struct CTraceScoped : public CDbgTrace {
  CTraceScoped(const char *name) : CDbgTrace(name) {
    begin();
  }
  ~CTraceScoped() {
    end();
  }
}; 

#endif

