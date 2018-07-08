#pragma once

#include "components/comp_base.h"


class TCompPlayerInput : public TCompBase
{
	/* Trick to save button state values */
	int inhibition = 0;
	bool crouchButton = false;
	bool attackButtonJustPressed = false;

    bool isInNoClipMode = false;

	DECL_SIBLING_ACCESS();

public:

	VEC2 movementValue;

	void debugInMenu();
	void load(const json& j, TEntityParseContext& ctx);
	void update(float dt);

    static void registerMsgs();

private:
	float _time = 0.f;
  float _timerRemoveInhibitor = 0.f;
  float _timeOffsetToRemoveInhibitor;
  bool _moveObject = false;
  bool _enemyStunned = false;
  bool _buttonPressed = false;

  void onMsgNoClipToggle(const TMsgNoClipToggle& msg);
};

