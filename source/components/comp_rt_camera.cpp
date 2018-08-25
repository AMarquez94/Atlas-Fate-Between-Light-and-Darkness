#include "mcv_platform.h"
#include "comp_rt_camera.h"
#include "handle/handle.h"
#include "resources/resources_manager.h"
#include "render/render_manager.h"
#include "render/render_objects.h"
#include "render/render_utils.h"
#include "components/comp_camera.h"
#include "components/comp_transform.h"

DECL_OBJ_MANAGER("rt_camera", TCompRTCamera);

TCompRTCamera::TCompRTCamera() : rt(nullptr), resolution(512, 512) { }

// This should be local vars
CDeferredRenderer tx_deferred;

void TCompRTCamera::load(const json& j, TEntityParseContext& ctx) {
    
    resolution = VEC2(512, 512);
    if (j.count("resolution"))
        resolution = loadVEC2(j["resolution"]);

    color_fmt = readFormat(j, "color_fmt");
    depth_fmt = readFormat(j, "depth_fmt");
    name = j.value("name", "");
    assert(!name.empty());

    if (j.count("custom_categories")) {
        auto& j_cats = j["custom_categories"];
        assert(j_cats.is_array());
        for (size_t i = 0; i < j_cats.size(); ++i) {
            if (j_cats[i].is_string())
                custom_categories.push_back(j_cats[i]);
        }
    }

    if (!isValid())
        init();
}

void TCompRTCamera::init() {

    assert(rt == nullptr);
    rt = new CRenderToTexture();
    bool is_ok = rt->createRT(name.c_str(), resolution.x, resolution.y, color_fmt, depth_fmt);
    assert(is_ok);

    // We will replace the env.dds texture with our render target
    tx_deferred.create(resolution.x, resolution.y, name.c_str());
}

void TCompRTCamera::generate(CDeferredRenderer& renderer) {

    CEntity * rt_cam = CHandle(this).getOwner();
    TCompCamera * cam = rt_cam->get<TCompCamera>();
    activateCamera(*cam, resolution.x, resolution.y);

    if (custom_categories.size() > 0) {
        rt->clear(VEC4(0, 0, 0, 0));
        rt->activateRT();
        for(auto cat: custom_categories)
            CRenderManager::get().renderCategory(cat.c_str());
    }
    else {
        tx_deferred.render(rt, CHandle(this).getOwner());
    }

    CRenderToTexture::setNullRT();
}