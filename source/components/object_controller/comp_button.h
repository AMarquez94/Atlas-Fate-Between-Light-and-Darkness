#pragma once

#include "components/comp_base.h"

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
    void setCanBePressed(bool canBePressed);
    bool canBePressed;

private:

    std::string _script;

    void onMsgButtonActivated(const TMsgButtonActivated& msg);
	void onMsgGroupCreated(const TMsgEntitiesGroupCreated& msg);
	void onMsgEntityCreated(const TMsgEntityCreated& msg);
};

