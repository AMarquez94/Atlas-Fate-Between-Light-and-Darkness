#include "mcv_platform.h"
#include "input/devices/pad_xbox.h"

#pragma comment(lib, "XINPUT9_1_0.lib")

namespace Input
{
	CPadXbox::CPadXbox(const std::string& name, int controllerId)
		: IDevice(name)
		, _controllerId(controllerId)
	{
	}

	void CPadXbox::updatePadData(float delta, TInterface_Pad& data)
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		// Simply get the state of the controller from XInput.
		data.connected = XInputGetState(_controllerId, &state) == ERROR_SUCCESS;
		if (data.connected)
		{
			data._buttons[PAD_A].update(delta, getButtonValue(state, XINPUT_GAMEPAD_A));
			data._buttons[PAD_B].update(delta, getButtonValue(state, XINPUT_GAMEPAD_B));
			data._buttons[PAD_X].update(delta, getButtonValue(state, XINPUT_GAMEPAD_X));
			data._buttons[PAD_Y].update(delta, getButtonValue(state, XINPUT_GAMEPAD_Y));
			data._buttons[PAD_START].update(delta, getButtonValue(state, XINPUT_GAMEPAD_START));
			data._buttons[PAD_OPTIONS].update(delta, getButtonValue(state, XINPUT_GAMEPAD_BACK));
			data._buttons[PAD_L1].update(delta, getButtonValue(state, XINPUT_GAMEPAD_LEFT_SHOULDER));
			data._buttons[PAD_L2].update(delta, getTriggerValue(state, true));
			data._buttons[PAD_L3].update(delta, getButtonValue(state, XINPUT_GAMEPAD_LEFT_THUMB));
			data._buttons[PAD_R1].update(delta, getButtonValue(state, XINPUT_GAMEPAD_RIGHT_SHOULDER));
			data._buttons[PAD_R2].update(delta, getTriggerValue(state, false));
			data._buttons[PAD_R3].update(delta, getButtonValue(state, XINPUT_GAMEPAD_RIGHT_THUMB));
			data._buttons[PAD_LEFT].update(delta, getButtonValue(state, XINPUT_GAMEPAD_DPAD_LEFT));
			data._buttons[PAD_RIGHT].update(delta, getButtonValue(state, XINPUT_GAMEPAD_DPAD_RIGHT));
			data._buttons[PAD_DOWN].update(delta, getButtonValue(state, XINPUT_GAMEPAD_DPAD_DOWN));
			data._buttons[PAD_UP].update(delta, getButtonValue(state, XINPUT_GAMEPAD_DPAD_UP));

			// analog sticks
			VEC2 leftAnalog = getAnalogValues(state, true);
			VEC2 rightAnalog = getAnalogValues(state, false);
			data._buttons[PAD_LANALOG_X].update(delta, leftAnalog.x);
			data._buttons[PAD_LANALOG_Y].update(delta, leftAnalog.y);
			data._buttons[PAD_RANALOG_X].update(delta, rightAnalog.x);
			data._buttons[PAD_RANALOG_Y].update(delta, rightAnalog.y);
		}
	}

	float CPadXbox::getButtonValue(const XINPUT_STATE& state, int bt)
	{
		return (state.Gamepad.wButtons & bt) != 0 ? 1.f : 0.f;
	}

	float CPadXbox::getTriggerValue(const XINPUT_STATE& state, bool left)
	{
		BYTE value = left ? state.Gamepad.bLeftTrigger : state.Gamepad.bRightTrigger;
		return static_cast<float>(value) / std::numeric_limits<BYTE>::max();
	}

	VEC2 CPadXbox::getAnalogValues(const XINPUT_STATE& state, bool left)
	{
		VEC2 value(0, 0);
		float deadZone = 0.f;
		if (left)
		{
			value.x = static_cast<float>(state.Gamepad.sThumbLX);
			value.y = static_cast<float>(state.Gamepad.sThumbLY);
			deadZone = static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		}
		else
		{
			value.x = static_cast<float>(state.Gamepad.sThumbRX);
			value.y = static_cast<float>(state.Gamepad.sThumbRY);
			deadZone = static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE); ;
		}

		// normalize [-1..1]
		value = value / std::numeric_limits<SHORT>().max();
		deadZone = deadZone / std::numeric_limits<SHORT>().max();

		float length = value.Length();
		if (length <= deadZone)
		{
			value = VEC2(0.f, 0.f);
		}
		else
		{
			value.Normalize();
			value = value * ((length - deadZone) / (1.0f - deadZone));
		}

		return value;
	}

	void CPadXbox::feedback(const TInterface_Feedback& data)
	{
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = static_cast<WORD>(data.leftRumble * std::numeric_limits<WORD>::max());
		vibration.wRightMotorSpeed = static_cast<WORD>(data.rightRumble * std::numeric_limits<WORD>::max());

		XInputSetState(_controllerId, &vibration);
	}
}