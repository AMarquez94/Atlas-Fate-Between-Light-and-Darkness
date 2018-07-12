function onSceneStart()
	playerController = getPlayerController();
end

function onSceneEnd()
	particles:killAll();
end