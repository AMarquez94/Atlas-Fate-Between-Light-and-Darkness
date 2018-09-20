function onSceneStart()
	playerController = getPlayerController();
	immortal();
	execScriptDelayed("immortal()", 1);
	resetPatrolLights();
	
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