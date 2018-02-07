#ifndef INC_APP_H_
#define INC_APP_H_

#include "mcv_platform.h"

class CApp {

  // Windows stuff
  HWND hWnd;
  HINSTANCE hInstance;
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  // 
  int xres = 0;
  int yres = 0;
  CTimer time_since_last_render;

  static CApp* app_instance;

public:
	bool resetMouse = false;

  static CApp& get() {
    assert(app_instance);
    return *app_instance;
  }

  HWND getWnd() const { return hWnd; }

  CApp() {
    assert(app_instance == nullptr);
    app_instance = this;
  }

  ~CApp() {
    assert(app_instance == this);
    app_instance = nullptr;
  }

  // Remove copy ctor
  CApp(const CApp&) = delete;

  bool readConfig();
  bool createWindow(HINSTANCE new_hInstance, int nCmdShow);
  bool start();
  void mainLoop();
  void doFrame();
  bool stop();
};

#endif
