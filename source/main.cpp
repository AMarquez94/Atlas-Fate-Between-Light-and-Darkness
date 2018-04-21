#include "mcv_platform.h"
#include "windows/app.h"
#include "SLB/LogicManager.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {

	printf("Hello world\n");

	SLB::Manager m;
	LogicManager::BootLuaSLB(&m);

	SLB::Script s(&m);

	s.doFile("../scripts/test.lua");
	s.doString("OnPlayerKilled()");
	// unprotected, do not use
	//s.doString("Test()");

	// protected
	//lua_State* ls = s;
	auto p1 = s.exists("Test");
	auto p2 = s.exists("OnPlayerKilled");
	//s.get()

	s.doString("OnPlayerKilled()");

	printf("Goodbye world\n");
	system("PAUSE");
	/*
  CApp app;

  if (!app.readConfig())
	return -1;

  if (!app.createWindow(hInstance, nCmdShow))
	return -2;

  if (!app.start())
	return -3;

  app.mainLoop();

  app.stop();

  return 0;*/
}