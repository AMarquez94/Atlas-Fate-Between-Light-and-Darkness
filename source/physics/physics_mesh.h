#pragma once

#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"
#include "resources/resource.h"

class TCompTransform;

class CPhysicsMesh : public IResource {

public:
	uint32_t num_vertexs = 0;
	uint32_t num_indices = 0;
	uint32_t bytes_per_vtx = 0;
	uint32_t bytes_per_idx = 0;
	std::vector< uint8_t > vtxs;
	std::vector< uint8_t > idxs;

	bool create(
		  const void* vertex_data
		, const void* index_data
		, uint32_t vertex_size
		, uint32_t index_size
		, uint32_t tbytes_per_vertex
		, uint32_t tbytes_per_idx);

	void destroy() override;
	void debugInMenu() override;
};
