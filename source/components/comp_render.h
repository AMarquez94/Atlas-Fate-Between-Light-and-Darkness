#pragma once

#include "comp_base.h"

class CRenderMesh;
class CTexture;
class CRenderTechnique;
class CMaterial;

class TCompRender : public TCompBase {

public:

    ~TCompRender();

    // Properties
    VEC4               color = VEC4(1, 1, 1, 1);
    AABB               aabb;
    bool               global_enabled = true;
    VEC4               self_color = VEC4(1, 1, 1, 1);
    float              self_intensity = 1.f;

    // This represents a single object mesh with several materials. Not multiples meshes
    struct CMeshWithMaterials {
        bool               enabled = true;
        const CRenderMesh* mesh = nullptr;
        std::vector<const CMaterial*> materials;
    };
    std::vector<CMeshWithMaterials> meshes;

    void debugInMenu();
    void renderDebug();
    void load(const json& j, TEntityParseContext& ctx);

    void loadMesh(const json& j, TEntityParseContext& ctx);
    void onDefineLocalAABB(const TMsgDefineLocalAABB& msg);
    void onSetVisible(const TMsgSetVisible& msg);
    void refreshMeshesInRenderManager(bool delete_me_from_keys = true);
    void setMaterial(const std::string &name);

	void renderMeshes();

    static void registerMsgs();

    DECL_SIBLING_ACCESS();
};