#include "mcv_platform.h"
#include "module_navmeshes.h"


bool CModuleNavmesh::start()
{
  navmesh = CNavmesh();
  //navmeshQuery = CNavmeshQuery(&navmesh);
  navmeshCreated = false;
  return true;
}

bool CModuleNavmesh::stop()
{
  destroyNavmesh();
  return true;
}

void CModuleNavmesh::update(float delta)
{

}

void CModuleNavmesh::render()
{
  navmesh.render(false);
}

void CModuleNavmesh::buildNavmesh(const std::string& path)
{
  /* TODO: add our config */
  navmesh.m_input.clearInput();
  navmesh.loadAll(path.c_str());
  if (navmesh.m_navMesh) {
    navmeshCreated = true;
    navmesh.prepareQueries();
  }
  //navmesh.build();
}

void CModuleNavmesh::destroyNavmesh() {
  navmesh.destroy();
  navmeshCreated = false;
}

const std::vector<VEC3> CModuleNavmesh::findPath(VEC3 start, VEC3 end) {
  return navmeshQuery.findPath(start, end);
}