#include "mcv_platform.h"
#include "entity/entity_parser.h"
#include "comp_bullet_controller.h"
#include "../comp_transform.h"
#include "../comp_tags.h"

DECL_OBJ_MANAGER("bullet_controller", TCompBulletController);

void TCompBulletController::debugInMenu() {
	ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 20.f);
}

void TCompBulletController::onAssignBulletOwner(const TMsgAssignBulletOwner& msg) {
	// Save who generated me
	h_sender = msg.h_owner;

	// Copy his position/orientation into my position/orientation
	TCompTransform *c_my_tmx = get<TCompTransform>();
	CEntity* e_sender = h_sender;
	TCompTransform *c_his_tmx = e_sender->get<TCompTransform>();
	*c_my_tmx = *c_his_tmx;
}

void TCompBulletController::registerMsgs() {
	DECL_MSG(TCompBulletController, TMsgAssignBulletOwner, onAssignBulletOwner);
}

void TCompBulletController::load(const json& j, TEntityParseContext& ctx) {
	h_sender = ctx.entity_starting_the_parse;
	speed = j.value("speed", 1.0f);
}

void TCompBulletController::update(float dt) {

	TCompTransform *c_my_transform = get<TCompTransform>();
	if (!c_my_transform)
		return;

	// Advance
	float amount_moved = dt * speed;
	c_my_transform->setPosition(c_my_transform->getPosition() + c_my_transform->getFront() * amount_moved);

	// ---------------------
	// Check for collisions 
	// This will store the handle close enough to me
	CHandle h_near;

	// Get all entities with the tag 'enemy'
	auto& handles = CTagsManager::get().getAllEntitiesByTag(getID("enemy"));
	for (auto h : handles) {
		// Get the entity
		CEntity* e_candidate = h;
		if (!e_candidate)
			continue;
		// Get his transform
		TCompTransform* c_candidate_transform = e_candidate->get<TCompTransform>();
		assert(c_candidate_transform);

		// Find if it's close enough
		float distance_to_c = VEC3::Distance(c_my_transform->getPosition(), c_candidate_transform->getPosition());
		if (distance_to_c < this->collision_radius)
			h_near = h;
	}

	// 
	if (h_near.isValid()) {
		CEntity *e_near = h_near;

		dbg("Entity %s has been hit\n", e_near->getName());

		// Notify the entity that he has been hit
		TMsgDamage msg;
		msg.h_sender = h_sender;      // Who send this bullet
		msg.h_bullet = CHandle(this); // The bullet information
		e_near->sendMsg(msg);

		// Queue my owner entity as 'destroyed'
		CHandle(this).getOwner().destroy();
	}

}

