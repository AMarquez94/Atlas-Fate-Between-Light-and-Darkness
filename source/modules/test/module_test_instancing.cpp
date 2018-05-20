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

    auto rmesh = Resources.get("data/meshes/GeoSphere001.instanced_mesh")->as<CRenderMesh>();
    // Remove cast and upcast to CRenderMeshInstanced
    instances_mesh = (CRenderMeshInstanced*)rmesh;
    return true;
}

void CModuleTestInstancing::update(float delta) {

    // Move the instances in the cpu
    static float t = 0;
    t += delta;
    for (auto& p : instances)
        p.world = p.world * MAT44::CreateTranslation(VEC3(0, 0.1f * sin(t), 0));
    instances_mesh->setInstancesData(instances.data(), instances.size(), sizeof(TInstance));

}

void CModuleTestInstancing::render() {

    if (ImGui::TreeNode("Test Instancing")) {
        ImGui::Text("Num Instances: %ld / %ld. GPU:%d", instances.size(), instances.capacity(), instances_mesh->getVertexsCount());
        static float sz = 50.f;
        static int num = 3;
        ImGui::DragFloat("Size", &sz, 0.01f, -50.f, 50.f);
        ImGui::DragInt("Num", &num, 0.1f, 1, 10);
        if (ImGui::Button("Add")) {
            for (int i = 0; i < num; ++i) {
                TInstance new_instance;
                new_instance.world = MAT44::CreateTranslation(VEC3(randomFloat(-sz, sz), 0, randomFloat(-sz, sz)));
                instances.push_back(new_instance);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Del") && !instances.empty()) {
            instances.pop_back();
        }
        ImGui::TreePop();
    }
}