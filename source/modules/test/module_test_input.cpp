#include "mcv_platform.h"
#include "module_test_input.h"

bool CModuleTestInput::start()
{
  return true;
}

void CModuleTestInput::update(float delta)
{
	float r = 0.f;
	float g = 0.f;
	float b = 0.f;

	const Input::TButton& bt = CEngine::get().getInput().host(Input::PLAYER_1).keyboard().key(VK_SPACE);

	// ---------------------- TEST: button times
	/*const Input::TButton& bt = EngineInput[VK_SPACE];
	if (bt.getsPressed())
	{
		dbg("BUTTON PRESSED: %.2f\n", bt.time_released);
	}
	if (bt.getsReleased())
	{
		dbg("BUTTON RELEASED: %.2f\n", bt.time_pressed);
	}*/

	// ---------------------- TEST: mouse
	/*const Input::TInterface_Mouse& mouse = EngineInput.mouse();
	dbg("MOUSE POSITION: %.2f, %.2f\n", mouse._position.x, mouse._position.y);*/

	// ---------------------- TEST: analog values
	/*r = EngineInput[Input::PAD_L2].value;
	g = EngineInput[Input::PAD_R2].value;
	b = 0.5f + 0.5f * EngineInput[Input::PAD_LANALOG_X].value;*/

	// ---------------------- TEST: feedback
	/*Input::TInterface_Feedback feedback;
	feedback.leftRumble = EngineInput[Input::PAD_L2].value;
	feedback.rightRumble = EngineInput[Input::PAD_R2].value;
	EngineInput.feedback(feedback);*/

	// ---------------------- TEST: mapping
	r = EngineInput["red"].value;
	g = EngineInput["green"].value;
	b = 0.5f + 0.5f * EngineInput["blue"].value;

	CEngine::get().getRender().setBackgroundColor(r, g, b, 1.f);
}
