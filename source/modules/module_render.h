#pragma once

#include "module.h"
#include "camera/camera.h"
#include "handle/handle_def.h"

class CRenderToTexture;

class CModuleRender : public IModule
{
	CRenderToTexture* rt_main = nullptr;

public:
	CModuleRender(const std::string& name);
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void render() override;

	void generateFrame();
	void activateMainCamera();

	void configure(int xres, int yres);
	void setBackgroundColor(float r, float g, float b, float a);

	LRESULT OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	CHandle h_e_camera;
	CCamera camera;

	int _xres;
	int _yres;
	VEC4 _backgroundColor;
};