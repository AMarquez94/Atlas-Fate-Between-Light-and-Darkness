function onSceneStart_map_intro()
	playerController = getPlayerController();
	shadowsToggle();
	--pauseEnemies();
	--blendInCamera("Camera Follow Drone", 0);
end

function onSceneEnd_map_intro()

		blendInCamera("scene_transition", 2.0);
		--pausePlayerToggle();
		cinematicIntroToMapA();

end