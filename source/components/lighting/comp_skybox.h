#pragma once

#include "../comp_base.h"
#include "../comp_camera.h"

class CMaterial;
class CRenderMesh;

class TCompSkybox : public TCompBase {

	//const VEC4 color;
	//const CRenderMesh* mesh = nullptr;
	//const CMaterial* material = nullptr;

	DECL_SIBLING_ACCESS();

public:

	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);
	void renderDebug();
	void activate();
};