#pragma once

#include "comp_base.h"

class CRenderMesh;
class CTexture;
class CRenderTechnique;
class CMaterial;

class TCompRender : public TCompBase {
public:
	VEC4               color = VEC4(1, 1, 1, 1);

	// This represents a single object mesh with several materials. Not multiples meshes
	const CRenderMesh* mesh = nullptr;
	std::vector<const CMaterial*> materials;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void loadMesh(const json& j, TEntityParseContext& ctx);

	DECL_SIBLING_ACCESS();
};