function onSceneStart_scene_coliseo()
	toggleButtonCanBePressed("COL_mesh_terminal02", true);
	toggleButtonCanBePressed("COL_mesh_terminal01", false);
end

function onScenePartialStart_scene_coliseo()
	onSceneStart_scene_coliseo();
	movePlayerToRefPos("col_intro_suelo001", i_ref_pos);
	--tdoor_intro_corridor = toDoor(toEntity(getEntityByName("tech_door_intro")):getCompByName("door"));
	--tdoor_intro_coliseo = toDoor(toEntity(getEntityByName("tech_door_intro")):getCompByName("door"));
end

function onSceneStart_scene_coliseo_2()
	toggleButtonCanBePressed("COL_mesh_terminal02", false);
	toggleButtonCanBePressed("COL_mesh_terminal01", true);
end

function onScenePartialStart_scene_coliseo_2()
	onSceneStart_scene_coliseo_2();
	movePlayerToRefPos("col_intro_suelo001", i_ref_pos);
end

function transition_coliseum_to_zone_a()
	toggleButtonCanBePressed("Button Open ZoneA", false);
	execScriptDelayed("blendInCamera(\"scene_transition_zone_a\", 1.0, \"cinematic\", \"\")", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	--execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_zone_a\")", 4);
end

function transition_coliseum_to_courtyard()
	toggleButtonCanBePressed("Button Open Courtyard", false);
	execScriptDelayed("blendInCamera(\"scene_transition_couryard\", 1.0, \"cinematic\", \"\")", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	--execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_basilic_courtyard\")", 4);
end