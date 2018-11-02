function onSceneStart_scene_basilic_courtyard()
	setCorridorInvisible("");
	toRender(toEntity(getEntityByName("BC_Screen001_Apagado")):getCompByName("render")).visible = false;
	toRender(toEntity(getEntityByName("BC_Screen002_Apagado")):getCompByName("render")).visible = false;
	execScriptDelayed("isInCinematicMode(false)", 0.1);
end

function onScenePartialStart_scene_basilic_courtyard()
	--onSceneStart_scene_basilic_courtyard();
	movePlayerToRefPos("bc_suelo001", i_ref_pos);
	toRender(toEntity(getEntityByName("BC_Screen001_Apagado")):getCompByName("render")).visible = false;
	toRender(toEntity(getEntityByName("BC_Screen002_Apagado")):getCompByName("render")).visible = false;
	gameManager:changeMainTheme();
end

function transition_basilic_courtyard_to_interior()
	execScriptDelayed("loadScene(\"scene_basilic_interior\")", 2);
end

function onTriggerEnter_BC_trigger_opendoor_bc_player()
	execScriptDelayed("openDoorOnEnter(\"bc_marco_puerta001\", false)", 0.5);
	getEntityByName("BC_trigger_opendoor_bc"):destroy();
end

function onTriggerEnter_BC_trigger_closedoor_bc_player()
	bc_bcdoor = toDoor(toEntity(getEntityByName("bc_marco_puerta001")):getCompByName("door"));
	bc_bcdoor:setClosedScript("setCorridorInvisible(\"\")");
	bc_bcdoor:close();
	getEntityByName("BC_trigger_closedoor_bc"):destroy();
end

function onTriggerEnter_BC_trigger_closed_door_player()
	--look_closing_screen();
	subClear();
	playVoice("bc_door_blocked");
	startTransmission(5);
	activateSubtitles(24);
	execScriptDelayed("deactivateSubtitles();", 5);
	temp = getEntityByName("BC_trigger_closed_door");
	temp:destroy();
end

function onTriggerEnter_BC_Trigger_ScreenLight_player()
	sendPlayerIlluminatedMsg(getEntityByName("BC_Trigger_ScreenLight"), true);
end

function onTriggerExit_BC_Trigger_ScreenLight_player()
	sendPlayerIlluminatedMsg(getEntityByName("BC_Trigger_ScreenLight"), false);
end

function onTriggerEnter_BC_trigger_interior_enter_player()
	enter_basilic_cinematic();
	getEntityByName("BC_trigger_interior_enter"):destroy();
end



function bc_button_shutdown_screens_pressed(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	execScriptDelayed("look_closing_screen()",0.25);
	onTriggerExit_BC_Trigger_ScreenLight_player();
	getEntityByName("BC_Trigger_ScreenLight"):destroy();
	
end

function look_closing_screen()
	startCinematicMode(10.25);
	setInBlackScreen(0.25);
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	resetMainCameras();

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Basilic_Courtyard_Rot_1\", 0.0, \"cinematic\", \"\")", 0.27);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Basilic_Courtyard_Rot_1\", 5)", 5.25);

	execScriptDelayed("toRender(toEntity(getEntityByName(\"BC_Screen001\")):getCompByName(\"render\")).visible = false;",3);
	execScriptDelayed("toRender(toEntity(getEntityByName(\"BC_Screen002\")):getCompByName(\"render\")).visible = false;",3);
	execScriptDelayed("toRender(toEntity(getEntityByName(\"BC_Screen001_Apagado\")):getCompByName(\"render\")).visible = true;",3);
	execScriptDelayed("toRender(toEntity(getEntityByName(\"BC_Screen002_Apagado\")):getCompByName(\"render\")).visible = true;",3);

	setCinematicPlayerState(true, "inhibitor_capsules", "");
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 10.25);

end

function enter_basilic_cinematic()
	startCinematicMode(10.25);
	gameManager:preloadFinalSceneSoundEvent();
	setInBlackScreen(0.25);	
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	setCinematicPlayerState(true, "inhibitor_capsules", "");

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Basilic_Courtyard_Enter_Rot_2\", 10.0, \"cinematic\", \"\")", 0.27);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Basilic_Courtyard_Enter_Rot_1\", 0.0, \"cinematic\", \"\")", 0.27);

	execScriptDelayed("setInBlackScreen(0.75);",9.75);	
	execScriptDelayed("loadScene(\"scene_basilic_interior\");",11);
	--execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	--Camera_Cinematic_Basilic_Courtyard_Enter_Rot_2

end