#include "mcv_platform.h"
#include "windows/app.h"
/*
#include "fmod/fmod.hpp"
#include <iostream>

#pragma comment(lib, "fmod64_vc.lib" )*/

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {

	// Objeto sound puede ser de dos tipos
	// Objeto stream o objeto sample
	/*
	FMOD::System *system;
	FMOD::Sound *sound;
	FMOD::Channel * channel = 0;
	FMOD_RESULT result;
	void *extradriverdata = 0;

	System_Create(&system);
	system->init(32, FMOD_INIT_NORMAL, extradriverdata);
	result = system->createStream("test.ogg", FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound ); // Carga de disco, decodificarse
	result = system->playSound(sound, 0, false, &channel);

	result = system->close();
	result = system->release();*/

	CApp app;

	if (!app.readConfig())
		return -1;

	if (!app.createWindow(hInstance, nCmdShow))
		return -2;

	if (!app.start())
		return -3;

	app.mainLoop();

	app.stop();

	return 0;
}