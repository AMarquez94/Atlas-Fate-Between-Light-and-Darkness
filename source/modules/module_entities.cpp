#include "mcv_platform.h"
#include "module_entities.h"
#include "render/render_objects.h"
#include "render/texture/texture.h"
#include "render/texture/material.h"
#include "entity/entity.h"
#include "components/comp_render.h"
#include "components/comp_transform.h"
#include "components/comp_name.h"
#include "components/comp_tags.h"
#include "components/comp_light.h"
#include "render/render_manager.h"

void CModuleEntities::loadListOfManagers(const json& j, std::vector< CHandleManager* > &managers) {
	managers.clear();
	// For each entry in j["update"] add entry to om_to_update 
	std::vector< std::string > names = j;
	for (auto& n : names) {
		auto om = CHandleManager::getByName(n.c_str());
		assert(om || fatal("Can't find a manager of components of type %s to update. Check file components.json\n", n.c_str()));
		managers.push_back(om);
	}
}
bool CModuleEntities::start()
{
  json j = loadJson("data/components.json");
  
  // Initialize the ObjManager preregistered in their constructors
  // with the amount of components defined in the data/components.json
  std::map< std::string, int > comp_sizes = j["sizes"];;
  int default_size = comp_sizes["default"];

  // Reorder the init manager based on the json
  // The bigger the number in the init_order section, the lower comp_type id you get
  std::map< std::string, int > init_order = j["init_order"];;
  std::sort( CHandleManager::predefined_managers
           , CHandleManager::predefined_managers + CHandleManager::npredefined_managers
    , [&init_order](CHandleManager* m1, CHandleManager* m2) {
    int priority_m1 = init_order[m1->getName()];
    int priority_m2 = init_order[m2->getName()];
    return priority_m1 > priority_m2;
  });
  // Important that the entity is the first one for the chain destruction of components
  assert(strcmp(CHandleManager::predefined_managers[0]->getName(), "entity") == 0);

  // Now with the sorted array
  for (size_t i = 0; i < CHandleManager::npredefined_managers; ++i) {
    auto om = CHandleManager::predefined_managers[i];
    auto it = comp_sizes.find(om->getName());
    int sz = (it == comp_sizes.end()) ? default_size : it->second;
    dbg("Initializing obj manager %s with %d\n", om->getName(), sz);
    om->init(sz, false);
  }
  loadListOfManagers(j["update"], om_to_update);
  loadListOfManagers(j["render_debug"], om_to_render_debug);

  return true;
}

void CModuleEntities::update(float delta)
{
	for (auto om : om_to_update)
	{
		PROFILE_FUNCTION(om->getName());
		om->updateAll(delta);
	}

	CHandleManager::destroyAllPendingObjects();
}

bool CModuleEntities::stop() {
	// Destroy all entities, should destroy all components in chain
	auto hm = getObjectManager<CEntity>();
	hm->forEach([](CEntity* e) {
		CHandle h(e);
		h.destroy();
	});
	CHandleManager::destroyAllPendingObjects();
	return true;
}

void CModuleEntities::render()
{
  Resources.debugInMenu();

  if (ImGui::TreeNode("All Entities...")) {

	  ImGui::SameLine();
	  static bool flat = false;
	  ImGui::Checkbox("Flat", &flat);

	  static ImGuiTextFilter Filter;
	  ImGui::SameLine();
	  Filter.Draw("Filter");

	  auto om = getObjectManager<CEntity>();
	  om->forEach([](CEntity* e) {
		  CHandle h_e(e);
		  if (!flat && h_e.getOwner().isValid())
			  return;
		  if (Filter.IsActive() && !Filter.PassFilter(e->getName()))
			  return;
		  ImGui::PushID(e);
		  e->debugInMenu();
		  ImGui::PopID();
	  });
	  ImGui::TreePop();
  }

  if (ImGui::TreeNode("All Components...")) {
    for (uint32_t i = 1; i < CHandleManager::getNumDefinedTypes(); ++i)
      CHandleManager::getByType(i)->debugInMenuAll();
    ImGui::TreePop();
  }

  CTagsManager::get().debugInMenu();

  // I just need to activate one light... but at this moment...	
  getObjectManager<TCompLight>()->forEach([](TCompLight* c) {
	c->activate();
  });

  CRenderManager::get().renderCategory("default");
  CRenderManager::get().debugInMenu();

  // Change the technique to some debug solid
  auto solid = Resources.get("data/materials/solid.material")->as<CMaterial>();
  solid->activate();
  for (auto om : om_to_render_debug)
	  om->renderDebugAll();
}

void CModuleEntities::renderDebugOfComponents() {
	PROFILE_FUNCTION("renderDebugOfComponents");
	// Change the technique to some debug solid
	auto solid = Resources.get("data/materials/solid.material")->as<CMaterial>();
	solid->activate();
	for (auto om : om_to_render_debug)
		for (auto om : om_to_render_debug) {
			PROFILE_FUNCTION(om->getName());
			om->renderDebugAll();
		}
}

void CModuleEntities::destroyAllEntities() {
	auto om = getObjectManager<CEntity>();
	om->forEach([](CEntity* e) {
		CHandle h_e(e);
		h_e.destroy();
	});
	CHandleManager::destroyAllPendingObjects();
}