#include "mcv_platform.h"
#include "particle_system.h"
#include "particle_parser.h"
#include "particle_editor.h"
#include "render/render_objects.h"
#include "components/comp_camera.h"
#include "components/comp_transform.h"
#include "components/comp_particles.h"
#include "components/comp_name.h"
#include "particles/particle_system.h"
#include "utils/dirent.h"
#include "utils/utils.h"
#include <random>

#include "imgui/curve.h"

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

    int Curve(const char *label, const ImVec2& size, int maxpoints, ImVec2 *points);
    float CurveValue(float p, int maxpoints, const ImVec2 *points);
    float CurveValueSmooth(float p, int maxpoints, const ImVec2 *points);
}

ParticlesEditor::ParticlesEditor() {

}

ParticlesEditor::~ParticlesEditor() {

}

void ParticlesEditor::Init() {

    _index_shape = 4;
    _particles_files = readFiles("data/particles/");
    _images_files = readFiles("data/textures/particles/");
}

// Read all the files from the particles folder
std::vector<std::string> ParticlesEditor::readFiles(const std::string & path) {

    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> _paths;

    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL)
            _paths.push_back(ent->d_name);

        closedir(dir);
    }

    // To solve a problem with extra number of files given by the api
    _paths.erase(_paths.begin(), _paths.begin() + 2);
    return _paths;
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
        if (_main_system != nullptr) {
            // Debug the entity so it can be edited in the panel
            CEntity * e_main = _main_debug;
            e_main->debugInMenu();
            ImGui::Separator();
            ImGui::Separator();

            debugSystem();
            debugEmission();
            debugShape();
            debugVelocity();
            debugColor();
            debugSize();
            debugNoise();
            debugRenderer();
        }
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
        LoadParticleSystem();
    }

    ImGui::Separator();
    ImGui::Separator();
}

void ParticlesEditor::debugSystem() {

    if (ImGui::CollapsingHeader("System")) {

        ImGui::DragFloat("Duration", &_main_system->n_system.duration, 0.01f, 0.f, 50.f);
        if (ImGui::Checkbox("Looping", &_main_system->n_system.looping)) {
            CEntity * e_main = _main_debug;
            TCompParticles * particles = e_main->get<TCompParticles>();
            Particles::CSystem * system = Engine.getParticles().getSystem(particles->_particles);
            system->launch();
        }

        ImGui::DragFloat("Start delay", &_main_system->n_system.start_delay, 0.01f, 0.f, 100.f);
        ImGui::DragFloat("Start lifetime", &_main_system->n_system.start_lifetime, 0.01f, 0.f, 100.f);
        ImGui::DragFloat("Start speed", &_main_system->n_system.start_speed, 0.01f, 0, 100.f);
        ImGui::DragFloat3("Start size", &_main_system->n_system.start_size.x, 0.01f, 0, 100.f);
        ImGui::DragFloat3("Start Rotation", &_main_system->n_system.start_rotation.x, 0.01f, 0, 100.f);
        ImGui::DragFloat("Randomize Rotation", &_main_system->n_system.random_rotation, 0.01f, 0.f, 1.f);
        ImGui::ColorEdit4("Start Color", &_main_system->n_system.start_color.x, 0.01f);
        ImGui::DragFloat("Gravity Modifier", &_main_system->n_system.gravity, 0.01f, -100, 100.f);
        ImGui::DragFloat("Simulation Speed", &_main_system->n_system.simulation_speed, 0.1f, 0, 100);
        ImGui::Checkbox("Collision on Ground", &_main_system->n_collision.collision);
        ImGui::DragInt("Max. Particles", &_main_system->n_system.max_particles, 1, 1, 100);
    }
}

void ParticlesEditor::debugEmission() {

    if (ImGui::CollapsingHeader("Emission")) {

        ImGui::DragFloat("Rate over Time", &_main_system->n_emission.rate_time, 0.01f, 0.f, 50.f);
        ImGui::DragFloat("Rate over Distance", &_main_system->n_emission.rate_distance, 0.01f, 0.f, 100.f);
        ImGui::DragFloat("Variation", &_main_system->n_emission.variation, 0.01f, 0.f, 50.f);
        //ImGui::DragFloat("Interval", &_main_system->n_emission.interval, 0.01f, 0.f, 100.f);
    }
}

void ParticlesEditor::debugShape() {

    if (ImGui::CollapsingHeader("Shape")) {

        _index_shape = _main_system->n_shape.type;
        float actual_deg = rad2deg(_main_system->n_shape.angle);
        if (ImGui::Combo("Type", &_index_shape, "Point\0Line\0Square\0Box\0Sphere\0Circle\0Cone")) {
            _main_system->n_shape.type = mappings(_index_shape);
        }

        ImGui::DragFloat3("Size", &_main_system->n_shape.size.x, 0.01f, 0.f);
        if (ImGui::DragFloat("Angle", &actual_deg, 0.1f, 0.f, 90.f)) {
            _main_system->n_shape.angle = deg2rad(actual_deg);
        }

        // Render the mesh by type
        debugRender();
    }
}

void ParticlesEditor::debugVelocity() {

    if (ImGui::CollapsingHeader("Velocity over Lifetime")) {

        if (ImGui::Curve("Range [0,1]", ImVec2(600, 200), 10, _values_velocity))
        {
            _main_system->n_velocity.velocity = TTrack<VEC3>();
            for (unsigned int i = 0; i < sizeof((_values_velocity)) / sizeof((_values_velocity[0])); i++) {
                if (_values_velocity[i].x <= 0 && _values_velocity[i].y <= 0)
                    break;
                _main_system->n_velocity.velocity.set(_values_velocity[i].x, VEC3(_values_velocity[i].y, _values_velocity[i].y, _values_velocity[i].y));
            }
        }

        ImGui::DragFloat3("Velocity", &_main_system->n_velocity.constant_velocity.x, 0.01f, 0.f, 50.f);
        ImGui::DragFloat("Acceleration", &_main_system->n_velocity.acceleration, 0.01f, 0.f, 50.f);
        ImGui::DragFloat("Wind", &_main_system->n_velocity.wind, 0.01f, 0.f, 100.f);
    }
}

void ParticlesEditor::debugColor() {

    if (ImGui::CollapsingHeader("Color over Lifetime")) {

        ImGui::ColorEdit4("Color", &_main_system->n_system.start_color.x, 0.01f);
        // To add in the future.
        //ImGui::ColorEdit4("Color", &_main_system->n_color.colors.get(0).x, 0.01f);

    }
}

void ParticlesEditor::debugSize() {

    if (ImGui::CollapsingHeader("Size over Lifetime")) {

        if (ImGui::Curve("Range [0,1]", ImVec2(600, 200), 10, _values_size))
        {
            _main_system->n_size.sizes = TTrack<VEC3>();
            for (unsigned int i = 0; i < sizeof((_values_size)) / sizeof((_values_size[0])); i++) {
                if (_values_size[i].x == 0 && _values_size[i].y == 0)
                    break;
                _main_system->n_size.sizes.set(_values_size[i].x, VEC3(_values_size[i].y, _values_size[i].y, _values_size[i].y));
            }

            _main_system->n_size.sizes.sort();
        }
    }

    //if (ImGui::CollapsingHeader("Rotation over Lifetime")) {

    //    if (ImGui::Curve("Range [0,1]", ImVec2(600, 200), 10, foo))
    //    {
    //        // curve changed
    //        dbg("test");
    //    }
    //    ImGui::DragFloat("Angular velocity", &_main_system->n_velocity.angular, 0.01f, 0.f, 50.f);
    //}
}

void ParticlesEditor::debugNoise() {

    if (ImGui::CollapsingHeader("Noise")) {

        //ImGui::DragFloat3("WIP [NOT WORKING YET]", &_main_system->life.duration, 0.01f, 0.f, 50.f);

    }
}

void ParticlesEditor::debugRenderer() {

    if (ImGui::CollapsingHeader("Renderer")) {
        if (ImGui::Combo("Texture Selected", &_tx_internal_index, _images_files)) {
            _main_system->n_renderer.texture = Resources.get("data/textures/particles/" + _images_files[_tx_internal_index])->as<CTexture>();
        }
        ((CTexture*)(_main_system->n_renderer.texture))->debugInMenu();
        ImGui::DragFloat2("Tiles", &_main_system->n_renderer.frameSize.x, 1.f, 1.f, 100.f);
        ImGui::DragFloat("Frame over Time", &_main_system->n_renderer.frameSpeed, 0.01f, 0.f, 100.f);
        ImGui::DragInt("Start frame", &_main_system->n_renderer.initialFrame, 1, 0, 100);
        ImGui::DragInt("Cycles", &_main_system->n_renderer.numFrames, 1, 1, 100);
    }
}

void ParticlesEditor::debugRender() {

    CEntity * e_main = _main_debug;
    TCompTransform * e_transform = e_main->get<TCompTransform>();
    MAT44 tr = MAT44::CreateTranslation(e_transform->getPosition());
    MAT44 sc = MAT44::CreateScale(e_transform->getScale() *_main_system->n_shape.size);
    MAT44 rt = MAT44::CreateFromQuaternion(e_transform->getRotation());
    MAT44 mat = sc * rt * tr;

    switch (_main_system->n_shape.type)
    {
        case Particles::TCoreSystem::TNShape::Point:
        {            
            MAT44 sc = MAT44::CreateScale(0.1f);
            auto mesh = Resources.get("unit_quad_center_xy.mesh")->as<CRenderMesh>();
            renderMesh(mesh, mat * sc, VEC4(0, 1, 1, 1));
        } break;

        case Particles::TCoreSystem::TNShape::Line:
        {
            auto mesh = Resources.get("line_center.mesh")->as<CRenderMesh>();
            MAT44 sc2 = MAT44::CreateScale(e_transform->getScale() *_main_system->n_shape.size * .5);
            MAT44 rt2 = MAT44::CreateFromQuaternion(e_transform->getRotation() + QUAT::CreateFromAxisAngle(VEC3(0,1,0), deg2rad(90)));
            MAT44 mat2 = sc2 * rt2 * tr;
            renderMesh(mesh, mat2, VEC4(0, 1, 1, 1));
        } break;

        case Particles::TCoreSystem::TNShape::Square:
        {
            auto mesh = Resources.get("unit_quad_center_xy.mesh")->as<CRenderMesh>();
            renderMesh(mesh, mat, VEC4(0, 1, 1, 1));
        } break;

        case Particles::TCoreSystem::TNShape::Box:
        {
            auto mesh = Resources.get("wired_unit_cube.mesh")->as<CRenderMesh>();
            renderMesh(mesh, mat, VEC4(0, 1, 1, 1));
        } break;

        case Particles::TCoreSystem::TNShape::Sphere:
        {
            auto mesh = Resources.get("sphere.mesh")->as<CRenderMesh>();
            renderMesh(mesh, mat, VEC4(0, 1, 1, 1));
        } break;

        case Particles::TCoreSystem::TNShape::Cone:
        {
            auto mesh = Resources.get("circle_xz.mesh")->as<CRenderMesh>();
            renderMesh(mesh, mat, VEC4(0, 1, 1, 1));
        } break;

        case Particles::TCoreSystem::TNShape::Circle:
        {
            auto mesh = Resources.get("circle_xz.mesh")->as<CRenderMesh>();
            renderMesh(mesh, mat, VEC4(0, 1, 1, 1));
        } break;
    }
}

void ParticlesEditor::saveParticleSystem() {

    if (_main_debug.isValid()) {

        CEntity * e_main = _main_debug;
        TCompParticles * particles = e_main->get<TCompParticles>();

        Particles::CParser parser = Particles::CParser();
        parser.writeFile(particles->_core);
    }
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

    TCompTransform * c_e_transform = e->get<TCompTransform>();
    c_e_transform->setPosition(VEC3(0, 1, 0));

    std::vector<TTrack<VEC3>::TKeyframe> sizes = _main_system->n_size.sizes.getKeyframes();
    for (unsigned int i = 0; i < sizes.size(); i++)
        _values_size[i] = ImVec2(sizes[i].time, sizes[i].value.x);

    std::vector<TTrack<VEC3>::TKeyframe> velocity = _main_system->n_velocity.velocity.getKeyframes();
    for (unsigned int i = 0; i < velocity.size(); i++)
        _values_velocity[i] = ImVec2(velocity[i].time, velocity[i].value.x);
}

Particles::TCoreSystem::TNShape::EType ParticlesEditor::mappings(int index) {

    switch (index)
    {
        case Particles::TCoreSystem::TNShape::Point:
        {
            return Particles::TCoreSystem::TNShape::Point;
        } break;

        case Particles::TCoreSystem::TNShape::Line:
        {
            return Particles::TCoreSystem::TNShape::Line;
        } break;

        case Particles::TCoreSystem::TNShape::Square:
        {
            return Particles::TCoreSystem::TNShape::Square;
        } break;

        case Particles::TCoreSystem::TNShape::Box:
        {
            return Particles::TCoreSystem::TNShape::Box;
        } break;

        case Particles::TCoreSystem::TNShape::Sphere:
        {
            return Particles::TCoreSystem::TNShape::Sphere;
        } break;

        case Particles::TCoreSystem::TNShape::Circle:
        {
            return Particles::TCoreSystem::TNShape::Circle;
        } break;

        case Particles::TCoreSystem::TNShape::Cone:
        {
            return Particles::TCoreSystem::TNShape::Cone;
        } break;
    }

    return Particles::TCoreSystem::TNShape::Box;
}

void ParticlesEditor::LoadTextureFromSource(const std::string & name) {


}