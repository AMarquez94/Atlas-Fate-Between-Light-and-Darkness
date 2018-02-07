#include "mcv_platform.h"
#include "module_entities.h"
#include "render/render_objects.h"
#include "render/texture/texture.h"
#include "entity/entity.h"
#include "components/comp_render.h"
#include "components/comp_transform.h"
#include "components/comp_name.h"



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

  // For each entry in j["update"] add entry to om_to_update
  std::vector< std::string > names = j["update"];
  for (auto& n : names) {
    auto om = CHandleManager::getByName(n.c_str());
    assert(om || fatal( "Can't find a manager of components of type %s to update. Check file components.json\n", n.c_str()));
    om_to_update.push_back(om);
  }

  return true;
}

void CModuleEntities::update(float delta)
{
  for (auto om : om_to_update)
    om->updateAll(delta);

  CHandleManager::destroyAllPendingObjects();
}

void CModuleEntities::render()
{
  Resources.debugInMenu();

  auto om = getObjectManager<CEntity>();
  om->debugInMenuAll();

  if (ImGui::TreeNode("All Components...")) {
    for (uint32_t i = 1; i < CHandleManager::getNumDefinedTypes(); ++i)
      CHandleManager::getByType(i)->debugInMenuAll();
    ImGui::TreePop();
  }

  // Do the basic render
  auto om_render = getObjectManager<TCompRender>();
  om_render->forEach([](TCompRender* c) {

    TCompTransform* c_transform = c->get<TCompTransform>();
    if (!c_transform)
      return;

    cb_object.obj_world = c_transform->asMatrix();
    //cb_object.obj_color = e->color
    cb_object.updateGPU();
    if (c->texture)
      c->texture->activate(0);
    c->tech->activate();
    c->mesh->activateAndRender();
  });

}
