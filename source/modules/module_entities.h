#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class CModuleEntities : public IModule
{
  float time_scale_factor = 1.f;
  std::vector< CHandleManager* > om_to_update;
  std::vector< CHandleManager* > om_to_render_debug;
  void loadListOfManagers(const json& j, std::vector< CHandleManager* > &managers);
  void renderDebugOfComponents();

public:
  CModuleEntities(const std::string& aname) : IModule(aname) { }
  bool start();
  void update(float delta) override;
  void render() override;
  void destroyAllEntities();
};

CHandle getEntityByName(const std::string& name);
