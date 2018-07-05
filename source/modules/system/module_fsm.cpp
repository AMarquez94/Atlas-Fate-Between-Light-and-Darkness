#include "mcv_platform.h"
#include "module_fsm.h"
//#include "utils/json.hpp"
//#include <fstream>

#include "fsm/fsm.h"
#include "fsm/custom_states.h"
#include "fsm/custom_transitions.h"

// for convenience
//using json = nlohmann::json;

CModuleFSM::CModuleFSM(const std::string& name)
	: IModule(name)
{}

bool CModuleFSM::start()
{
	Resources.registerResourceClass(getResourceClassOf<FSM::CMachine>());

	_factory.registerInstancer("IdleState", new FSM::StateInstancer<FSM::IdleState>());
	_factory.registerInstancer("WalkState", new FSM::StateInstancer<FSM::WalkState>());
	_factory.registerInstancer("WalkSlowState", new FSM::StateInstancer<FSM::WalkSlowState>());
	_factory.registerInstancer("RunState", new FSM::StateInstancer<FSM::RunState>());
	_factory.registerInstancer("FallState", new FSM::StateInstancer<FSM::FallState>());
	_factory.registerInstancer("CrouchState", new FSM::StateInstancer<FSM::CrouchState>());
	_factory.registerInstancer("CrouchWalkState", new FSM::StateInstancer<FSM::CrouchWalkState>());
	_factory.registerInstancer("CrouchWalkSlowState", new FSM::StateInstancer<FSM::CrouchWalkSlowState>());
	_factory.registerInstancer("EnterMergeState", new FSM::StateInstancer<FSM::EnterMergeState>());
	_factory.registerInstancer("MergeState", new FSM::StateInstancer<FSM::MergeState>());
	_factory.registerInstancer("ExitMergeState", new FSM::StateInstancer<FSM::ExitMergeState>());
	_factory.registerInstancer("ExitMergeCrouchedState", new FSM::StateInstancer<FSM::ExitMergeCrouchedState>());
	_factory.registerInstancer("LandMergeState", new FSM::StateInstancer<FSM::LandMergeState>());
	_factory.registerInstancer("SoftLandState", new FSM::StateInstancer<FSM::SoftLandState>());
	_factory.registerInstancer("HardLandState", new FSM::StateInstancer<FSM::HardLandState>());
	_factory.registerInstancer("AttackState", new FSM::StateInstancer<FSM::AttackState>());
	_factory.registerInstancer("InhibitorRemovedState", new FSM::StateInstancer<FSM::InhibitorRemovedState>());
	_factory.registerInstancer("InhibitorTryToRemoveState", new FSM::StateInstancer<FSM::InhibitorTryToRemoveState>());
	_factory.registerInstancer("DieState", new FSM::StateInstancer<FSM::DieState>());
	_factory.registerInstancer("DeadState", new FSM::StateInstancer<FSM::DeadState>());
	_factory.registerInstancer("GrabEnemyState", new FSM::StateInstancer<FSM::GrabEnemyState>());
	_factory.registerInstancer("MovingObjectState", new FSM::StateInstancer<FSM::MovingObjectState>());
	_factory.registerInstancer("PressingButtonState", new FSM::StateInstancer<FSM::PressingButtonState>());


	_factory.registerInstancer("VariableTransition", new FSM::TransitionInstancer<FSM::VariableTransition>());
	_factory.registerInstancer("TimeTransition", new FSM::TransitionInstancer<FSM::TimeTransition>());
	_factory.registerInstancer("MultipleVariableTransition", new FSM::TransitionInstancer<FSM::MultipleVariableTransition>());
	_factory.registerInstancer("AnimationTransition", new FSM::TransitionInstancer<FSM::AnimationTransition>());
    _factory.registerInstancer("SonarState", new FSM::StateInstancer<FSM::SonarState>());

	//const FSM::CMachine* myFsm = Resources.get("data/fsm/character.fsm")->as<FSM::CMachine>();
	//assert(myFsm);

	return true;
}

bool CModuleFSM::stop()
{
	return true;
}

void CModuleFSM::renderInMenu()
{
	//if (ImGui::TreeNode("Input"))
	//{
	//	// hosts
	//	for (int i = 0; i < Input::NUM_PLAYERS; ++i)
	//	{
	//		auto& host = _hosts[i];
	//		ImGui::PushID(&host);

	//		if (ImGui::TreeNode("Host"))
	//		{
	//			// devices
	//			if (ImGui::TreeNode("Devices"))
	//			{
	//				auto& devices = host.devices();
	//				for (auto& dev : devices)
	//				{
	//					ImGui::Text("%s", dev->getName().c_str());
	//				}
	//				ImGui::TreePop();
	//			}

	//			// keyboard
	//			if (ImGui::TreeNode("Keyboard"))
	//			{
	//				auto& keyboard = host.keyboard();
	//				for (int i = 0; i < Input::NUM_KEYBOARD_KEYS; ++i)
	//				{
	//					const std::string& keyName = getButtonName(Input::KEYBOARD, i);
	//					if (!keyName.empty())
	//					{
	//						ImGui::Text("%20s", keyName.c_str());
	//						ImGui::SameLine();
	//						ImGui::ProgressBar(keyboard.key(i).value);
	//					}
	//				}
	//				ImGui::TreePop();
	//			}

	//			// mouse
	//			if (ImGui::TreeNode("Mouse"))
	//			{
	//				auto& mouse = host.mouse();
	//				ImGui::Text("Position");
	//				ImGui::SameLine();
	//				ImGui::Value("X", mouse._position.x);
	//				ImGui::SameLine();
	//				ImGui::Value("Y", mouse._position.y);
	//				ImGui::Value("Wheel", mouse._wheel_delta);
	//				for (int i = 0; i < Input::MOUSE_BUTTONS; ++i)
	//				{
	//					const std::string& btName = getButtonName(Input::MOUSE, i);
	//					ImGui::Text("%20s", btName.c_str());
	//					ImGui::SameLine();
	//					ImGui::ProgressBar(mouse.button(Input::EMouseButton(i)).value);
	//				}
	//				ImGui::TreePop();
	//			}

	//			// pad
	//			if (ImGui::TreeNode("Pad"))
	//			{
	//				auto& pad = host.pad();
	//				for (int i = 0; i < Input::PAD_BUTTONS; ++i)
	//				{
	//					const std::string& btName = getButtonName(Input::PAD, i);
	//					ImGui::Text("%20s", btName.c_str());
	//					ImGui::SameLine();
	//					if (i < Input::PAD_LANALOG_X)
	//					{
	//						ImGui::ProgressBar(pad.button(Input::EPadButton(i)).value);
	//					}
	//					else
	//					{
	//						ImGui::ProgressBar(0.5f + 0.5f * pad.button(Input::EPadButton(i)).value);
	//					}
	//				}
	//				ImGui::TreePop();
	//			}

	//			// mapping
	//			if (ImGui::TreeNode("Mapping"))
	//			{
	//				auto& mapping = host.mapping();

	//				for (auto& map : mapping.buttons())
	//				{
	//					ImGui::Text("%20s", map.first.c_str());
	//					ImGui::SameLine();
	//					ImGui::ProgressBar(0.5f + 0.5f * map.second.result.value);
	//				}
	//				ImGui::TreePop();
	//			}
	//			ImGui::TreePop();
	//		}
	//		ImGui::PopID();
	//	}

	//	ImGui::TreePop();
	//}
}
