function onSceneStart_scene_coliseo()
	toggleButtonCanBePressed("COL_mesh_terminal02", true);
	toggleButtonCanBePressed("COL_mesh_terminal01", false);
end

function onSceneStart_scene_coliseo_2()
	toggleButtonCanBePressed("COL_mesh_terminal02", false);
	toggleButtonCanBePressed("COL_mesh_terminal01", true);
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