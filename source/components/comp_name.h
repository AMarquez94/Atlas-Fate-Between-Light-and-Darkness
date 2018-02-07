#pragma once

#include "comp_base.h"
#include <unordered_map>
 
class TCompName : public TCompBase {
  static const size_t max_size = 64;
  char name[max_size];

public:

  static std::unordered_map< std::string, CHandle > all_names;
  const char* getName() const { return name; }
  void setName(const char* new_name);

  void debugInMenu();
  void load(const json& j, TEntityParseContext& ctx);

};