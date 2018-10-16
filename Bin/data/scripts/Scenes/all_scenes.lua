function onSceneStart()
	playerController = getPlayerController();
	playerNoiseEmitter = getPlayerNoiseEmitter();
	gameManager = getGameManager();
	immortal(true);
	--cg_drawfps(false);
	execScriptDelayed("immortal(false)", 1);
	execScriptDelayed("resetPatrolLights()", 0.1);
	if(ambiance == nil or not ambiance:isValid()) then
		ambiance = playEvent("event:/Ambiance/Intro_Ambiance");
	end

	if isDebug() then
		lanternsDisable(true);
		shadowsToggle();
		postFXToggle();
		cg_drawlights(2);
	end	
end

function onSceneEnd()
	--particles:killAll();
	stopAllAudioComponents();
	if(ambiance ~= nil and ambiance:isValid()) then
		ambiance:stop();
	end
end

function onScenePartialEnd()
	
end

function onTriggerEnter_TriggerKill_player()
	playerController:die();
end

function setCorridorInvisible()
	makeVisibleByTag("corridor", false);
	makeVisibleByTag("corridor_door", true);
	makeVisibleByTag("dir_light", true);
end

function startCinematicMode(time_to_end)
	subClear();
	isInCinematicMode(true);
	execScriptDelayed("isInCinematicMode(false);", time_to_end);
end