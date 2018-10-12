function onSceneStart_scene_zone_a()

end

function onScenePartialStart_scene_zone_a()
	onSceneStart_scene_zone_a();
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
	--makeVisibleByTag("corridor", true);
	toDoor(toEntity(getEntityByName("zone_a_outmarco_puerta002")):getCompByName("door")):open();
end

function enable_button_exit(button_handle)
	toButton(toEntity(getEntityByName("Button End Scene")):getCompByName("button")):setCanBePressed(true);
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
end

function onTriggerEnter_ZON_Trigger_Enter_ZoneA_player()
	toDoor(toEntity(getEntityByName("zone_a_in_marco_puerta001")):getCompByName("door")):close();
	getEntityByName("ZON_Trigger_Enter_ZoneA"):destroy();
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