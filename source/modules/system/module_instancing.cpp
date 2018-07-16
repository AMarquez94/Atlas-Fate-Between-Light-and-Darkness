#include "mcv_platform.h"
#include "module_instancing.h"
#include "geometry/transform.h"
#include "entity/entity_parser.h"
#include "components/comp_name.h"
#include "components/comp_group.h"
#include "components/comp_transform.h"
#include "render/compute/compute_task.h"

static CComputeTask compute_task;
static CTexture*    compute_result;

float unitRandom() {

    return (float)rand() / (float)RAND_MAX;
}

float randomFloat(float vmin, float vmax) {

    return vmin + (vmax - vmin) * unitRandom();
}

VEC3 randomVEC3(VEC3 vmin, VEC3 vmax) {
    VEC3 v;
    v.x = randomFloat(vmin.x, vmax.x);
    v.y = randomFloat(vmin.y, vmax.y);
    v.z = randomFloat(vmin.z, vmax.z);
    return v;
}

bool CModuleInstancing::stop()
{
    scene_group.destroy();

    return true;
}

bool CModuleInstancing::start() {

    // Load static meshes
    {
        auto rmesh = Resources.get("data/meshes/GeoSphere001.instanced_mesh")->as<CRenderMesh>();
        // Remove cast and upcast to CRenderMeshInstanced
        instances_mesh = (CRenderMeshInstanced*)rmesh;
    }
    {
        auto rmesh = Resources.get("data/meshes/blood.instanced_mesh")->as<CRenderMesh>();
        blood_instances_mesh = (CRenderMeshInstanced*)rmesh;
    }
    {
        auto rmesh = Resources.get("data/meshes/particles.instanced_mesh")->as<CRenderMesh>();
        particles_instances_mesh = (CRenderMeshInstanced*)rmesh;
    }
    {
        auto rmesh = Resources.get("data/meshes/grass.instanced_mesh")->as<CRenderMesh>();
        grass_instances_mesh = (CRenderMeshInstanced*)rmesh;
        grass_instances_mesh->setInstancesData(grass_instances.data(), grass_instances.size(), sizeof(TGrassParticle));
    }

    // Create a scene entity to hold all global instances.
    {
        scene_group.create< CEntity >();
        CEntity* e = scene_group;

        CHandle h_comp = getObjectManager<TCompTransform>()->createHandle();
        e->set(h_comp.getType(), h_comp);

        h_comp = getObjectManager<TCompName>()->createHandle();
        e->set(h_comp.getType(), h_comp);

        TCompName * c_name = e->get<TCompName>();
        c_name->setName("Instanced Meshes");

        CHandle h_group = getObjectManager<TCompGroup>()->createHandle();
        e->set(h_group.getType(), h_group);
    }

    {
        //compute_result = new CTexture();
        //compute_result->setNameAndClass("output_bw.dds", getResourceClassOf<CTexture>());
        //if (!compute_result->create(256, 256, DXGI_FORMAT_R8G8B8A8_UNORM, CTexture::CREATE_COMPUTE_OUTPUT))
        //    return false;

        //json j = loadJson("data/compute_task.json");
        //if (!compute_task.create(j))
        //    return false;
        // Missing
        // compute_task.destroy();
    }

    return true;
}

/* Load the global instance mesh in the scene */
bool CModuleInstancing::parseInstance(const json& j, TEntityParseContext& ctx) {

    std::string name = j.value("mesh", "data/meshes/GeoSphere001.instanced_mesh");

    if (_global_names.find(name) == _global_names.end()) {

        CHandle h_e;
        h_e.create< CEntity >();
        CEntity* e = h_e;

        TEntityParseContext ctx_temp;
        ctx_temp.current_entity = e;
        // Bind it to me
        auto om = CHandleManager::getByName("render");
        CHandle h_comp = om->createHandle();

        h_comp.load(j, ctx_temp);
        e->set(h_comp.getType(), h_comp);

        h_comp = getObjectManager<TCompTransform>()->createHandle();
        e->set(h_comp.getType(), h_comp);

        h_comp = getObjectManager<TCompName>()->createHandle();
        e->set(h_comp.getType(), h_comp);

        std::size_t pos = name.find("meshes/") + 7;
        std::string sub_name = name.substr(pos);
        TCompName * c_name = e->get<TCompName>();
        c_name->setName(sub_name.c_str());

        if (scene_group.isValid()) {
            CEntity * t_group = scene_group;
            TCompGroup* c_group = t_group->get<TCompGroup>();
            c_group->add(h_e);
        }

        _global_names.insert(std::pair<std::string, std::string>(name, name));
    }

    return true;
}

void CModuleInstancing::update(float delta) {

    // Rotate the particles
    for (auto& p : particles_instances)
        p.angle += delta;

    // Move the instances in the cpu
    static float t = 0;
    t += delta;

    for (auto& p : instances)
        p.world = p.world * MAT44::CreateTranslation(VEC3(0, 0.1f * sin(t), 0));

    // Handle the dynamic instances
    //for (auto p : _dynamic_instances)
    //    for (auto q : p.second)
    //            q->world = q->world * _dynamic_transform[q]->asMatrix();
}

int CModuleInstancing::addInstance(const std::string & name, MAT44 w_matrix) {

    // Add the instance collector if it's not in our database
    if (_global_instances.find(name) == _global_instances.end()) {

        auto rmesh = (CRenderMeshInstanced*)Resources.get(name)->as<CRenderMesh>();
        TInstanceCollector collector;
        collector._instances_mesh = rmesh;
        _global_instances.insert(std::pair<std::string, TInstanceCollector>(name, collector));

        // Add the generic group entity into the scene dynamically?
    }

    // Create the new instance and add it to the set
    TInstance static_instance;
    static_instance.world = w_matrix;
    _global_instances[name]._instances.push_back(static_instance);

    return _global_instances[name]._instances.size() - 1;
}

void CModuleInstancing::removeInstance(TInstance* instance) {

    // Loop through and remove instance from vector.
}

void CModuleInstancing::clearInstances() {

    _global_instances.clear();
}

void CModuleInstancing::clearInstance(const std::string &name) {

    if (_global_instances.find(name) != _global_instances.end()) {
        _global_instances[name]._instances.clear();
    }
}

// Maybe we should refactor this with pointers..
void CModuleInstancing::updateInstance(const std::string& name, int index, const MAT44& w_matrix) {

    _global_instances[name]._instances[index].world = w_matrix;
}

void CModuleInstancing::renderMain() {

    for (auto p : _global_instances)
        p.second._instances_mesh->setInstancesData(p.second._instances.data(), p.second._instances.size(), sizeof(TInstance));

    instances_mesh->setInstancesData(instances.data(), instances.size(), sizeof(TInstance));
    blood_instances_mesh->setInstancesData(blood_instances.data(), blood_instances.size(), sizeof(TInstanceBlood));
    particles_instances_mesh->setInstancesData(particles_instances.data(), particles_instances.size(), sizeof(TRenderParticle));

    //compute_task.debugInMenu();
    //compute_task.run();
}

void CModuleInstancing::render() {

    if (ImGui::TreeNode("Instance Manager")) {

        // -- Creation params ----------------------
        static float sz = 50.f;
        static int num = 3;
        ImGui::DragFloat("Size", &sz, 0.01f, -50.f, 50.f);
        ImGui::DragInt("Num", &num, 0.1f, 1, 10);

        // ----------------------------------------------
        if (ImGui::TreeNode("Particles")) {
            ImGui::Text("Num Instances: %ld / %ld. GPU:%d", particles_instances.size(), particles_instances.capacity(), particles_instances_mesh->getVertexsCount());

            if (ImGui::Button("Add")) {
                for (int i = 0; i < num; ++i) {
                    TRenderParticle new_instance;
                    new_instance.scale_x = randomFloat(1.f, 5.f);
                    new_instance.scale_y = new_instance.scale_x;
                    new_instance.pos = VEC3(randomFloat(-sz, sz), new_instance.scale_x, randomFloat(-sz, sz));
                    new_instance.nframe = randomFloat(0.f, 16.f);
                    new_instance.angle = deg2rad(randomFloat(0, 360));
                    new_instance.color.x = unitRandom();
                    new_instance.color.y = unitRandom();
                    new_instance.color.z = 1 - new_instance.color.x - new_instance.color.y;
                    new_instance.color.w = 1;
                    particles_instances.push_back(new_instance);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Del") && !particles_instances.empty())
                particles_instances.pop_back();
            ImGui::TreePop();
        }

        // ----------------------------------------------
        if (ImGui::TreeNode("Spheres")) {
            ImGui::Text("Num Instances: %ld / %ld. GPU:%d", instances.size(), instances.capacity(), instances_mesh->getVertexsCount());

            if (ImGui::Button("Add")) {
                for (int i = 0; i < num; ++i) {
                    TInstance new_instance;
                    new_instance.world = MAT44::CreateTranslation(VEC3(randomFloat(-sz, sz), 0, randomFloat(-sz, sz)));
                    instances.push_back(new_instance);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Del") && !instances.empty())
                instances.pop_back();
            ImGui::TreePop();
        }

        // ----------------------------------------------
        if (ImGui::TreeNode("Blood")) {
            ImGui::Text("Num Instances: %ld / %ld. GPU:%d", blood_instances.size(), blood_instances.capacity(), blood_instances_mesh->getVertexsCount());
            if (ImGui::Button("Add")) {
                for (int i = 0; i < num; ++i) {
                    TInstanceBlood new_instance;
                    new_instance.world =
                        MAT44::CreateRotationY(randomFloat((float)-M_PI, (float)M_PI))
                        *
                        MAT44::CreateScale(randomFloat(2.f, 10.f))
                        *
                        MAT44::CreateTranslation(VEC3(randomFloat(-sz, sz), 0, randomFloat(-sz, sz)));
                    new_instance.color.x = unitRandom();
                    new_instance.color.y = unitRandom();
                    new_instance.color.z = 1 - new_instance.color.x - new_instance.color.y;
                    new_instance.color.w = 1;
                    blood_instances.push_back(new_instance);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Del") && !blood_instances.empty())
                blood_instances.pop_back();
            if (ImGui::TreeNode("Instances")) {
                for (auto& p : blood_instances) {
                    ImGui::PushID(&p);
                    VEC3 scale, trans;
                    QUAT rot;
                    p.world.Decompose(scale, rot, trans);
                    CTransform tmx;
                    tmx.setRotation(rot);
                    tmx.setPosition(trans);
                    tmx.setScale(scale.x);
                    if (tmx.debugInMenu())
                        p.world = tmx.asMatrix();
                    ImGui::ColorEdit4("Color", &p.color.x);
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        // ----------------------------------------------
        if (ImGui::TreeNode("Grass")) {
            bool changed = false;
            ImGui::Text("Num Instances: %ld / %ld. GPU:%d", grass_instances.size(), grass_instances.capacity(), grass_instances_mesh->getVertexsCount());
            int num_changed = num * 100;
            if (ImGui::Button("Add 100")) {
                for (int i = 0; i < num_changed; ++i) {
                    TGrassParticle new_instance;
                    new_instance.pos = VEC3(randomFloat(-sz, sz), 0, randomFloat(-sz, sz));
                    grass_instances.push_back(new_instance);
                }
                changed = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Del") && !instances.empty()) {
                if (num_changed < instances.size())
                    num_changed = instances.size();
                instances.resize(instances.size() - num_changed);
                changed = true;
            }
            ImGui::TreePop();

            // Update GPU with the new CPU
            if (changed)
                grass_instances_mesh->setInstancesData(grass_instances.data(), grass_instances.size(), sizeof(TGrassParticle));
        }

        ImGui::TreePop();
    }
}