#pragma once

#include "module.h"

class CModuleRender : public IModule
{
public:
	CModuleRender(const std::string& name);
  bool start() override;
  bool stop() override;
  void update(float delta) override;
  void render() override;

  void generateFrame();

  void configure(int xres, int yres);
  void setBackgroundColor(float r, float g, float b, float a);
  
  LRESULT OnOSMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
  int _xres;
  int _yres;
  float _backgroundColor[4];
};