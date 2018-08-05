#include "mcv_platform.h"
#include "comp_gpu_particles.h"
#include "comp_transform.h"
#include "comp_render.h"
#include "render/mesh/mesh_instanced.h"

DECL_OBJ_MANAGER("gpu_particles", TCompGPUParticles);

CRenderMeshInstanced* particles_instances_mesh = nullptr;

void TCompGPUParticles::debugInMenu() {
    assert(system_instance);
    system_instance->debugInMenu();
}

void TCompGPUParticles::load(const json& j, TEntityParseContext& ctx) {
    system_instance = new GPUParticles::TSystemInstance();
    bool is_ok = system_instance->create(j);
    assert(is_ok);
}

void TCompGPUParticles::update(float dt) {
    assert(system_instance);
    TCompTransform* c_trans = get<TCompTransform>();
    assert(c_trans);
    system_instance->core.center = c_trans->getPosition();
    system_instance->update(dt);

    if (!instances_mesh) {
        TCompRender* c_render = get<TCompRender>();
        assert(c_render);
        auto rmesh = c_render->meshes[0].mesh;
        instances_mesh = (CRenderMeshInstanced*)rmesh;
    }

    if (system_instance->use_cpu_update)
        instances_mesh->setInstancesData(system_instance->curr_render.data(), system_instance->curr_render.size(), sizeof(GPUParticles::TRenderInstance));
    else {
        instances_mesh->setInstancesData(nullptr, system_instance->curr_render.size(), sizeof(GPUParticles::TRenderInstance));
        // This only works if the buffers match the size, but since the render instances does allocations in x2, sizes will not match.
        // Render.ctx->CopyResource( instances_mesh->getVB(), system_instance->curr_render.buffer );

        // So, use a copy subresource
        Render.ctx->CopySubresourceRegion(instances_mesh->getVB(), 0, 0, 0, 0,
            system_instance->curr_render.buffer, 0, nullptr);
    }
}

