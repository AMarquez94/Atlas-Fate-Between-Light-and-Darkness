function onSceneStart()
	playerController = getPlayerController();
	playerNoiseEmitter = getPlayerNoiseEmitter();
	gameManager = getGameManager();
	immortal(true);
	--cg_drawfps(false);
	execScriptDelayed("immortal(false)", 1);
	execScriptDelayed("resetPatrolLights()", 0.1);
	
	cinematicsEnabled = true;

	if isDebug() then
		lanternsDisable(true);
		shadowsToggle();
		postFXToggle();
		cg_drawlights(2);
	end	
end

function onSceneEnd()
	particles:killAll();
	stopAllAudioComponents();
end

function onTriggerEnter_TriggerKill_player()
	playerController:die();
end