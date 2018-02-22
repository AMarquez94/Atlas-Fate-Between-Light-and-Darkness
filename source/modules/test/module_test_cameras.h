#pragma once

#include "modules/module.h"
#include "geometry/curve.h"

class CModuleTestCameras : public IModule
{
public:
	CModuleTestCameras(const std::string& name)
		: IModule(name)
	{}
	bool start() override;
	void update(float delta) override;
	void render() override;

private:
	CCurve _curve;
};