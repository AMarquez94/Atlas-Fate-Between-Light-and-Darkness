#include "mcv_platform.h"
#include "fsm.h"
#include "factory.h"
#include <fstream>

// for convenience
using json = nlohmann::json;


// ----------------------------------------------
class CFSMResourceClass : public CResourceClass {
public:
  CFSMResourceClass() {
    class_name = "FSMs";
    extensions = { ".fsm" };
  }
  IResource* create(const std::string& name) const override {
    dbg("Creating fsm %s\n", name.c_str());
    FSM::CMachine* res = new FSM::CMachine();
    bool is_ok = res->load(name);
    assert(is_ok);
    return res;
  }
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
const CResourceClass* getResourceClassOf<FSM::CMachine>() {
  static CFSMResourceClass the_resource_class;
  return &the_resource_class;
}

namespace FSM
{
  bool CMachine::load(const std::string& filename)
  {
    std::ifstream file_json(filename);
    json jData;
    file_json >> jData;

    CFactory& factory = Engine.getFSM().getFactory();

    auto& jStates = jData["states"];
    for (auto& jState : jStates)
    {
      const std::string& stType = jState["type"];
      const std::string& stName = jState["name"];
      bool isFinal = jState.value("final", false);
      
      IState* st = factory.makeState(stType);
      st->setName(stName);
      st->setFinal(isFinal);
      st->load(jState);
      _states.push_back(st);
    }

    auto& jTransitions = jData["transitions"];
    for (auto& jTransition : jTransitions)
    {
      const std::string& type = jTransition["type"];
      ITransition* tr = factory.makeTransition(type);
      const std::string& source = jTransition["source"];
      const std::string& target = jTransition["target"];
      bool negated = jTransition.value("negated", false);
      tr->setSource(getState(source));
      tr->setTarget(getState(target));
      tr->setNegated(negated);
      tr->load(jTransition);
      assert(tr->getSourceState() && tr->getTargetState());
      _transitions.push_back(tr);
    }

    auto& jVariables = jData["variables"];
    for (auto& jVariable : jVariables)
    {
      _variables.setVariant(jVariable);
    }

    const std::string& initialState = jData["initial_state"];
    _initialState = getState(initialState);

    return true;
  }

  const IState* CMachine::getState(const std::string& name) const
  {
    for (auto& state : _states)
    {
      if (state->getName() == name)
      {
        return state;
      }
    }
    return nullptr;
  }

  void CMachine::getStateTransitions(const IState* state, VTransitions& output) const
  {
    for (auto& transition : _transitions)
    {
      if (transition->getSourceState() == state)
      {
        output.push_back(transition);
      }
    }
  }
}