#ifndef INC_APP_H_
#define INC_APP_H_

#include "mcv_platform.h"
#include "utils/directory_watcher.h"

class CApp {

	// FPS Counter to be displayed per screen
	struct FPSCounter {

		int n_frames = 0;
		float elapsed_time = 0.f;
	}fps_counter;

	// Windows stuff
	HWND hWnd;
	HINSTANCE hInstance;
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// 
	CTimer time_since_last_render;
	CDirectoyWatcher resources_dir_watcher;

	static CApp* app_instance;

public:

	int fps = 0;

	int  xres = 0;
	int  yres = 0;
    bool fullscreen = false;
	bool resetMouse = false;
	bool lostFocus = false;
	bool has_focus = false;


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
