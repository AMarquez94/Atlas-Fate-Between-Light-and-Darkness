#include "mcv_platform.h"
#include "comp_suspect_manager.h"
#include "entity/entity_parser.h"

DECL_OBJ_MANAGER("suspect_manager", TCompSuspectManager);

void TCompSuspectManager::onEnemySuspecting(const TMsgEnemySuspecting & msg)
{
    bool found = false;
    int i = 0;
    for (i = 0; !found && i < suspectElements.size(); i++) {
        if (suspectElements[i].enemySuspecting == msg.enemy_suspecting) {
            found = true;
        }
    }

    if (msg.is_suspecting && !found) {
        createSuspectElement(msg.enemy_suspecting);
    }
    else if (!msg.is_suspecting && found) {
        suspectElements[i - 1].element.destroy();
        suspectElements.erase(suspectElements.begin() + i - 1);
    }
}

void TCompSuspectManager::createSuspectElement(CHandle enemySuspecting)
{
    TEntityParseContext ctxSuspectElement;
    parseScene("data/prefabs/suspect/suspect_triangle.prefab", ctxSuspectElement);

    CEntity *eSuspectElement = ctxSuspectElement.entities_loaded[0];
    TMsgEnemySuspecting msg;
    msg.enemy_suspecting = enemySuspecting;
    msg.is_suspecting = true;
    eSuspectElement->sendMsg(msg);

    SuspectElement element;
    element.element = ctxSuspectElement.entities_loaded[0];
    element.enemySuspecting = enemySuspecting;
    suspectElements.push_back(element);
}

void TCompSuspectManager::debugInMenu() {
}

void TCompSuspectManager::load(const json& j, TEntityParseContext& ctx) {

}

void TCompSuspectManager::update(float dt) {

}

void TCompSuspectManager::registerMsgs() {
    DECL_MSG(TCompSuspectManager, TMsgEnemySuspecting, onEnemySuspecting);
}
