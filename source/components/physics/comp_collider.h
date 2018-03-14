#pragma once

#include "components/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"
#include "render/mesh/mesh_loader.h"

// TO-DO After milestone1

// ----------------------------------------------
class ColliderMeshResourceClass : public CResourceClass {
public:
	ColliderMeshResourceClass() {
	class_name = "olliders";
	extensions = { ".collider" };
	}
	IResource* create(const std::string& name) const override {
	dbg("Creating mesh %s\n", name.c_str());
	TMeshLoader* res = loadCollider(name.c_str());
	return res;
	}
};

// A specialization of the template defined at the top of this file
// If someone class getResourceClassOf<CTexture>, use this function:
template<>
	const CResourceClass* getResourceClassOf<CRenderMesh>() {
	static ColliderMeshResourceClass the_resource_class;
	return &the_resource_class;
}

struct TColliderConfig {

	bool is_trigger;

	physx::PxVec3 center;
	physx::PxActor* actor;
	physx::PxMaterial* gMaterial;

};

struct TBoxColliderConfig : public TColliderConfig
{
	physx::PxVec3 size;
};

struct TSphereColliderConfig : public TColliderConfig
{
	float radius;
};

struct TCapsuleColliderConfig : public TColliderConfig
{
	float height;
	float radius;
	physx::PxVec3 direction;
};

struct TConvexColliderConfig : public TColliderConfig
{
	float height;
	float radius;
	physx::PxVec3 direction;
};


struct TMeshColliderConfig : public TColliderConfig
{
	float height;
	float radius;
	physx::PxVec3 direction;
};

class TCompCollider : public TCompBase {
	
	VEC3 lastFramePosition;
	VEC3 normal_gravity = VEC3(0, -9.8f, 0);
	std::map<TCompTransform, CHandle> ownHandles;

	physx::PxVec3 velocity = physx::PxVec3(0, 0, 0);
	physx::PxVec3 totalDownForce = physx::PxVec3(0, 0, 0);

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);
	void onTriggerEnter(const TMsgTriggerEnter& msg);
	void onTriggerExit(const TMsgTriggerExit& msg);

	DECL_SIBLING_ACCESS();

public:

	// Collider parameter description
	TColliderConfig * config;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	void Resize(float new_size);
	void SetUpVector(VEC3 new_up);

	const VEC3 & GetUpVector(void) const;
	bool OnTrigger(const CHandle & entity) const;

	static void registerMsgs();
};