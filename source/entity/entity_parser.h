#pragma once

// This will store the current loading process of a single file
struct TEntityParseContext {

	// To be used when one file dispatches the loading of other scenes
	// or prefabs
	TEntityParseContext* parent = nullptr;

	// In case our prefab creates other prefabs which creates...
	int                  recursion_level = 0;

	// The current filename being loaded
	std::string          filename;

	// An array of the handles currenty registered 
	VHandles             entities_loaded;

	// In case one entity has requested parsing this file
	CHandle              entity_starting_the_parse;

	// Current entity being parsed. Makes sense only when parsing components of that entity
	CHandle              current_entity;

	// Absolute transform where all instances should be placed relative to
	// Make sense for the comp_transform
	CTransform           root_transform;

	// Find an entity in the list of entities parsed in this file
	// or search in the parent contexts, or search in the global dict
	CHandle findEntityByName(const std::string& name) const;

	// True if the entity is a prefab
	bool is_prefab = false;

	TEntityParseContext() = default;
	TEntityParseContext(TEntityParseContext& another, const CTransform& delta_transform);
};

bool parseScene(const std::string& filename, TEntityParseContext& ctx);

