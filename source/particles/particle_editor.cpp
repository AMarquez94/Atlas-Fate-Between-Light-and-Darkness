#include "mcv_platform.h"
#include "particle_system.h"
#include "particle_parser.h"
#include "particle_editor.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/comp_transform.h"
#include "components/comp_particles.h"
#include "components/comp_name.h"
#include "utils/dirent.h"
#include "utils/utils.h"
#include <random>

namespace ImGui
{
    static auto vector_getter = [](void* vec, int idx, const char** out_text)
    {
        auto& vector = *static_cast<std::vector<std::string>*>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
        *out_text = vector.at(idx).c_str();
        return true;
    };

    bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
    {
        if (values.empty()) { return false; }
        return Combo(label, currIndex, vector_getter, static_cast<void*>(&values), values.size());
    }

    bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
    {
        if (values.empty()) { return false; }
        return ListBox(label, currIndex, vector_getter,
            static_cast<void*>(&values), values.size());
    }

}

ParticlesEditor::ParticlesEditor() {

}

ParticlesEditor::~ParticlesEditor() {

}

void ParticlesEditor::Init() {

    readFiles();
}

// Read all the files from the particles folder
void ParticlesEditor::readFiles() {

    _particles_files.clear();

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir("data/particles/")) != NULL) {
        while ((ent = readdir(dir)) != NULL)
            _particles_files.push_back(ent->d_name);

        closedir(dir);
    }

    // To solved a problem with extra number of files given by the api
    _particles_files.erase(_particles_files.begin(), _particles_files.begin() + 2);
}

void ParticlesEditor::debugMenu() {

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.149f, 0.1607f, 0.188f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 255.0f, 255.0f, 255.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0, 0.0f, 0.0f, 0.75f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0, 0.0f, 0.0f, 0.75f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.219f, 0.349f, 0.501f, 0.75f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);

    // Render each render GUI explicit in order.
    ImGui::Begin("Particle Editor", NULL);
    {
        debugLoader();
        debugSystem();
        debugEmission();
        debugShape();
        debugVelocity();
        debugRenderer();
        debugNoise();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
}

// Main debug draw functions

void ParticlesEditor::debugLoader() {

    ImGui::Combo("##File Selected", &_internal_index, _particles_files);
    ImGui::SameLine();
    if (ImGui::SmallButton("Load"))
        LoadParticleSystem();

    ImGui::SameLine();
    if (ImGui::SmallButton("Save")) {
        saveParticleSystem();
    }

    ImGui::SameLine();
    if (ImGui::SmallButton("Reset")) {
    }

    ImGui::Separator();
}

void ParticlesEditor::debugSystem() {

    //CEntity* e = _main_debug;
    //TCompParticles * c_e_particles = e->get<TCompParticles>();
    //Particles::TCoreSystem * system = const_cast<Particles::TCoreSystem*>(c_e_particles->_core);

    if (ImGui::CollapsingHeader("System")) {

        ImGui::DragFloat("Duration", &_main_system->n_system.duration, 0.01f, 0.f, 50.f);
        ImGui::Checkbox("Looping", &_main_system->n_system.looping);
        ImGui::DragFloat("Start delay", &_main_system->n_system.start_delay, 0.01f, 0.f, 100.f);
        ImGui::DragFloat("Start lifetime", &_main_system->n_system.start_lifetime, 0.01f, 0.f, 100.f);
        ImGui::DragFloat("Start speed", &_main_system->n_system.start_speed, 0.01f, 0, 100.f);
        ImGui::DragFloat3("3D Start Size", &_main_system->n_system.d_start_size.x, 0.01f, 0, 100.f);
        ImGui::DragFloat("Start size", &_main_system->n_system.start_size, 0.01f, 0, 100.f);
        ImGui::DragFloat3("3D Start Rotation", &_main_system->n_system.d_start_rotation.x, 0.01f, 0, 100.f);
        ImGui::DragFloat("Start Rotation", &_main_system->n_system.start_rotation, 0.01f, 0, 100.f);
        ImGui::DragFloat("Randomize Rotation", &_main_system->n_system.random_rotation, 0.01f, 0, 100.f);
        ImGui::ColorEdit4("Start Color", &_main_system->n_system.start_color.x, 0.01f);
        ImGui::DragFloat("Gravity Modifier", &_main_system->n_system.gravity, 0.01f, 0, 100.f);
        ImGui::DragFloat("Simulation Speed", &_main_system->n_system.simulation_speed, 1, 1, 100);
        ImGui::DragInt("Max. Particles", &_main_system->n_system.max_particles, 1, 1, 100);
    }
}

void ParticlesEditor::debugEmission() {

    if (ImGui::CollapsingHeader("Emission")) {

        ImGui::DragFloat("Rate over Time", &_main_system->n_emission.rate_time, 0.01f, 0.f, 50.f);
        ImGui::DragFloat("Rate over Distance", &_main_system->n_emission.rate_distance, 0.01f, 0.f, 100.f);

    }
}

void ParticlesEditor::debugShape() {

    if (ImGui::CollapsingHeader("Shape")) {

        //ImGui::Combo("Type", &_main_system->n_shape.type, "");
        ImGui::DragFloat3("Size", &_main_system->n_shape.size.x, 0.01f, 0.f);
        ImGui::DragFloat("Angle", &_main_system->n_shape.angle, 0.01f, 0.f, 50.f);

    }
}

void ParticlesEditor::debugVelocity() {

    if (ImGui::CollapsingHeader("Velocity over Lifetime")) {

        ImGui::DragFloat3("Velocity", &_main_system->n_velocity.constant_velocity.x, 0.01f, 0.f, 50.f);

    }
}

void ParticlesEditor::debugColor() {

    if (ImGui::CollapsingHeader("Color over Lifetime")) {

        //ImGui::ColorEdit4("Color", &_main_system->n_color.colors.get(0).x, 0.01f);

    }
}

void ParticlesEditor::debugSize() {

    if (ImGui::CollapsingHeader("Size over Lifetime")) {

        //ImGui::DragFloat3("Rate over Time", &_main_system->life.duration, 0.01f, 0.f, 50.f);
        //ImGui::DragFloat3("Rate over Time", &_main_system->life.duration, 0.01f, 0.f, 50.f);
        //ImGui::DragFloat3("Rate over Time", &_main_system->life.duration, 0.01f, 0.f, 50.f);

    }

    if (ImGui::CollapsingHeader("Rotation over Lifetime")) {

        ImGui::DragFloat("Angular velocity", &_main_system->n_velocity.angular, 0.01f, 0.f, 50.f);

    }
}

void ParticlesEditor::debugNoise() {

    if (ImGui::CollapsingHeader("Noise")) {

        ImGui::DragFloat3("Rate over Time", &_main_system->life.duration, 0.01f, 0.f, 50.f);

    }
}

void ParticlesEditor::debugRenderer() {

    if (ImGui::CollapsingHeader("Renderer")) {

    }
}

void ParticlesEditor::saveParticleSystem() {

}

void ParticlesEditor::LoadParticleSystem() {

    _main_debug.destroy();

    // Create the debug handle
    _main_debug.create< CEntity >();
    CEntity* e = _main_debug;

    // Add a particle component
    CHandle h_comp = getObjectManager<TCompParticles>()->createHandle();
    e->set(h_comp.getType(), h_comp);

    h_comp = getObjectManager<TCompTransform>()->createHandle();
    e->set(h_comp.getType(), h_comp);

    h_comp = getObjectManager<TCompName>()->createHandle();
    e->set(h_comp.getType(), h_comp);
    TCompName * c_name = e->get<TCompName>();
    c_name->setName("dummy_particle");

    TCompParticles * c_e_particles = e->get<TCompParticles>();
    c_e_particles->_core = Resources.get("data/particles/" + _particles_files[_internal_index])->as<Particles::TCoreSystem>();
    _main_system = const_cast<Particles::TCoreSystem*>(c_e_particles->_core);
    e->sendMsg(TMsgEntityCreated{});
}