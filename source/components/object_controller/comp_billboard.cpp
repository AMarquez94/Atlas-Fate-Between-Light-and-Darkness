#include "mcv_platform.h"
#include "comp_billboard.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"
#include "components/comp_camera.h"

DECL_OBJ_MANAGER("billboard", TCompBillBoard);

void TCompBillBoard::debugInMenu() {


}

void TCompBillBoard::load(const json& j, TEntityParseContext& ctx) {

}

void TCompBillBoard::registerMsgs()
{

}

void TCompBillBoard::update(float dt) {

    TCompTransform * self_transform = get<TCompTransform>();
    float yaw, pitch, roll;
    
    CEntity* eCurrentCamera = Engine.getCameras().getOutputCamera();
    assert(eCurrentCamera);
    TCompCamera* camera = eCurrentCamera->get<TCompCamera>();
    assert(camera);

    float new_yaw = getYawFromVector(-camera->getFront());
    self_transform->getYawPitchRoll(&yaw, &pitch, &roll);
    self_transform->setYawPitchRoll(new_yaw, pitch, 0);
}