#include "mcv_platform.h"
#include "module_navmeshes.h"


bool CModuleNavmesh::start() {
    navmesh = CNavmesh();
    navmeshCreated = false;
    return true;
}

bool CModuleNavmesh::stop() {
    destroyNavmesh();
    navmeshesCache.clear();
    return true;
}

void CModuleNavmesh::update(float delta) {

}

void CModuleNavmesh::render() {
    if (renderNamvesh) {
        navmesh.render();
    }
    if (ImGui::TreeNode("Navmesh"))
    {
        ImGui::Text("Navmesh cache %d", navmeshesCache.size());
        ImGui::TreePop();
    }
    //ImGui::DragFloat2("Nearest Poly Extents", &navmeshQuery.nearestPolyExtents.x, 0.5f, 0.0f, 100.f);
}

void CModuleNavmesh::buildNavmesh(const std::string& path) {
    std::vector<char> navmeshData;
    auto it_navmesh = navmeshesCache.find(path);
    if (it_navmesh == navmeshesCache.end()) {
        navmeshData = EngineFiles.loadResourceFile(path);
        navmeshesCache[path] = navmeshData;
        EngineFiles.addPendingResourceFile(path, false);
    }
    else {
        navmeshData = it_navmesh->second;
    }

    navmesh.load(navmeshData);
    if (navmesh.m_navMesh) {
        navmeshCreated = true;
        navmesh.prepareQueries();
    }
    else {
        fatal("Error when creating navmesh\n");
    }
}

void CModuleNavmesh::destroyNavmesh() {
    navmesh.destroy();
    navmeshCreated = false;
}

const std::vector<VEC3> CModuleNavmesh::findPath(VEC3 start, VEC3 end) {
    if (navmeshCreated) {
        return navmeshQuery.findPath(start, end);
    }
    else {
        return std::vector<VEC3>();
    }
}

float CModuleNavmesh::wallDistance(VEC3 pos) {
    if (navmeshCreated) {
        return navmeshQuery.wallDistance(pos);
    }
    else {
        return -1.f;
    }
}

bool CModuleNavmesh::raycast(VEC3 start, VEC3 end, VEC3& hitPos) {
    
    if (navmeshCreated) {
        return navmeshQuery.raycast(start, end, hitPos);
    }
    else {
        return false;
    }
}

VEC3 CModuleNavmesh::closestNavmeshPoint(VEC3 start) {
    if (navmeshCreated) {
        return navmeshQuery.closestNavmeshPoint(start);
    }
    else {
        return VEC3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    }
}

float CModuleNavmesh::navmeshLong(const std::vector<VEC3> navmeshPath)
{
    float navmeshLong = 0.f;
    if (navmeshPath.size() > 1) {
        for (int i = 1; i < navmeshPath.size(); i++) {
            navmeshLong += VEC3::Distance2D(navmeshPath[i], navmeshPath[i - 1]);
        }
    }
    return navmeshLong;
}
