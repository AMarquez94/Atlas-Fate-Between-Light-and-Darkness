#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class CModuleLerp : public IModule
{
	struct LerpingElement {

		float *element_to_lerp;
		float max_element_to_lerp;
		float value_to_lerp;
		float time_to_start_lerping = 0.0;
		float time_to_end_lerp;
		float current_time = 0.0;
	};

	std::list<LerpingElement> _elements_to_lerp;

public:

	CModuleLerp(const std::string& aname) : IModule(aname) { }
	void update(float delta) override;
	void render() override;

	void lerpElement(float *element_to_lerp, float value_to_lerp, float time_to_lerp, float time_to_start = 0.0f);

};

#pragma once
