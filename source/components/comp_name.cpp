#include "mcv_platform.h"
#include "comp_name.h"
#include "handle/handle.h"

DECL_OBJ_MANAGER("name", TCompName);

std::unordered_map< std::string, CHandle > TCompName::all_names;

void TCompName::debugInMenu() {
  ImGui::Text(name);
  ImGui::Text("Handle: %d", CHandle(this).getOwner());
}

void TCompName::setName(const char* new_name) {
  assert(strlen(new_name) < max_size);
  strcpy(name, new_name);

  // Store the handle of the CompName, not the Entity, 
  // because during load, I still don't have an owner
  all_names[name] = CHandle(this);
}
 
void TCompName::load(const json& j, TEntityParseContext& ctx) {
  auto str = j.get<std::string>();
  setName(str.c_str());
}

CHandle getEntityByName(const std::string& name) {

  auto it = TCompName::all_names.find(name);
  if( it == TCompName::all_names.end() )
    return CHandle();

  CHandle h_name = it->second;
  return h_name.getOwner();
}
