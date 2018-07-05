function onSceneStart_map_intro()
	playerController = getPlayerController();
	--blendInCamera("Camera Follow Drone", 0);
end

function onSceneEnd_map_intro()

		blendInCamera("scene_transition", 3.0);
		pausePlayerToggle();
		--probando(level);

end