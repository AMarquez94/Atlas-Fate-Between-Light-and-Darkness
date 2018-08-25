#include "mcv_platform.h"
#include "comp_render_cube.h"
#include "handle/handle.h"
#include "render/texture/render_to_cube.h"
#include "resources/resources_manager.h"
#include "render/render_manager.h"
#include "components/comp_transform.h"

DECL_OBJ_MANAGER("cube_render", TCompRenderCube);

TCompRenderCube::TCompRenderCube() : rt(nullptr), resolution(256) { }

// This should be local vars
CDeferredRenderer my_def;
ID3D11ShaderResourceView* prev_srv = nullptr;
ID3D11ShaderResourceView* new_srv = nullptr;
CTexture* prev_env = nullptr;

void TCompRenderCube::load(const json& j, TEntityParseContext& ctx) {

    resolution = j.value("res", 256);
    color_fmt = readFormat(j, "color_fmt");
    depth_fmt = readFormat(j, "depth_fmt");
    name = j.value("name", "");
    assert(!name.empty());
    if (!isValid())
        init();
}

void TCompRenderCube::init() {

    assert(rt == nullptr);
    rt = new CRenderToCube;
    bool is_ok = rt->create(name.c_str(), resolution, color_fmt, depth_fmt);

    assert(is_ok);

    // Hacking
    // We will replace the env.dds texture with our render target
    std::string dst_texture = "data/textures/cubemaps/env.dds"; // j.value("dst_texture", "");
    prev_env = (CTexture*)(Resources.get(dst_texture)->as<CTexture>());
    prev_srv = (ID3D11ShaderResourceView*)prev_env->getShaderResourceView();
    new_srv = (ID3D11ShaderResourceView*)rt->getShaderResourceView();
    prev_env->setDXParams(resolution, resolution, nullptr, new_srv);
    my_def.create(resolution, resolution, "cube");
}

void TCompRenderCube::generate(CDeferredRenderer& renderer) {

    CEntity* e = CHandle(this).getOwner();
    TCompTransform* trans = e->get<TCompTransform>();
    rt->setPosition(trans->getPosition());

    prev_env->setDXParams(resolution, resolution, nullptr, prev_srv);

    static float colors[6][4] =
    {
    { 1.f, 0.f, 0.f, 0.f },
    { 0.f, 1.f, 0.f, 0.f },
    { 0.f, 0.f, 1.f, 0.f },
    { 0.f, 1.f, 1.f, 0.f },
    { 1.f, 0.f, 1.f, 0.f },
    { 1.f, 1.f, 0.f, 0.f },
    };

    for (int i = 0; i < rt->getNSides(); ++i) {
        //CCamera camera;
        //rt->activateFace(i, &camera);
        //rt->clearColorBuffer(i, colors[i]);
        //rt->clearDepthBuffer();
        my_def.renderToCubeFace(rt, i);
    }

    CRenderToTexture::setNullRT();
    prev_env->setDXParams(resolution, resolution, nullptr, new_srv);
}