#ifndef _AICONTROLLER_H
#define _AICONTROLLER_H

// ai controllers using maps to function pointers for easy access and scalability. 

// we put this here so you can assert from all controllers without including everywhere
#include <assert.h>	
#include <string>
#include <map>
#include "entity/entity.h"
#include "components/comp_base.h"
#include "components/comp_transform.h"

// states are a map to member function pointers, to 
// be defined on a derived class. 
class IAIController;

typedef void (IAIController::*statehandler)();

class IAIController : public TCompBase {

protected:
  CHandle         h_entity;
  CHandle         h_transform;        // Cached
  TCompTransform* getMyTransform();

  std::string                         state;
  // the states, as maps to functions
  std::map<std::string, statehandler> statemap;

public:
  void debugInMenu();
  void ChangeState(const std::string&);	// state we wish to go to
  virtual void Init() { }// resets the controller
  void update(float dt);	// recompute behaviour
  void AddState(const std::string&, statehandler);
  void setEntity(CHandle new_entity);
};

#endif