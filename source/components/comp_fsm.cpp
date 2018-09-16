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

  if (ImGui::CollapsingHeader("Action Historic")) {
      ImGui::Separator();
      for (int i = 0; i < historic.size(); i++) {
          ImGui::Text("Action: %s - %d", historic[i].action, historic[i].number_of_times);
      }
      ImGui::Separator();
  }
}

void TCompFSM::registerMsgs() {
  DECL_MSG(TCompFSM, TMsgEntityCreated, onCreate);
  DECL_MSG(TCompFSM, TMsgSetFSMVariable, onVariableValue);
  DECL_MSG(TCompFSM, TMsgScenePaused, onPaused); 
  DECL_MSG(TCompFSM, TMsgConsoleOn, onConsoleChanged);
  DECL_MSG(TCompFSM, TMsgNoClipToggle, onMsgNoClipToggle);
  DECL_MSG(TCompFSM, TMsgPlayerAIEnabled, onMsgAIMode);
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
	if (!paused && !isConsoleOn && !isInNoClipMode  /*&& !isInAIMode*/) {
		_context.update(dt);
        addActionToHistoric(_context.getCurrentState()->getName());
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

void TCompFSM::onMsgAIMode(const TMsgPlayerAIEnabled & msg)
{
    isInAIMode = msg.enableAI;
    if (isInAIMode) {
        _context.restart();
    }
}



/* TODO: Delete */
void TCompFSM::addActionToHistoric(const std::string & action)
{
    if (historic.size() == 0 || std::strcmp(historic[historic.size() - 1].action, action.c_str()) != 0) {
        HistoricalAction historicalAction;
        historicalAction.action = action.c_str();
        historicalAction.number_of_times = 1;
        historic.push_back(historicalAction);
        if (historic.size() >= 500) {
            historic.erase(historic.begin());
        }
    }
    else {
        historic[historic.size() - 1].number_of_times += 1;
    }
}
