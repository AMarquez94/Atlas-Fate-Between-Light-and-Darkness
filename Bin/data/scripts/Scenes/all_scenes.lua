function onSceneStart()
	playerController = getPlayerController();
	immortal();
	execScriptDelayed("immortal()", 1);
end

function onSceneEnd()
	particles:killAll();
end