function onSceneStart_scene_basilic_courtyard()
	setCorridorInvisible();
	r_shutdownscreen01 = toRender(toEntity(getEntityByName("BC_Screen001_Apagado")):getCompByName("render"));
	r_shutdownscreen01.visible = false;
	r_shutdownscreen02 = toRender(toEntity(getEntityByName("BC_Screen002_Apagado")):getCompByName("render"));
	r_shutdownscreen02.visible = false;
end

function onScenePartialStart_scene_basilic_courtyard()
	--onSceneStart_scene_basilic_courtyard();
	movePlayerToRefPos("bc_suelo001", i_ref_pos);
	r_shutdownscreen01 = toRender(toEntity(getEntityByName("BC_Screen001_Apagado")):getCompByName("render"));
	r_shutdownscreen01.visible = false;
	r_shutdownscreen02 = toRender(toEntity(getEntityByName("BC_Screen002_Apagado")):getCompByName("render"));
	r_shutdownscreen02.visible = false;
end

function transition_basilic_courtyard_to_interior()
	execScriptDelayed("loadScene(\"scene_basilic_interior\")", 2);
end

function onTriggerEnter_TriggerVictory_player()
	changeGamestate("main_menu");
end

function onTriggerEnter_BC_trigger_opendoor_bc_player()
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"bc_marco_puerta001\")):getCompByName(\"door\")):open()", 0.5);
	getEntityByName("BC_trigger_opendoor_bc"):destroy();
end

function onTriggerEnter_BC_trigger_closedoor_bc_player()
	bc_bcdoor = toDoor(toEntity(getEntityByName("bc_marco_puerta001")):getCompByName("door"));
	bc_bcdoor:setClosedScript("setCorridorInvisible()");
	bc_bcdoor:close();
	getEntityByName("BC_trigger_closedoor_bc"):destroy();
end

function onTriggerEnter_BC_trigger_closed_door_player()
	--look_closing_screen();
	subClear();
	activateSubtitles(24);
	execScriptDelayed("deactivateSubtitles();", 8);
	temp = getEntityByName("BC_trigger_closed_door");
	temp:destroy();
end

function onTriggerEnter_BC_Trigger_ScreenLight_player()
	sendPlayerIlluminatedMsg(getEntityByName("BC_Trigger_ScreenLight"), true);
end

function onTriggerExit_BC_Trigger_ScreenLight_player()
	sendPlayerIlluminatedMsg(getEntityByName("BC_Trigger_ScreenLight"), false);
end

function bc_button_shutdown_screens_pressed(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	onTriggerExit_BC_Trigger_ScreenLight_player();
	getEntityByName("BC_Trigger_ScreenLight"):destroy();
	toRender(toEntity(getEntityByName("BC_Screen001")):getCompByName("render")).visible = false;
	toRender(toEntity(getEntityByName("BC_Screen002")):getCompByName("render")).visible = false;
	r_shutdownscreen01.visible = true;
	r_shutdownscreen02.visible = true;
end

function look_closing_screen()

	setInBlackScreen(0.25);
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	resetMainCameras();
	--execScriptDelayed("blendInCamera(\"Camera_Cinematic_ZoneA_Tower\", 0.0, \"cinematic\", \"\")", 0.27);
	--execScriptDelayed("blendInCamera(\"Camera_Cinematic_Basilic_Courtyard_Rot_2\", 10.0, \"cinematic\", \"\")", 5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Basilic_Courtyard_Rot_1\", 0.0, \"cinematic\", \"\")", 0.27);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Basilic_Courtyard_Rot_1\", 5)", 5.25);

	--execScriptDelayed("setInBlackScreen(0.25)",9);
	--execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Sonar_2\", 0)", 9.25);
	--execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Sonar_1\", 0)", 9.25);
	--execScriptDelayed("setOutBlackScreen(0.25);",9.75);

	setCinematicPlayerState(true, "inhibitor_capsules", "");
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 10.25);

end