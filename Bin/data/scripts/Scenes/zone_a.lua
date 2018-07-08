function transition_zone_a_to_coliseum()
	toggleButtonCanBePressed("Button End Scene", false);
	execScriptDelayed("blendInCamera(\"scene_transition\", 1.0)", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_coliseo_2\")", 4);
end

function enable_button_exit()
	toggleButtonCanBePressed("Button Open Exit", false);
	toggleButtonCanBePressed("Button End Scene", true);
end