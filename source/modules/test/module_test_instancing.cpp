#include "mcv_platform.h"
#include "module_test_instancing.h"
#include "geometry/transform.h"

float unitRandom() {

    return (float)rand() / (float)RAND_MAX;
}

float randomFloat(float vmin, float vmax) {

    return vmin + (vmax - vmin) * unitRandom();
}

bool CModuleTestInstancing::start() {

    {
        auto rmesh = Resources.get("data/meshes/GeoSphere001.instanced_mesh")->as<CRenderMesh>();
        // Remove cast and upcast to CRenderMeshInstanced
        instances_mesh = (CRenderMeshInstanced*)rmesh;
    }
    {
        auto rmesh = Resources.get("data/meshes/blood.instanced_mesh")->as<CRenderMesh>();
        blood_instances_mesh = (CRenderMeshInstanced*)rmesh;
    }

    return true;
}

void CModuleTestInstancing::update(float delta) {

    // Move the instances in the cpu
    static float t = 0;
    t += delta;
    for (auto& p : instances)
        p.world = p.world * MAT44::CreateTranslation(VEC3(0, 0.1f * sin(t), 0));
    instances_mesh->setInstancesData(instances.data(), instances.size(), sizeof(TInstance));

    blood_instances_mesh->setInstancesData(blood_instances.data(), blood_instances.size(), sizeof(TInstanceBlood));
}

void CModuleTestInstancing::render() {

    if (ImGui::TreeNode("Instancing")) {

        // -- Creation params ----------------------
        static float sz = 50.f;
        static int num = 3;
        ImGui::DragFloat("Size", &sz, 0.01f, -50.f, 50.f);
        ImGui::DragInt("Num", &num, 0.1f, 1, 10);

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
        ImGui::TreePop();
    }
}