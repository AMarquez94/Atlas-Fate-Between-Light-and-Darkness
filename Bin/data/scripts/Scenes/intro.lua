function onSceneStart_scene_intro()
	
	show_tutorial_sm_enemy = false;
	setCorridorInvisible("");
	
	--#Debug position for start
	--move("The Player", VEC3(-7, 0, -43), VEC3(-7, 0, -44));

	-- First Cinematic --
	if(cinematicsEnabled and not isCheckpointSaved() and not intro_intro_cinematicExecuted) then
		intro_intro_cinematic();
	else
		execScriptDelayed("isInCinematicMode(false)", 0.1);
	end
	
	setAIState("Patrol_Cinematic_Inhibitor", true, "dead_cinematic");
	getSignRendersForIntro();
end

function onSceneEnd_scene_intro()
	--ambiance:stop(true);
end

function onScenePartialEnd_scene_intro()
	i_ref_pos = getPlayerLocalCoordinatesInReferenceTo("intro_suelo001");
	i_corridor_dust = getEntityLocalCoordinatesInReferenceTo("corridor_dust", "intro_suelo001");
	i_corridor_fog = getEntityLocalCoordinatesInReferenceTo("corridor_fog", "intro_suelo001");
	i_corridor_whisps = getEntityLocalCoordinatesInReferenceTo("corridor_whisps", "intro_suelo001");
end

function intro_intro_cinematic()
	startCinematicMode(91);
	move("The Player", VEC3(-6.275, 12.115, 32.7),VEC3(-6.275, 12.115, 31.7));
	resetMainCameras();
	setCinematicPlayerState(true,"crouchwalkfallsm_cinematic");

	--Setting active and deactivating the intro cinematic video
	activateCinematicVideoIntro(2,25);
	execScriptDelayed("deactivateCinematicVideoIntro();",28);

	blendInCamera("Camera_Cinematic_Intro_video", 0.0, "cinematic", "");
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Intro_Player_Back\", 20, \"cinematic\", \"sineinout\")", 29);

	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_video\", 0)", 52);

	 --Activating the subtitles
	execScriptDelayed("startTransmission(28.5)", 51.5);
	execScriptDelayed("playVoice(\"intro_cinematic\")", 51.3);
	execScriptDelayed("activateSubtitles(1);",51.5);
	execScriptDelayed("activateSubtitles(2);",53.5);
	execScriptDelayed("activateSubtitles(3);",58);
	execScriptDelayed("activateSubtitles(4);",63);
	execScriptDelayed("activateSubtitles(5);",65.5);
	execScriptDelayed("activateSubtitles(6);",70.5);
	execScriptDelayed("activateSubtitles(7);",72);
	execScriptDelayed("activateSubtitles(8);",73.25);
	execScriptDelayed("activateSubtitles(9);",77.75);
	execScriptDelayed("deactivateSubtitles();",80.0);

	--Second Frame
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_Player_Back\", 0)", 54.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Intro_Rotation_2\", 30.0, \"cinematic\", \"\")", 54.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Intro_Rotation_1\", 0.0, \"cinematic\", \"\")", 54.5);
	
	--Third Frame
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_Rotation_1\", 0)", 73.5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_Rotation_2\", 0)", 73.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Intro_Rotation_Other_2\", 30.0, \"cinematic\", \"\")", 73.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Intro_Rotation_Other_1\", 0.0, \"cinematic\", \"\")", 73.5);

	--Last Frame
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_Rotation_Other_2\", 0)", 81);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_Rotation_Other_1\", 0)", 81);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Intro_SM_Caida\", 0.0, \"cinematic\", \"\")", 81);

	--Returning to player camera
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Intro_SM_Caida\", 5)", 86);
	execScriptDelayed("intro_intro_cinematicExecuted = true", 52);
end

-- # Transition cinematic # --
function transition_intro_to_coliseum(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	makeVisibleByTag("corridor", true);
	toDoor(toEntity(getEntityByName("intro_marco_puerta001")):getCompByName("door")):open();
	
	h = toEntity(getEntityByName("intro_marco_puerta001"));
	
	t_transform = toTransform(h:getCompByName("transform"));	
	pos = t_transform:getPosition();
	rot = t_transform:getRotation();
	particles:launchDynamicSystemRot("data/particles/def_door_smoke.particles", pos, rot, true);
	particles:launchDynamicSystemRot("data/particles/def_door_smoke_base.particles", pos, rot, true);
end

function disableButton(button_handle, is_enabled)
	h = CHandle();
	h:fromUnsigned(button_handle);
	toButton(toEntity(h):getCompByName("button")):setCanBePressed(is_enabled);
end

-- # Trigger loading # --
function onTriggerEnter_intro_trigger_corridor_player()
	getEntityByName("intro_trigger_corridor"):destroy();
	tdoor = toDoor(toEntity(getEntityByName("intro_marco_puerta001")):getCompByName("door"));
	tdoor:setClosedScript("destroyIntroPreloadCol()");
	tdoor:close();
end

function destroyIntroPreloadCol()
	destroyPartialScene();
	execScriptDelayed("preloadScene(\"scene_coliseo\")", 0.1);
end

-- # Trigger cinematic # --

function onTriggerEnter_Trigger_Inhibitor_Cinematic_player()
	if(cinematicsEnabled and not intro_inhibitor_cinematicExecuted) then
		setAIState("Patrol_Cinematic_Inhibitor", true, "inhibitor_cinematic");
		intro_inhibitor_cinematic();
	end
	temp = getEntityByName("Trigger_Inhibitor_Cinematic");
	temp:destroy();
end

function intro_inhibitor_cinematic()
	startCinematicMode(5.75);
	move("The Player", VEC3(-19, 7, -33.5),VEC3(-19, 7, -35.5));
	resetMainCameras();
	
	execScriptDelayed("playVoice(\"intro_inhibitor\")", 5.75);
	execScriptDelayed("startTransmission(7.75)", 5.75);
	execScriptDelayed("activateSubtitles(14);", 5.75);
	execScriptDelayed("activateSubtitles(15);", 11);
	execScriptDelayed("deactivateSubtitles();", 13.5);

	setCinematicPlayerState(true, "inhibitor_cinematic", "");
	blendInCamera("Camera_Cinematic_Inhibitor_1" ,0,"cinematic","");


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
	render_inhibitor_sign.visible = false;	
	--end


	--execScriptDelayed("setCinematicPlayerState(false, \"\")", 5.75);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Inhibitor_2\",1.5,\"cinematic\", \"expoin\")", 0.5);	
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Inhibitor_1\",0)", 2.01);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Inhibitor_2\",0)", 5.25);

	execScriptDelayed("setInBlackScreen(0.5);",4.5);
	execScriptDelayed("setOutBlackScreen(0.25);",5.5);
	execScriptDelayed("move(\"The Player\", VEC3(-20.5, 0, -35.278),VEC3(-21.95, 0, -36.118));", 5.25);

	execScriptDelayed("resetMainCameras()",5.3);
	execScriptDelayed("render_tutorial_player.visible = true", 5.25);
	execScriptDelayed("render_tutorial_weapon_left.visible = true", 5.25);
	execScriptDelayed("render_tutorial_weapon_right.visible = true", 5.25);
	execScriptDelayed("render_inhibitor_sign.visible = true", 5.25);

	intro_inhibitor_cinematicExecuted = true;
end

function onTriggerEnter_Trigger_Capsules_Cinematic_player()
	if(cinematicsEnabled and not Capsules_CinematicExecuted) then
		subClear();
		startCinematicMode(12.4);
		execScriptDelayed("playVoice(\"intro_broken_bridge\")", 3);
		execScriptDelayed("startTransmission(6)", 3);
		execScriptDelayed("activateSubtitles(13);", 3);
		execScriptDelayed("deactivateSubtitles();", 9);
		
		setCinematicPlayerState(true, "capsules_cinematic", "");
		setInBlackScreen(0.25);
		execScriptDelayed("setOutBlackScreen(0.25);",0.3);
		execScriptDelayed("blendInCamera(\"Camera_Cinematic_Capsules_Rot_2\",10.0,\"cinematic\", \"\")", 0.27);	
		execScriptDelayed("blendInCamera(\"Camera_Cinematic_Capsules_Rot_1\",0.0,\"cinematic\", \"\")", 0.27);	

		execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Capsules_Rot_2\",0.0)", 5);	
		execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Capsules_Rot_1\",0.0)", 5);	
		execScriptDelayed("blendInCamera(\"Camera_Cinematic_Capsules_Rot_4\",10.0,\"cinematic\", \"\")", 5);	
		execScriptDelayed("blendInCamera(\"Camera_Cinematic_Capsules_Rot_3\",0.0,\"cinematic\", \"\")", 5);	

		execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Capsules_Rot_3\",0)", 12);
		execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Capsules_Rot_4\",0)", 12);

		execScriptDelayed("setInBlackScreen(0.25);",11.6);
		execScriptDelayed("setOutBlackScreen(0.25);",12.1);
		execScriptDelayed("setCinematicPlayerState(false, \"\")", 12.4);
		Capsules_CinematicExecuted = true;
		temp = getEntityByName("Trigger_Capsules_Cinematic");
		temp:destroy();
	end	
end
-- # Subtitles tutorials # --
function onTriggerEnter_intro_trigger_muralla_player()
	subClear();
	playVoice("intro_closeddoor");
	startTransmission(6);
	activateSubtitles(10);
	execScriptDelayed("deactivateSubtitles();", 6);
	temp = getEntityByName("intro_trigger_muralla");
	temp:destroy();
end

function onTriggerEnter_intro_trigger_muralla_alto_player()
	subClear();
	playVoice("intro_fall");
	startTransmission(10);
	activateSubtitles(11);
	execScriptDelayed("activateSubtitles(12);",7);
	execScriptDelayed("deactivateSubtitles();", 10);
	temp = getEntityByName("intro_trigger_muralla_alto");
	temp:destroy();
end

function onTriggerEnter_intro_trigger_patrol_care_player()
	subClear();
	playVoice("intro_patrol");
	startTransmission(7);
	activateSubtitles(16);
	execScriptDelayed("deactivateSubtitles();", 7);
	temp = getEntityByName("intro_trigger_patrol_care");
	temp:destroy();
end

function onTriggerEnter_intro_trigger_patrol_story_player()
	subClear();
	playVoice("intro_error");
	startTransmission(8.5);
	activateSubtitles(35);
	execScriptDelayed("deactivateSubtitles();", 8.5);
	temp = getEntityByName("intro_trigger_patrol_story");
	temp:destroy();
end

-- # Trigger tutorials # --

function onTriggerEnter_SMTutorial_player()
	moveTutorialPlayer(VEC3(-7,0,26), VEC3(-7,0,23.2), true, "sm_tutorial");
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
		moveTutorialPlayer(VEC3(-7, 0, -49.475), VEC3(-7, 0, -51), true, "attack_tutorial");
		patrol = spawn("tutorial/patrol_tutorial", VEC3(-7, 0, -50.5), VEC3(-7, 0, -52));
		render_attack_sign.visible = true;
	else
		moveTutorialPlayer(VEC3(-7, 0, -50), VEC3(-7, 0, -51), true, "sm_enemy_tutorial");
		patrol = spawn("tutorial/patrol_tutorial", VEC3(-7, 0, -50.5), VEC3(-7, 0, -52));
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
	moveTutorialPlayer(VEC3(-7, 0, -50), VEC3(-7, 0, -51), true, "sm_enemy_tutorial");
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