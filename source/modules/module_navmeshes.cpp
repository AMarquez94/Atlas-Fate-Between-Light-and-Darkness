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
  navmesh.destroy();
  navmeshCreated = false;
  return true;
}

void CModuleNavmesh::update(float delta)
{
  if (EngineInput["btDebugNavmesh"].getsPressed()) {
    buildNavmesh();
    navmeshCreated = true;
  }


}

void CModuleNavmesh::render()
{
  navmesh.render(false);
}

void CModuleNavmesh::buildNavmesh()
{
  /* TODO: add our config */
  navmesh.m_input.clearInput();
  navmesh.build();
}