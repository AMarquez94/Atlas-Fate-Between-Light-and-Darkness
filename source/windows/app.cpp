#include "mcv_platform.h"
#include "app.h"
#include "render/render.h"
#include "input/devices/mouse.h"
#include <windowsx.h>

CApp* CApp::app_instance = nullptr;

// 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK CApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  PAINTSTRUCT ps;
  HDC hdc;

  // If the OS processes it, do not process anymore
  if (CEngine::get().getModules().OnOSMsg(hWnd, message, wParam, lParam))
    return 1;

  switch (message)
  {
  case WM_PAINT:
    // Validate screen repaint in os/windows 
    hdc = BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

	case WM_MOUSEMOVE:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			int posX = GET_X_LPARAM(lParam);
			int posY = GET_Y_LPARAM(lParam);
			mouse->setPosition(static_cast<float>(posX), static_cast<float>(posY));
			app_instance->resetMouse = true;
		}
	}
		break;

	case WM_MBUTTONDOWN:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			mouse->setButton(Input::MOUSE_MIDDLE, true);
			SetCapture(hWnd);
		}
	}
	break;

	case WM_MBUTTONUP:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			mouse->setButton(Input::MOUSE_MIDDLE, false);
			ReleaseCapture();
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			mouse->setButton(Input::MOUSE_LEFT, true);
			SetCapture(hWnd);
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			mouse->setButton(Input::MOUSE_LEFT, false);
			ReleaseCapture();
		}
	}
	break;

	case WM_RBUTTONDOWN:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			mouse->setButton(Input::MOUSE_RIGHT, true);
			SetCapture(hWnd);
		}
	}
	break;

	case WM_RBUTTONUP:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			mouse->setButton(Input::MOUSE_RIGHT, false);
			ReleaseCapture();
		}
	}
	break;

	case WM_MOUSEWHEEL:
	{
		Input::CMouse* mouse = static_cast<Input::CMouse*>(EngineInput.getDevice("mouse"));
		if (mouse)
		{
			float wheel_delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
			mouse->setWheelDelta(wheel_delta);
		}
	}
	break;

  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
bool CApp::createWindow(HINSTANCE new_hInstance, int nCmdShow) {

  hInstance = new_hInstance;

  // Register class
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = NULL;
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = "MCVWindowsClass";
  wcex.hIconSm = NULL;
  if (!RegisterClassEx(&wcex))
    return false;

  // Create window
  RECT rc = { 0, 0, xres, yres };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  hWnd = CreateWindow("MCVWindowsClass", "Direct3D 11 MCV Project",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
    NULL);
  if (!hWnd)
    return false;

  ShowWindow(hWnd, nCmdShow);
	//ShowCursor(false);

  return true;
}

//--------------------------------------------------------------------------------------
// Process windows msgs, or if nothing to do, generate a new frame
//--------------------------------------------------------------------------------------
void CApp::mainLoop() {
  // Main message loop
  MSG msg = { 0 };
  while (WM_QUIT != msg.message)
  {
    // Check if windows has some msg for us
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      doFrame();

			/*if(resetMouse)
			{
				RECT rect;
				GetWindowRect(getWnd(), &rect);
				SetCursorPos((rect.left + rect.right) / 2, (rect.top + rect.bottom) / 2);
				resetMouse = false;
			}*/
    }
  }
}

//--------------------------------------------------------------------------------------
// Read any basic configuration required to boot, initial resolution, full screen, modules, ...
//--------------------------------------------------------------------------------------
bool CApp::readConfig() {
  // ...
  xres = 1024;
  yres = 640;

  time_since_last_render.reset();

  CEngine::get().getRender().configure(xres, yres);
  return true;
}

//--------------------------------------------------------------------------------------
bool CApp::start() {
  return CEngine::get().start();
}

//--------------------------------------------------------------------------------------
bool CApp::stop() {
  return CEngine::get().stop();
}

//--------------------------------------------------------------------------------------
void CApp::doFrame() {
  float dt = time_since_last_render.elapsedAndReset();
  CEngine::get().update(dt);
  CEngine::get().render();
}

