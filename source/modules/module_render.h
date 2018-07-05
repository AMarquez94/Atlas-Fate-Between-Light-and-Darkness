#pragma once

#include "module.h"
#include "camera/camera.h"
#include "handle/handle_def.h"
#include "render/deferred_renderer.h"

class CRenderToTexture;

class CModuleRender : public IModule
{
	CDeferredRenderer deferred;
	CRenderToTexture* rt_main = nullptr;

	//console variables
	bool _debugMode = true, _generateShadows = true, _generatePostFX = true, _showWireframe = false, _hideBackground = false, _showAllColliders = false, _showDynamicColliders = false;

public:


	CModuleRender(const std::string& name);
	bool start() override;
	bool stop() override;
	void update(float delta) override;
	void render() override;

	void generateFrame();
	void activateMainCamera();
	void renderWireframeLayer(bool onTop);
	void renderCollidersLayer(bool onlyDynamics);

	void configure(int xres, int yres);
	void setBackgroundColor(float r, float g, float b, float a);
	void debugDraw();

	//get + set methods
	bool const getDebugMode() { return _debugMode; }
	void setDebugMode(bool val) { _debugMode = val; }
	bool const getGenerateShadows() { return _generateShadows; }
	void setGenerateShadows(bool val) { _generateShadows = val; }
	bool const getGeneratePostFX() { return _generatePostFX; }
	void setGeneratePostFX(bool val) { _generatePostFX = val; }
	bool const getShowWireframe() { return _showWireframe; }
	void setShowWireframe(bool val) { _showWireframe = val; }
	bool const getShowAllColliders() { return _showAllColliders; }
	void setShowAllColliders(bool val) { _showAllColliders = val; }
	bool const getShowDynamicColliders() { return _showDynamicColliders; }
	void setShowDynamicColliders(bool val) { _showDynamicColliders = val; }
	CHandle getMainCamera();


	LRESULT OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
	CHandle h_e_camera;
	CCamera camera;

	int _xres;
	int _yres;
	VEC4 _backgroundColor;
};