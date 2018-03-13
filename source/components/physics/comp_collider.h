#pragma once

#include "components/comp_base.h"
#include "PxPhysicsAPI.h"
#include "entity/common_msgs.h"
#include "render/mesh/mesh_loader.h"

// TO-DO After milestone1
/*
// ----------------------------------------------
class ColliderMeshResourceClass : public CResourceClass {
public:
ColliderMeshResourceClass() {
class_name = "Meshes";
extensions = { ".mesh" };
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
}*/

enum {
	triggerKill = (1 << 0),
	triggerEnemy = (1 << 1),
	triggerCheckpoint = (1 << 2),
	disableFilter = (1 << 3)
};
typedef uint32_t colliderFlags;

class TCompCollider : public TCompBase {

	physx::PxVec3 velocity = physx::PxVec3(0, 0, 0);

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();

public:

	bool isInside;
	bool isGrounded;

	struct TConfig
	{
		colliderFlags flags;

		//todo implement
		VEC3 offset;
		VEC3 halfExtent;
		float radius;
		float height;
		float currentHeight;
		float vOffset;

		unsigned int mask;
		unsigned int group;
		std::string filename;
		physx::PxGeometryType::Enum shapeType;

		bool is_dynamic;
		bool is_trigger;
		bool is_controller;
		bool gravity;
	};

	VEC3 lastFramePosition;

	TConfig config;
	physx::PxActor* actor;
	physx::PxController* controller;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

	static void registerMsgs();
	void onTriggerEnter(const TMsgTriggerEnter& msg);
	void onTriggerExit(const TMsgTriggerExit& msg);

	/* Aux methods, temp */
	void Resize(float new_size);
	void SetUpVector(VEC3 new_up);
	void SetUpVelocity(float velocity);
	VEC3 GetUpVector();

	VEC3 normal_gravity = VEC3(0, -9.8f, 0);
	physx::PxVec3 totalDownForce = physx::PxVec3(0, 0, 0);
	physx::PxControllerFilters filters;

	class CustomQueryFilterCallback : public physx::PxQueryFilterCallback {

	public:
		physx::PxQueryHitType::Enum PxQueryFilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags);
		physx::PxQueryHitType::Enum PxQueryFilterCallback::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) {
			return physx::PxQueryHitType::eTOUCH;
		}
	};

	CustomQueryFilterCallback customQueryFilter;

};