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

class TCompCollider: public TCompBase {

	VEC3 velocity = VEC3::Zero;

	void onCreate(const TMsgEntityCreated& msg);
	void onDestroy(const TMsgEntityDestroyed& msg);

	DECL_SIBLING_ACCESS();
	
public:
   
	bool isInside;
	bool isGrounded;

  struct TConfig
  {
    
    //todo implement
    VEC3 offset;
    VEC3 halfExtent;
    float radius;
	float height;
	float currentHeight;

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
  VEC3 GetUpVector();

  VEC3 normal_gravity = VEC3(0, -9.8f, 0);
};