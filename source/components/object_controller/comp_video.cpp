#include "mcv_platform.h"
#include "comp_video.h"
#include "components/comp_transform.h"
#include "components/comp_hierarchy.h"
#include "components/physics/comp_rigidbody.h"
#include "entity/common_msgs.h"
#include "physics/physics_collider.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"
#include "components/comp_camera.h"
#include "render/video/video_texture.h"
#include "components/comp_render.h"
#include "render/render_objects.h"
#include "render/texture/texture.h"
#include "render/texture/material.h"
#include "render/render_utils.h"
#include "components/comp_culling.h"
#include "components/comp_aabb.h"

DECL_OBJ_MANAGER("video", TCompVideo);

void TCompVideo::onMsgVideoStatus(const TMsgVideoStatus & msg) {

    if (_video) {
        enabled = msg.status;
        _video->setActive(msg.status);
        if (!enabled) {
            _video->setToBeReset(true);
        }
    }
}

void TCompVideo::onMsgEntityCreated(const TMsgEntityCreated& msg) {

    TCompRender * c_render = get<TCompRender>();
    const CTexture * tex = ((c_render->meshes[0].materials[0])->textures[TS_ALBEDO]);
    _video = (CVideoTexture*)tex;
}

void TCompVideo::debugInMenu() {

    ImGui::Checkbox("Checked", &enabled);
}

void TCompVideo::load(const json& j, TEntityParseContext& ctx) {

}

void TCompVideo::registerMsgs() {

    DECL_MSG(TCompVideo, TMsgVideoStatus, onMsgVideoStatus);
    DECL_MSG(TCompVideo, TMsgEntityCreated, onMsgEntityCreated);
}

void TCompVideo::update(float dt) {

    CEntity * owner = CHandle(this).getOwner();
    CEntity * cam = EngineRender.getMainCamera();

    const TCompCulling* culling = nullptr;
    if (cam)
        culling = cam->get<TCompCulling>();
    const TCompCulling::TCullingBits* culling_bits = culling ? &culling->bits : nullptr;

    if (owner && culling_bits) {
        CHandle aabb = owner->get<TCompAbsAABB>();
        TCompAbsAABB * p_aabb = owner->get<TCompAbsAABB>();

        if (p_aabb) {
            auto idx = aabb.getExternalIndex();
            bool test = culling_bits->test(idx);
            if (test != enabled) {
                enabled = test;
                _video->setActive(test);
            }
        }
    }
}