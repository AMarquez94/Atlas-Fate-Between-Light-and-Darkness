#pragma once

#include "components/comp_base.h"

class CCurve;

class TCompCurve : public TCompBase
{
private:

    int _dbg_smooth;

    CHandle _target;
    std::string _targetName;

    // TODO: BORRAR
    std::vector<VEC3> historicPos;
    std::vector<float> historicRatio;

    const CCurve* _curve = nullptr;
    float _ratio = 0.f;
    float _speed = 0.f;
    float _sensitivity = 0.f;

    bool _active = true;
    bool _loop = false;
    bool _finished = false;

    VEC3 getTargetPos();
    void onMsgCameraActive(const TMsgCameraActivated & msg);
    void onMsgCameraFullActive(const TMsgCameraFullyActivated& msg);
    void onMsgCameraDeprecated(const TMsgCameraDeprecated & msg);

    DECL_SIBLING_ACCESS();

public:

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    void setRatio(float ratio) { _ratio = ratio; }

    static void registerMsgs();
};

