#pragma once

#include "mcv_platform.h"
#include "gpu_buffer.h"
#include "compute_task.h"

class CGPUBuffer;

namespace GPUParticles {

    struct TStateInstance {
        VEC3  pos;
        float size;
        VEC3  vel;
        float unit_time;        // Between 0..1
        float time_factor;      // unit_time += time_factor*delta_time;
        float dummy1;
        float dummy2;
        float dummy3;
    };

    struct TRenderInstance {
        VEC3  pos;
        float size;
    };

    // Core parameters
    struct TSystemCore {
        VEC3  center;
        float delta_time = 0.f;
        VEC3  speed;
        float radius = 10.0f;
        float height = 10.0f;
        float life_time = 1.0f;
        float min_size = 1.0f;
        float max_size = 1.0f;
        bool load(const json& j);
        void debugInMenu();
        void initParticle(TStateInstance* s) const;
    };

    struct TSystemInstance {
        CComputeTask       update_task;
        CRenderTechnique*  tech = nullptr;
        TSystemCore        core;

        typedef TGPUTypedBuffer<TStateInstance> VStates;
        typedef TGPUTypedBuffer<TRenderInstance> VRenders;
        typedef TGPUTypedBuffer<TSystemCore> VCore;

        // Store state and prev state
        static const int num_states = 2;
        VStates states[num_states];
        int     curr_state_idx = 0;

        // To render the particles
        VRenders curr_render;

        VCore    gpu_core;

        bool               use_cpu_update = false;

        int                nparticles = 0;
        bool create(const json& j);
        void destroy();
        void initState(VStates& state);
        void updateStatesCPU(VStates& prev, float delta, VStates& next, VRenders& render);
        void updateStatesGPU(VStates& prev, float delta, VStates& next, VRenders& render);
        void update(float dt);
        void debugInMenu();

    };

}
