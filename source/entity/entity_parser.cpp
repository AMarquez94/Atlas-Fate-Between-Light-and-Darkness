#include "mcv_platform.h"
#include "entity_parser.h"
#include "entity.h"
#include "resources/json_resource.h"

// Find in the current list of entities created, the first entity matching
// the given name
CHandle TEntityParseContext::findEntityByName(const std::string& name) const {

  // Search linearly in the list of entity currently loaded
  for (auto h : entities_loaded) {
    CEntity* e = h;
    if (e->getName() == name )
      return h;
  }

  // Delegate it to my parent
  if (parent)
    return parent->findEntityByName(name);

  return getEntityByName(name);
}

// 
bool parseScene(const std::string& filename, TEntityParseContext& ctx) {

  ctx.filename = filename;

  const json& j_scene = Resources.get(filename)->as<CJsonResource>()->getJson();
  assert(j_scene.is_array());

  // For each item in the array...
  for (int i = 0; i < j_scene.size(); ++i ) {
	  auto& j_item = j_scene[i];

    assert(j_item.is_object());
    if (j_item.count("entity")) {
		auto& j_entity = j_item["entity"];

      // Create a new fresh entity
      CHandle h_e;
      h_e.create< CEntity >();
      
      // Cast to entity object
      CEntity* e = h_e;

      // Do the parse
      e->load(j_entity, ctx);

      ctx.entities_loaded.push_back(h_e);
    }

  }

  // Notify each entity created that we have finished
  // processing this file
  for (auto h : ctx.entities_loaded) {
    
  }


  return true;
}