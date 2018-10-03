#pragma once

#include "resource.h"
#include <mutex>

class CResourceManager {

  // I'm the owner of all resources, and are stored in the map using the resource.name
  std::map< std::string, IResource* > all_resources;
  
  // These are the 'resource classes' stored by extension
  std::map< std::string, const CResourceClass* > resource_classes;

  // To make the reload in proper order
  std::vector< const CResourceClass* > resource_classes_by_file_change_priority;

  std::vector<std::string> pending_resources;

  std::mutex pending_resources_mutex;
  std::recursive_mutex resources_mutex;
  std::condition_variable condition_variable;


public:

  void registerResourceClass(const CResourceClass* new_class);
  const IResource* get(const std::string& res_name);

  void debugInMenu();
  void destroyAll();

  // 
  void registerResource( IResource* res );
  void onFileChanged(const std::string& filename);

  void addPendingResource(const std::string& pendingResource);

  const std::string getFirstPendingResource();
  const std::string getResourceName(const std::string& resourcePath);

};

extern CResourceManager Resources;

