#include "mcv_platform.h"
#include "module_navmeshes.h"


bool CModuleNavmesh::start()
{
  navmesh = CNavmesh();
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
  if (EngineInput["btDebugNavmesh"].getsPressed()) {
    buildNavmesh("data/navmeshes/test.bin");
    navmeshCreated = true;
  }


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
  navmesh.m_draw_mode = CNavmesh::EDrawMode::NAVMESH_DRAW_MESH;
  //navmesh.build();
}

void CModuleNavmesh::destroyNavmesh() {
  navmesh.destroy();
  navmeshCreated = false;
}