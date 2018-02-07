#ifndef _AICONTROLLER_H
#define _AICONTROLLER_H

// ai controllers using maps to function pointers for easy access and scalability. 

// we put this here so you can assert from all controllers without including everywhere
// states are a map to member function pointers, to 
// be defined on a derived class. 
class CAIController;
class TEntity;

typedef void (CAIController::*statehandler)();

class CAIController
{
	std::string state;
	statehandler current_state;
	std::map<std::string,statehandler>statemap;

protected:

	TEntity * entity_self;
	TEntity * entity_target;

public:
	void ChangeState(std::string);	// state we wish to go to
	virtual void Init();	// resets the controller
	void Recalc();	// recompute behaviour
	void AddState(std::string,statehandler);
	void SetEntity(TEntity * target);
	void Configure(const json& j);
	const std::string& GetState();
};

#endif