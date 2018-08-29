function onSceneStart_scene_intro()
	--playerController = getPlayerController();	--no need. We will do it always in each loading scene
	--blendInCamera("Camera Follow Drone", 0);
	if(ambiance == nil or not ambiance:isValid()) then
		ambiance = playEvent("event:/Ambiance/Intro_Ambiance");
	end
end

function onSceneEnd_scene_intro()
	--ambiance:stop(true);
end

function transition_map_intro_to_coliseum()
	execScriptDelayed("blendInCamera(\"scene_transition\", 1.0)", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_coliseo\")", 5);
end

function onTriggerEnter_CrouchTutorial_player()
	h_tutorial_player = getEntityByName("Tutorial Player");
	e_tutorial_player = toEntity(h_tutorial_player);
	tutorial_player_transform = toTransform(e_tutorial_player:getCompByName("transform"));
	--Continue...
	tutorial_player_transform:setPosition(VEC3(1, 1, 1));
end

function onTriggerExit_CrouchTutorial_player()
	h_tutorial_player = getEntityByName("Tutorial Player");
	e_tutorial_player = toEntity(h_tutorial_player);
	tutorial_player_transform = toTransform(e_tutorial_player:getCompByName("transform"));
	tutorial_player_transform:setPosition(VEC3(0,-30.0,0));
end