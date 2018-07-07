function onSceneStart_map_intro()
	playerController = getPlayerController();
	--shadowsToggle();
	--pauseEnemies();
	--blendInCamera("Camera Follow Drone", 0);
end

function onSceneEnd_map_intro()

		blendInCamera("scene_transition", 1.0);
		pausePlayerToggle();
		cinematicMode();

end

function onSceneEnd_map_intro_to_ZoneA()

		loadScene("scene_zone_a");

end