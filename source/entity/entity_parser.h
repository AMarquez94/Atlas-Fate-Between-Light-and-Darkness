#pragma once

// This will store the current loading process of a single file
struct TEntityParseContext {

  // To be used when one file dispatches the loading of other scenes
  // or prefabs
  TEntityParseContext* parent = nullptr;

  // The current filename being loaded
  std::string          filename;

  // An array of the handles currenty registered 
  VHandles             entities_loaded;

  CHandle              current_entity;

  // Find an entity in the list of entities parsed in this file
  // or search in the parent contexts, or search in the global dict
  CHandle findEntityByName(const std::string& name) const;
};

bool parseScene(const std::string& filename, TEntityParseContext& ctx);

