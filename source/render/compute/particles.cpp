#include "mcv_platform.h"
#include "particles.h"

extern float unitRandom();
extern float randomFloat(float vmin, float vmax);
extern VEC3 randomVEC3(VEC3 vmin, VEC3 vmax);

namespace GPUParticles {

    // --------------------------------------------------
    bool TSystemCore::load(const json& j) {
        if (j.count("center"))
            center = loadVEC3(j["center"]);
        if (j.count("speed"))
            speed = loadVEC3(j["speed"]);
        radius = j.value("radius", radius);
        height = j.value("height", height);
        life_time = j.value("life_time", life_time);
        min_size = j.value("min_size", min_size);
        max_size = j.value("max_size", max_size);
        return true;
    }

    // --------------------------------------------------
    void TSystemCore::debugInMenu() {
        ImGui::DragFloat3("Speed", &speed.x, 0.01f, -5.f, 5.f);
        ImGui::DragFloat("Radius", &radius, 0.01f, 0.f, 25.f);
        ImGui::DragFloat("Height", &height, 0.01f, 0.f, 25.f);
        ImGui::DragFloat("Life Time", &life_time, 0.01f, 0.1f, 25.f);
        ImGui::DragFloat("Size At Start", &min_size, 0.01f, 0.0f, 10.f);
        ImGui::DragFloat("Size At End", &max_size, 0.01f, 0.0f, 10.f);
    }

    // --------------------------------------------------
    void TSystemCore::initParticle(TStateInstance* s) const {
        float new_life = life_time * randomFloat(0.8f, 1.2f);
        s->pos = randomVEC3(
            VEC3(-radius, 0, -radius),
            VEC3(radius, height, radius) \
        );
        s->vel = speed;
        s->time_factor = 1.0f / new_life;
        s->size = 1.0f;
        s->unit_time = 0.f;
        s->unit_time = unitRandom();
    }

    // --------------------------------------------------
    void TSystemInstance::initState(VStates& state) {

        auto d = state.data();
        for (size_t i = state.size(); i--; ++d)
            core.initParticle(d);
    }

    // --------------------------------------------------
    void TSystemInstance::updateStatesCPU(VStates& prev, float delta, VStates& next, VRenders& render) {
        assert(prev.size() == next.size());
        assert(prev.size() == render.size());
        auto s = prev.data();     // source
        auto d = next.data();     // destination
        auto r = render.data();     // destination
        for (size_t i = prev.size(); i--; ++s, ++d, ++r) {

            d->pos = s->pos + s->vel * delta;
            d->vel = s->vel;
            d->unit_time += delta * s->time_factor;
            d->time_factor = s->time_factor;

            if (d->unit_time > 1.0f)
                core.initParticle(d);

            // Fill render info
            r->pos = d->pos;
            r->size = core.min_size + (core.max_size - core.min_size) * d->unit_time;
        }
    }


    // --------------------------------------------------
    void TSystemInstance::updateStatesGPU(VStates& prev, float delta, VStates& next, VRenders& render) {

        // Update current CPU core values to the GPU buffer
        core.delta_time = delta;
        assert(gpu_core.data());
        memcpy(gpu_core.data(), &core, sizeof(core));
        gpu_core.copyCPUtoGPU();

        update_task.bind("inState", &prev);
        update_task.bind("outState", &next);
        update_task.bind("outRender", &render);
        update_task.bind("core", &gpu_core);
        int n = next.size();
        if (!n)
            return;
        int nx = std::min(65535, n);
        int ny = std::max(1, n / 65536);
        update_task.sizes[0] = nx;
        update_task.sizes[1] = ny;
        update_task.sizes[2] = 1;
        update_task.groups[0] = 1;
        update_task.groups[1] = 1;
        update_task.groups[2] = 1;
        update_task.run();

        //Render.ctx->CopyResource(next.buffer);
    }

    // --------------------------------------------------
    bool TSystemInstance::create(const json& j) {

        if (!core.load(j["core"]))
            return false;

        nparticles = j.value("nparticles", nparticles);

        if (!update_task.create(j["update"]))
            return false;

        // Create buffers
        for (int i = 0; i < num_states; ++i) {
            if (!states[i].create(nparticles))
                return false;
        }

        // One buffer to do the render
        if (!curr_render.create(nparticles))
            return false;

        if (!gpu_core.create(1))
            return false;

        // Create initial state
        curr_state_idx = 0;
        initState(states[curr_state_idx]);

        states[0].copyCPUtoGPU();

        return true;
    }

    // --------------------------------------------------
    void TSystemInstance::destroy() {
        for (int i = 0; i < num_states; ++i)
            states[i].destroy();
        curr_render.destroy();
        update_task.destroy();
    }

    // --------------------------------------------------
    void TSystemInstance::update(float dt) {

        if (dt > 1.0f)
            return;

        int prev_idx = curr_state_idx;
        int next_idx = (curr_state_idx + 1) % num_states;

        VStates& prev_state = states[prev_idx];
        VStates& next_state = states[next_idx];

        if (use_cpu_update)
            updateStatesCPU(prev_state, dt, next_state, curr_render);
        else
            updateStatesGPU(prev_state, dt, next_state, curr_render);

        curr_state_idx = next_idx;
    }

    // --------------------------------------------------
    void TSystemInstance::debugInMenu() {

        ImGui::Checkbox("Use CPU to update", &use_cpu_update);
        if (ImGui::SmallButton("Reset")) {
            curr_state_idx = 0;
            initState(states[curr_state_idx]);
            states[0].copyCPUtoGPU();
        }
        core.debugInMenu();
    }


}
