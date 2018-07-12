#include "mcv_platform.h"
#include "comp_fsm.h"

DECL_OBJ_MANAGER("fsm", TCompFSM);

// -------------------------------------------------
void TCompFSM::debugInMenu()
{
  // FSM
  ImGui::Text("Core FSM: %s", _context.getFSM()->getName().c_str());

  // internal state
  FSM::CContext::EState intState = _context.getInternalState();
  const char* stName = "";
  if (intState == FSM::CContext::EState::IDLE)          stName = "IDLE";
  else if (intState == FSM::CContext::EState::RUNNING)  stName = "RUNNING";
  else if (intState == FSM::CContext::EState::FINISHED) stName = "FINISHED";
  ImGui::Text("Internal state: %s", stName);
  
  // current state
  const FSM::IState* currState = _context.getCurrentState();
  ImGui::Text("Current state: %s", currState ? currState->getName().c_str() : "...");

  // time in state
  ImGui::Text("Time in state: %.2f", _context.getTimeInState());

  // variables
  const MVariants& variables = _context.getVariables();
  if (ImGui::TreeNode("Variables"))
  {
    for (auto& var : variables.getVariants())
    {
      ImGui::Text("%s = %s", var.first.c_str(), var.second.toString().c_str());
    }
    ImGui::TreePop();
  }
  paused = false;
}

void TCompFSM::registerMsgs() {
  DECL_MSG(TCompFSM, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompFSM, TMsgSetFSMVariable, onVariableValue);
  DECL_MSG(TCompFSM, TMsgScenePaused, onPaused); 
  DECL_MSG(TCompFSM, TMsgConsoleOn, onConsoleChanged);
  DECL_MSG(TCompFSM, TMsgNoClipToggle, onMsgNoClipToggle);
}

const std::string TCompFSM::getStateName()
{
	return _context.getCurrentState()->getName();
}

// -------------------------------------------------
void TCompFSM::load(const json& j, TEntityParseContext& ctx)
{
  const std::string& filename = j["file"];
  const FSM::CMachine* myFsm = Resources.get(filename)->as<FSM::CMachine>();
  assert(myFsm);
  _context.setFSM(myFsm);
}

void TCompFSM::update(float dt) 
{
	if (!paused && !isConsoleOn && !isInNoClipMode) {
		_context.update(dt);
	}
}

void TCompFSM::onCreate(const TMsgEntityCreated& msg) {
  _context.setOwner(CHandle(this).getOwner());
  _context.start();
}

void TCompFSM::onPaused(const TMsgScenePaused& msg) {

	paused = msg.isPaused;
}

void TCompFSM::onVariableValue(const TMsgSetFSMVariable& msg)
{
  _context.setVariable(msg.variant.getName(), msg.variant);
}

void TCompFSM::onConsoleChanged(const TMsgConsoleOn & msg)
{
	isConsoleOn = msg.isConsoleOn;
}

void TCompFSM::onMsgNoClipToggle(const TMsgNoClipToggle & msg)
{
    isInNoClipMode = !isInNoClipMode;
}
