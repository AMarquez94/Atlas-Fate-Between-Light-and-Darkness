#pragma once

#include "modules/module.h"
#include "entity/entity.h"

class CModuleEntities : public IModule
{

  std::vector< CHandleManager* > om_to_update;
  std::vector< CHandleManager* > om_to_render_debug;
  void loadListOfManagers(const json& j, std::vector< CHandleManager* > &managers);

public:
  CModuleEntities(const std::string& aname) : IModule(aname) { }
  bool start();
  void update(float delta) override;
  void render() override;
};

CHandle getEntityByName(const std::string& name);
