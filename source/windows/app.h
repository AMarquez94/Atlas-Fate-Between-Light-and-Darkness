#ifndef INC_APP_H_
#define INC_APP_H_

#include "mcv_platform.h"
#include "utils/directory_watcher.h"

class CApp {

  // Windows stuff
  HWND hWnd;
  HINSTANCE hInstance;
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  // 
  CTimer time_since_last_render;
  CDirectoyWatcher resources_dir_watcher;
  bool             has_focus = false;

  static CApp* app_instance;

public:
	int xres = 0;
	int yres = 0;
	bool resetMouse = false;
	bool hasFocus = true;
	bool lostFocus = false;

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

  bool hasFocus() const { return has_focus; }

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
