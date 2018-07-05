#include "mcv_platform.h"
#include "module_navmeshes.h"


bool CModuleNavmesh::start() {
  navmesh = CNavmesh();
  navmeshCreated = false;
  return true;
}

bool CModuleNavmesh::stop() {
  destroyNavmesh();
  return true;
}

void CModuleNavmesh::update(float delta) {

}

void CModuleNavmesh::render() {
    if (renderNamvesh) {
        navmesh.render();
    }
}

void CModuleNavmesh::buildNavmesh(const std::string& path) {
  navmesh.loadAll(path.c_str());
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
  return navmeshQuery.findPath(start, end);
}

float CModuleNavmesh::wallDistance(VEC3 pos) {
  return navmeshQuery.wallDistance(pos);
}

bool CModuleNavmesh::raycast(VEC3 start, VEC3 end, VEC3& hitPos) {
  return navmeshQuery.raycast(start, end, hitPos);
}

VEC3 CModuleNavmesh::closestNavmeshPoint(VEC3 start) {
  return navmeshQuery.closestNavmeshPoint(start);
}