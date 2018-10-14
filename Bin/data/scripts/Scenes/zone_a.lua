function onSceneStart_scene_zone_a()
	setCorridorInvisible()
end

function onScenePartialStart_scene_zone_a()
	--onSceneStart_scene_zone_a();
	movePlayerToRefPos("zone_a_in_suelo001", i_ref_pos);
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
	toDoor(toEntity(getEntityByName("zone_a_outmarco_puerta002")):getCompByName("door")):open();
end

function enable_button_exit(button_handle)
	toButton(toEntity(getEntityByName("Button End Scene")):getCompByName("button")):setCanBePressed(true);
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
end

function onTriggerEnter_ZON_Trigger_Enter_ZoneA_player()
	zonea_a_door = toDoor(toEntity(getEntityByName("zone_a_in_marco_puerta001")):getCompByName("door"));
	zonea_a_door:setClosedScript("setCorridorInvisible()");
	zonea_a_door:close();
	cinematic_enter_zone_a();
	getEntityByName("ZON_Trigger_Enter_ZoneA"):destroy();
end

function cinematic_enter_zone_a()

	setInBlackScreen(0.25);
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	--execScriptDelayed("move(\"The Player\", VEC3(-40,0,-90),VEC3(-42,0,-91));",0.27);
	--execScriptDelayed("resetMainCameras()",0.5);
	execScriptDelayed("move(\"The Player\", VEC3(-40,0,-90),VEC3(-41,0,-90));",3);

	--execScriptDelayed("blendInCamera(\"Camera_Cinematic_ZoneA_Tower\", 10.0, \"cinematic\", \"\")", 0.27);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_ZoneA_Tower\", 0.0, \"cinematic\", \"\")", 0.27);

	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Sonar_2\", 10.0, \"cinematic\", \"\")", 5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Sonar_1\", 0.0, \"cinematic\", \"\")", 5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_ZoneA_Tower\", 0)", 5.25);

	execScriptDelayed("setInBlackScreen(0.25)",7);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Sonar_2\", 0)", 7.25);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Sonar_1\", 0)", 7.25);
	execScriptDelayed("setOutBlackScreen(0.25);",7.75);
	--execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Coliseo_Rot_2\", 0)", 5);
	--execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Coliseo_Rot_1\", 0)", 5);
	--execScriptDelayed("blendInCamera(\"Camera_Cinematic_Coliseo_ZoneA_Door\", 0.0, \"cinematic\", \"\")", 5);

	

	setCinematicPlayerState(true, "inhibitor_capsules", "");
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 8);

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