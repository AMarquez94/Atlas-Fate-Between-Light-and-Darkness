#pragma once

#include "components/comp_base.h"

class CCurve;
using namespace std;

class TCompButton : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();

private:
	string _scriptName;
	float _delay;

	void onMsgButtonActivated(const TMsgButtonActivated& msg);

};

