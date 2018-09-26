function onSceneStart_scene_intro()
	
	show_tutorial_sm_enemy = false;
	if(ambiance == nil or not ambiance:isValid()) then
		ambiance = playEvent("event:/Ambiance/Intro_Ambiance");
	end
	
	cinematicsEnabled = false;
	
	--#Debug position for start
	--move("The Player", VEC3(-7, 0, -43), VEC3(-7, 0, -44));

	-- First Cinematic --
	if(cinematicsEnabled and not isCheckpointSaved()) then
		intro_intro_cinematic();
	end
	setAIState("Patrol_Cinematic_Inhibitor", true, "dead_cinematic");
	getSignRendersForIntro();
end

function onSceneEnd_scene_intro()
	--ambiance:stop(true);
end

function intro_intro_cinematic()
	move("The Player", VEC3(-7.5, 12.115, 34.2), VEC3(-7.5, 12.115, 33.2));
	--setCinematicPlayerState(true, "crouch_cinematic")
	blendInCamera("Camera_Cinematic_Intro", 0.0, "cinematic", "");
	setCinematicPlayerState(true,"crouchwalkfallsm_cinematic");
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Intro_End\", 1.2, \"cinematic\", \"cubicinout\")", 2.3);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro\", 0)", 4);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_End\", 3)", 5);
end

function transition_map_intro_to_coliseum()
	execScriptDelayed("blendInCamera(\"scene_transition\", 1.0, \"cinematic\", \"\")", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_coliseo\")", 5);
end

-- # Transition cinematic # --
function transition_intro_to_coliseum()
	--execScriptDelayed("blendInCamera(\"scene_transition\", 1.0, \"cinematic\", \"\")", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	--execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_coliseo\")", 2);
end

-- # Trigger cinematic # --

function onTriggerEnter_Trigger_Inhibitor_Cinematic_player()
	setAIState("Patrol_Cinematic_Inhibitor", true, "inhibitor_cinematic");
	if(cinematicsEnabled) then
		intro_inhibitor_cinematic();
	end
	temp = getEntityByName("Trigger_Inhibitor_Cinematic");
	temp:destroy();
end

function intro_inhibitor_cinematic()
	setCinematicPlayerState(true, "inhibitor_cinematic", "");
	blendInCamera("Camera_Cinematic_Inhibitor_Patrol" ,0,"cinematic","");
	--move player to patrol and orientate cameras
	player = getEntityByName("The Player");
	e_player = toEntity(player);
	patrol_cinematic = getEntityByName("Patrol_Cinematic_Inhibitor");
	e_patrol_cinematic = toEntity(patrol_cinematic);
	t_patrol = toTransform(e_patrol_cinematic:getCompByName("transform"));
	t_player = toTransform(e_player:getCompByName("transform"));
	t_player:lookAt(t_player:getPosition(), t_patrol:getPosition());
	resetMainCameras();
	--hide tutorial while in cinematic
	e_tutorial_player = toEntity(getEntityByName("Tutorial Player"));
	render_tutorial_player = toRender(e_tutorial_player:getCompByName("render"));
	render_tutorial_player.visible = false;
	e_tutorial_weapon_left = toEntity(getEntityByName("tuto_weap_disc_left"));
	render_tutorial_weapon_left = toRender(e_tutorial_weapon_left:getCompByName("render"));
	render_tutorial_weapon_left.visible = false;
	e_tutorial_weapon_right = toEntity(getEntityByName("tuto_weap_disc_right"));
	render_tutorial_weapon_right = toRender(e_tutorial_weapon_right:getCompByName("render"));
	render_tutorial_weapon_right.visible = false;
	--end
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 3.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Inhibitor_Patrol_End\",0.5,\"cinematic\", \"cubicinout\")", 1.4);	
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Inhibitor_Patrol\",0)", 2.0);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Inhibitor_Patrol_End\",0.5)", 3.0);
	execScriptDelayed("render_tutorial_player.visible = true", 3.5);
	execScriptDelayed("render_tutorial_weapon_left.visible = true", 3.5);
	execScriptDelayed("render_tutorial_weapon_right.visible = true", 3.5);

end

function onTriggerEnter_Trigger_Capsules_Cinematic_player()
	setCinematicPlayerState(true, "inhibitor_cinematic", "");
	blendInCamera("Camera_Cinematic_Capsules", 1.5, "cinematic", "cubicinout");
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Capsules\",1.5)", 5.0);
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 6.5);
	temp = getEntityByName("Trigger_Capsules_Cinematic");
	temp:destroy();
end


-- # Trigger tutorials # --

function onTriggerEnter_SMTutorial_player()
	moveTutorialPlayer(VEC3(-7,0,25.2), VEC3(-7,0,23.2), true, "sm_tutorial");
	render_sm_sign.visible = true;
end

function onTriggerExit_SMTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	render_sm_sign.visible = false;
end

function onTriggerEnter_SMVerTutorial_player()
	moveTutorialPlayer(VEC3(1.7, 0, 13), VEC3(1.7, 0, 12), true, "sm_ver_tutorial");
	render_smver_sign.visible = true;
	--container = spawn("tutorial/container_tutorial", VEC3(4.6, 0, 17.2), VEC3(4.6, 0, 18));
end

function onTriggerExit_SMVerTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	render_smver_sign.visible = false;
	--container:destroy();
end

function onTriggerEnter_SMFallTutorial_player()
	moveTutorialPlayer(VEC3(1.4, 16, 8), VEC3(1.4, 16, 7), true, "sm_fall_tutorial");
	render_smfall_sign.visible = true;
end

function onTriggerExit_SMFallTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	render_smfall_sign.visible = false;
end

function onTriggerEnter_RunCrouchTutorial_player()
	moveTutorialPlayer(VEC3(-11, 0, -6.6), VEC3(-11, 0, -7), true, "walk_run_tutorial");
	render_runcrouch_sign.visible = true;
end

function onTriggerExit_RunCrouchTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	render_runcrouch_sign.visible = false;
end

function onTriggerEnter_InhibitorTutorial_player()
	moveTutorialPlayer(VEC3(-25, 0, -38), VEC3(-24, 0, -38), true, "remove_inhibitor_tutorial");
	render_inhibitor_sign.visible = true;
end

function onTriggerExit_InhibitorTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	render_inhibitor_sign.visible = false;
end

function onTriggerEnter_SMFenceTutorial_player()
	moveTutorialPlayer(VEC3(-10, 0, -43), VEC3(-10, 0, -44), true, "sm_fence_tutorial");
	fence = spawn("tutorial/fence_tutorial", VEC3(-10, 0, -43.5), VEC3(-10, 0, -44.5));
	render_smobstacles_sign.visible = true;
end

function onTriggerExit_SMFenceTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	render_smobstacles_sign.visible = false;
	fence:destroy();
end

function onTriggerEnter_AttackTutorial_player()
	if not show_tutorial_sm_enemy then
		moveTutorialPlayer(VEC3(-6.3, 0, -49.475), VEC3(-6.3, 0, -51), true, "attack_tutorial");
		patrol = spawn("tutorial/patrol_tutorial", VEC3(-6.3, 0, -50.5), VEC3(-6.3, 0, -52));
		render_attack_sign.visible = true;
	else
		moveTutorialPlayer(VEC3(-6.3, 0, -49.475), VEC3(-6.3, 0, -51), true, "sm_enemy_tutorial");
		patrol = spawn("tutorial/patrol_tutorial", VEC3(-6.3, 0, -50.5), VEC3(-6.3, 0, -52));
		render_smenemy_sign.visible = true;
	end
end

function onTriggerExit_AttackTutorial_player()
	moveTutorialPlayer(VEC3(0,-30,0), VEC3(0,0,25), false, "");
	render_attack_sign.visible = false;
	render_smenemy_sign.visible = false;
	patrol:destroy();
end

function onPatrolStunned_IntroPatrol()
	show_tutorial_sm_enemy = true;
	moveTutorialPlayer(VEC3(-6.3, 0, -49.475), VEC3(-6.3, 0, -51), true, "sm_enemy_tutorial");
	render_attack_sign.visible = false;
	render_smenemy_sign.visible = true;
end

function moveTutorialPlayer(position, lookAt, active, tutorial_state)
	h_tutorial_player = getEntityByName("Tutorial Player");
	e_tutorial_player = toEntity(h_tutorial_player);
	tutorial_player_transform = toTransform(e_tutorial_player:getCompByName("transform"));
	tutorial_player_transform:lookAt(position, lookAt);
	setTutorialPlayerState(active, tutorial_state);
end

--# Sign renders #--
function getSignRendersForIntro()
	render_sm_sign = toRender(toEntity(getEntityByName("tutorial_sign_sm")):getCompByName("render"));
	render_sm_sign.visible = false;
	render_smver_sign = toRender(toEntity(getEntityByName("tutorial_sign_ver_sm")):getCompByName("render"));
	render_smver_sign.visible = false;
	render_smfall_sign = toRender(toEntity(getEntityByName("tutorial_sign_fall_sm")):getCompByName("render"));
	render_smfall_sign.visible = false;
	render_runcrouch_sign = toRender(toEntity(getEntityByName("tutorial_sign_run_crouch")):getCompByName("render"));
	render_runcrouch_sign.visible = false;
	render_inhibitor_sign = toRender(toEntity(getEntityByName("tutorial_sign_inhibitor")):getCompByName("render"));
	render_inhibitor_sign.visible = false;
	render_smobstacles_sign = toRender(toEntity(getEntityByName("tutorial_sign_obstacles")):getCompByName("render"));
	render_smobstacles_sign.visible = false;
	render_attack_sign = toRender(toEntity(getEntityByName("tutorial_sign_attack")):getCompByName("render"));
	render_attack_sign.visible = false;
	render_smenemy_sign = toRender(toEntity(getEntityByName("tutorial_sign_enemy_sm")):getCompByName("render"));
	render_smenemy_sign.visible = false;
end