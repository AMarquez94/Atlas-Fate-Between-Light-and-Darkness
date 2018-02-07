#pragma once

#include "input/device.h"
#include <Xinput.h>

namespace Input
{
	class CPadXbox : public IDevice
	{
	public:
		CPadXbox(const std::string& name, int controllerId);
		virtual void updatePadData(float delta, TInterface_Pad& data) override;
		virtual void feedback(const TInterface_Feedback& data) override;

	private:
		float getButtonValue(const XINPUT_STATE& state, int bt);
		float getTriggerValue(const XINPUT_STATE& state, bool left);
		VEC2 getAnalogValues(const XINPUT_STATE& state, bool left);

		int _controllerId = 0;
	};
}