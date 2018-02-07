#pragma once

#include "resource.h"

class CResourceManager {

  // I'm the owner of all resources, and are stored in the map using the resource.name
  std::map< std::string, IResource* > all_resources;
  
  // These are the 'resource classes' stored by extension
  std::map< std::string, const CResourceClass* > resource_classes;

public:

  void registerResourceClass(const CResourceClass* new_class);
  const IResource* get(const std::string& res_name);

  void debugInMenu();
  void destroyAll();

  // 
  void registerResource( IResource* res );

};

extern CResourceManager Resources;

