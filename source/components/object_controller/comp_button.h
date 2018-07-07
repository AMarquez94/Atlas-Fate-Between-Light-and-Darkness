#pragma once

#include "components/comp_base.h"

class CCurve;
using namespace std;

struct script {
	std::string name;
	float delay;
};

class TCompButton : public TCompBase
{
	DECL_SIBLING_ACCESS();

public:
	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();

private:
	vector <script> _scripts;
	float _delay;

	void onMsgButtonActivated(const TMsgButtonActivated& msg);

};

