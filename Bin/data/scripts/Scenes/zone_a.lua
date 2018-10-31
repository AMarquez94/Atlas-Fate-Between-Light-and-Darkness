function onSceneStart_scene_zone_a()
	setCorridorInvisible("")
	execScriptDelayed("isInCinematicMode(false)", 0.1);
end

function onScenePartialStart_scene_zone_a()
	--onSceneStart_scene_zone_a();
	movePlayerToRefPos("zone_a_in_suelo001", i_ref_pos);
	gameManager:changeMainTheme();
	--execScriptDelayed("toDoor(toEntity(getEntityByName(\"zone_a_in_marco_puerta001\")):getCompByName(\"door\")):open();", 0.5);
end

function onScenePartialEnd_scene_zone_a()
	i_ref_pos = getPlayerLocalCoordinatesInReferenceTo("zone_a_outsuelo002");
end

function onTriggerEnter_ZON_trigger_opendoor_zonea_player()
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"zone_a_in_marco_puerta001\")):getCompByName(\"door\")):open()", 0.5);
	getEntityByName("ZON_trigger_opendoor_zonea"):destroy();
end

function transition_zone_a_to_coliseum(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	makeVisibleByTag("corridor", true);

	h = toEntity(getEntityByName("zone_a_outmarco_puerta002"));
	toDoor(h:getCompByName("door")):open();
	
	t_transform = toTransform(h:getCompByName("transform"));	
	pos = t_transform:getPosition();
	rot = t_transform:getRotation();
	particles:launchDynamicSystemRot("data/particles/def_door_smoke.particles", pos, rot, true);
	particles:launchDynamicSystemRot("data/particles/def_door_smoke_base.particles", pos, rot, true);
	
end

function enable_button_exit(button_handle)
	execScriptDelayed("toButton(toEntity(getEntityByName(\"Button End Scene\")):getCompByName(\"button\")):setCanBePressed(true)",2.25);
	if(cinematicsEnabled) then
		execScriptDelayed("cinematic_tower_activated()",0.5);
	end
	zone_a_door_activated = true;
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
end

function onTriggerEnter_zonea_trigger_smartass_player()
	
	if(not zone_a_door_activated) then
		subClear();
		playVoice("zonea_inactive_door");
		startTransmission(5.5);
		activateSubtitles(22);
		execScriptDelayed("deactivateSubtitles();", 5.5);
		zone_a_door_activated = true;
	end
	temp = getEntityByName("zonea_trigger_smartass");
	temp:destroy();
end

function onTriggerEnter_ZON_Trigger_Enter_ZoneA_player()
	zonea_a_door = toDoor(toEntity(getEntityByName("zone_a_in_marco_puerta001")):getCompByName("door"));
	
	h = toEntity(getEntityByName("zone_a_in_marco_puerta001"));
	
	t_transform = toTransform(h:getCompByName("transform"));	
	pos = t_transform:getPosition();
	rot = t_transform:getRotation();
	particles:launchDynamicSystemRot("data/particles/def_door_smoke.particles", pos, rot, true);
	particles:launchDynamicSystemRot("data/particles/def_door_smoke_base.particles", pos, rot, true);
	
	zonea_a_door:setClosedScript("setCorridorInvisible(\"\")");
	zonea_a_door:close();
	if(cinematicsEnabled and not cinematic_enter_zone_aExecuted) then
		cinematic_enter_zone_a();
	end
	getEntityByName("ZON_Trigger_Enter_ZoneA"):destroy();
end

function cinematic_enter_zone_a()

	startCinematicMode(10)
	execScriptDelayed("playVoice(\"zonea_intro\")", 0.27);
	execScriptDelayed("startTransmission(8.98)", 0.27);
	execScriptDelayed("activateSubtitles(20);", 0.27);
	execScriptDelayed("activateSubtitles(21);", 5);
	execScriptDelayed("deactivateSubtitles();", 9.25);
	
	setInBlackScreen(0.25);
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	execScriptDelayed("move(\"The Player\", VEC3(-40,0,-90),VEC3(-41,0,-90));",3);
	execScriptDelayed("resetMainCameras();",3.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_ZoneA_Tower_2\", 10.0, \"cinematic\", \"\")", 0.27);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_ZoneA_Tower\", 0.0, \"cinematic\", \"\")", 0.27);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Sonar_2\", 10.0, \"cinematic\", \"\")", 5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Sonar_1\", 0.0, \"cinematic\", \"\")", 5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_ZoneA_Tower\", 0)", 5.25);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_ZoneA_Tower_2\", 0)", 5.25);

	execScriptDelayed("setInBlackScreen(0.25)",11);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Sonar_2\", 0)", 11.25);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Sonar_1\", 0)", 11.25);
	execScriptDelayed("setOutBlackScreen(0.25);",11.75);

	setCinematicPlayerState(true, "inhibitor_capsules", "");
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 12);
	cinematic_enter_zone_aExecuted = true;

	temp = getEntityByName("intro_trigger_muralla");
	temp:destroy();

end

function cinematic_tower_activated()
	startCinematicMode(12.8);
	setInBlackScreen(0.25);
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	subClear();
	execScriptDelayed("playVoice(\"zonea_tower_activated\");", 2.25);
	execScriptDelayed("startTransmission(3.75)", 2.25);
	execScriptDelayed("activateSubtitles(23);", 2.25);
	execScriptDelayed("deactivateSubtitles();", 6);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Exit_Door_2\", 9, \"cinematic\", \"quintinout\")", 1.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Exit_Door_1\", 0.0, \"cinematic\", \"\")", 0.27);

	execScriptDelayed("setInBlackScreen(0.25)",11.5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Exit_Door_1\", 0)", 11.75);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Exit_Door_2\", 0)", 11.75);
	execScriptDelayed("setOutBlackScreen(0.25);",12.25);
	setCinematicPlayerState(true, "inhibitor_capsules", "");
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 12.75);
end

function onTriggerEnter_ZON_Trigger_Exit_ZoneA_player()
	getEntityByName("ZON_Trigger_Exit_ZoneA"):destroy();
	tdoor = toDoor(toEntity(getEntityByName("zone_a_outmarco_puerta002")):getCompByName("door"));
	tdoor:setClosedScript("destroyZoneAPreloadCol()");
	tdoor:close();
end

function destroyZoneAPreloadCol()
	destroyPartialScene();
	execScriptDelayed("preloadScene(\"scene_coliseo_2\")", 0.1);
end