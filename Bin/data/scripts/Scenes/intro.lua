function onSceneStart_scene_intro()
	--playerController = getPlayerController();	--no need. We will do it always in each loading scene
	--blendInCamera("Camera Follow Drone", 0);
	show_tutorial_sm_enemy = false;
	if(ambiance == nil or not ambiance:isValid()) then
		ambiance = playEvent("event:/Ambiance/Intro_Ambiance");
	end
end

function onSceneEnd_scene_intro()
	--ambiance:stop(true);
end

function transition_map_intro_to_coliseum()
	execScriptDelayed("blendInCamera(\"scene_transition\", 1.0)", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_coliseo\")", 5);
end

function onTriggerEnter_SMTutorial_player()
	moveTutorialPlayer(VEC3(0,0,26), VEC3(0,0,25), true, "sm_tutorial");
end

function onTriggerExit_SMTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
end

function onTriggerEnter_SMVerTutorial_player()
	moveTutorialPlayer(VEC3(4.6, 0, 19), VEC3(4.6,0,15), true, "sm_ver_tutorial");
	container = spawn("tutorial/container_tutorial", VEC3(4.6, 0, 17.2), VEC3(4.6, 0, 18));
end

function onTriggerExit_SMVerTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	container:destroy();
end

function onTriggerEnter_SMFallTutorial_player()
	moveTutorialPlayer(VEC3(1.4, 16, 8), VEC3(1.4, 16, 7), true, "sm_fall_tutorial");
end

function onTriggerExit_SMFallTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
end

function onTriggerEnter_CrouchTutorial_player()
	moveTutorialPlayer(VEC3(-0.8, 0, -11), VEC3(-0.8, 0, -12), true, "crouch_tutorial");
end

function onTriggerExit_CrouchTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
end

function onTriggerEnter_InhibitorTutorial_player()
	moveTutorialPlayer(VEC3(-13, 0, -37), VEC3(-13, 0, -38), true, "remove_inhibitor_tutorial");
end

function onTriggerExit_InhibitorTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
end

function onTriggerEnter_SMFenceTutorial_player()
	moveTutorialPlayer(VEC3(-10, 0, -43), VEC3(-10, 0, -44), true, "sm_fence_tutorial");
	fence = spawn("tutorial/fence_tutorial", VEC3(-10, 0, -43.5), VEC3(-10, 0, -44.5));
end

function onTriggerExit_SMFenceTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	fence:destroy();
end

function onTriggerEnter_AttackTutorial_player()
	if not show_tutorial_sm_enemy then
		moveTutorialPlayer(VEC3(-6.3, 0, -49), VEC3(-6.3, 0, -51), true, "attack_tutorial");
		patrol = spawn("tutorial/patrol_tutorial", VEC3(-6.3, 0, -50.5), VEC3(-6.3, 0, -52));
	else
		moveTutorialPlayer(VEC3(-6.3, 0, -49), VEC3(-6.3, 0, -51), true, "sm_enemy_tutorial");
		patrol = spawn("tutorial/patrol_tutorial", VEC3(-6.3, 0, -50.5), VEC3(-6.3, 0, -52));
	end
end

function onTriggerExit_AttackTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	patrol:destroy();
end

function onPatrolStunned_IntroPatrol()
	show_tutorial_sm_enemy = true;
	moveTutorialPlayer(VEC3(-6.3, 0, -49), VEC3(-6.3, 0, -51), true, "sm_enemy_tutorial");
end


function moveTutorialPlayer(position, lookAt, active, tutorial_state)
	h_tutorial_player = getEntityByName("Tutorial Player");
	e_tutorial_player = toEntity(h_tutorial_player);
	tutorial_player_transform = toTransform(e_tutorial_player:getCompByName("transform"));
	tutorial_player_transform:lookAt(position, lookAt);
	setTutorialPlayerState(active, tutorial_state);
end