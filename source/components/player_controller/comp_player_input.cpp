#include "mcv_platform.h"
#include "comp_player_input.h"
#include "components/comp_fsm.h"
#include "components/comp_transform.h"
#include "components/player_controller/comp_player_tempcontroller.h"

DECL_OBJ_MANAGER("player_input", TCompPlayerInput);

void TCompPlayerInput::debugInMenu() {
}

void TCompPlayerInput::load(const json& j, TEntityParseContext& ctx) {
}

void TCompPlayerInput::update(float dt)
{
	CEntity* e = CHandle(this).getOwner();
	_time += dt;

	/* Movement messages*/
	{
		if (EngineInput["btUp"].hasChanged()
			|| EngineInput["btDown"].hasChanged()
			|| EngineInput["btLeft"].hasChanged()
			|| EngineInput["btRight"].hasChanged()) {

			TMsgSetFSMVariable walkMsg;
			walkMsg.variant.setName("speed");

			movementValue.x = EngineInput["btRight"].value - EngineInput["btLeft"].value;
			movementValue.y = EngineInput["btUp"].value - EngineInput["btDown"].value;

			float total_value = movementValue.Length();

			walkMsg.variant.setFloat(total_value);
			e->sendMsg(walkMsg);
		}

		/* Speed boost messages */
		TMsgSetFSMVariable boostMsg;

		if (EngineInput["btRun"].hasChanged()) {

      boostMsg.variant.setName("boost_speed");
			crouchButton = false;
			TMsgSetFSMVariable crouch;
			crouch.variant.setName("crouch");
			crouch.variant.setBool(false);
			e->sendMsg(crouch);

			boostMsg.variant.setFloat(EngineInput["btRun"].value);
			e->sendMsg(boostMsg);
		}

		if (EngineInput["btSlow"].hasChanged()) {
      boostMsg.variant.setName("slow_speed");
			boostMsg.variant.setFloat(EngineInput["btSlow"].value);
			e->sendMsg(boostMsg);
		}
	}

	/* Shadow merge messages*/
	{
		if (EngineInput["btShadowMerging"].hasChanged())
		{
			//TMsgSetFSMVariable shadowMerge;
			//shadowMerge.variant.setName("onmerge");
			//shadowMerge.variant.setBool(EngineInput["btShadowMerging"].isPressed());
			//e->sendMsg(shadowMerge);
		}
	}

	/* Player/User interaction messages */
	{

		if (EngineInput["btAttack"].getsPressed())
		{
			TMsgSetFSMVariable attack;
			attack.variant.setName("attack");
			attack.variant.setBool(true);
			e->sendMsg(attack);
			attackButtonJustPressed = true;
		}
		else if (attackButtonJustPressed) {
			TMsgSetFSMVariable attack;
			attack.variant.setName("attack");
			attack.variant.setBool(false);
			e->sendMsg(attack);
			attackButtonJustPressed = false;
		}

		if (EngineInput["btCrouch"].hasChanged())
		{
			// Replace this with triggers contact.
			/*CEntity * c_my_entity = CHandle(this).getOwner();
			TCompCollider * c_my_collider = c_my_entity->get<TCompCollider>();
			TCompTransform * c_my_transform = c_my_entity->get<TCompTransform>();
			if(!c_my_collider->collisionDistance(c_my_transform->getPosition(), -EnginePhysics.gravity, 1.f) && crouchButton)
			*/
			crouchButton = !crouchButton;
			TMsgSetFSMVariable crouch;
			crouch.variant.setName("crouch");
			crouch.variant.setBool(crouchButton);
			e->sendMsg(crouch);
		}

		if (EngineInput["btAction"].hasChanged())
		{
			TMsgSetFSMVariable action;
			action.variant.setName("action");
			action.variant.setBool(true);
			e->sendMsg(action);
		}

		if (EngineInput["btSecAction"].getsPressed())
		{
			/* Move this from here.. */
			CEntity * c_my_entity = CHandle(this).getOwner();
			TCompTempPlayerController * c_my_player = c_my_entity->get<TCompTempPlayerController>();

			if (c_my_player->isInhibited) {
				TMsgSetFSMVariable keyPressed;
				keyPressed.variant.setName("hitPoints");
				keyPressed.variant.setBool(true);
				e->sendMsg(keyPressed);
			}
		}


		if (EngineInput["btUp"].getsReleased()) {
			TCompTempPlayerController * c_my_player = get<TCompTempPlayerController>();
			c_my_player->upButtonReselased();
		}
	}

}
