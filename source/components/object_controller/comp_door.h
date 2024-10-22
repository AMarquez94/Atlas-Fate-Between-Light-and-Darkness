#pragma once

#include "components/comp_base.h"

class TCompDoor : public TCompBase {

    enum EDoorState {
        OPENED,
        OPENING,
        CLOSED,
        CLOSING
    };

    EDoorState state;
    float timer = 0;

    std::string opening_anim;
    std::string loading_anim;
    std::string closing_anim;

    std::string closed_script;
    std::string opened_script;

    std::string open_soundevent;
    std::string close_soundevent;

    float time_to_open;
    float time_to_close;

    void onScenePaused(const TMsgScenePaused & msg);

    DECL_SIBLING_ACCESS();

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);
    static void registerMsgs();
    void open();
    void close();
    void load();
    void setOpenedScript(const std::string& script);
    void setClosedScript(const std::string& script);
};