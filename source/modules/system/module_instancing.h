#pragma once

#include "modules/module.h"
#include "render/mesh/mesh_instanced.h"

class TInstance;
class TCompTransform;
class TEntityParseContext;

// We should use a template here instead of differentiating the types
class CModuleInstancing : public IModule {

    // -------------------------------------------------------------------
    struct TInstance {
        MAT44 world;
    };

    struct TDynInstance {
        MAT44 world;
        float time;
        float total_time;
    };

    // -------------------------------------------------------------------
    struct TInstanceVolume {
        MAT44 world;
        VEC3 light_pos;
    };

    // -------------------------------------------------------------------
    struct TRenderParticle {
        VEC3  pos;      // Maps to iCenter.xyz
        float angle;    // Maps to iCenter.w
        VEC4  color;
        float scale_x;
        float scale_y;
        float nframe;
    };

    // Static instances, for testing purposes
    CHandle scene_group;

    CRenderMeshInstanced* grass_instances_mesh = nullptr;
    std::vector< TInstance > grass_instances;

    // -------------------------------------------------------------------
    CRenderMeshInstanced* instances_mesh = nullptr;
    std::vector< TInstance > instances;

    // -------------------------------------------------------------------
    CRenderMeshInstanced* particles_instances_mesh = nullptr;
    std::vector<TRenderParticle> particles_instances;

    // Dynamic global instances, to be hold in the map.
    // -------------------------------------------------------------------
    struct TInstanceCollector {
        std::vector<TInstance> _instances;
        CRenderMeshInstanced* _instances_mesh;
    };
    
    struct TDynInstanceCollector {
        std::vector<TDynInstance> _instances;
        CRenderMeshInstanced* _instances_mesh;
    };

    std::map<std::string, std::string> _global_names;
    std::map<std::string, TInstanceCollector> _global_instances;
    std::map<std::string, TDynInstanceCollector> _dynamic_instances;

    void updateTimedInstances(float dt);

public:

    CModuleInstancing(const std::string& name) : IModule(name) {}

    bool stop() override;
    bool start() override;
    void render() override;
    void update(float delta) override;
    void renderMain();

    int addDynamicInstance(const std::string & name, const std::string & mat, MAT44 w_matrix, float time);
    int addInstance(const std::string & name, const std::string & type, MAT44 w_matrix);

    int addCustomInstance(const std::string & name, const std::string & type, MAT44 w_matrix);
    void removeInstance(TInstance* instance);
    void updateInstance(const std::string& name, int index, const MAT44& w_matrix);
    void clearInstance(const std::string& name);
    void clearInstances();

    bool parseInstance(const json& j, TEntityParseContext& ctx);
    bool parseContainer(const json& j, TEntityParseContext& ctx);
};