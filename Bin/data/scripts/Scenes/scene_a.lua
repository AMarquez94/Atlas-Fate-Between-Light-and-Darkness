function onSceneStart_scene_a()
	shadowsToggle();
	pauseEnemies();
	--blendInCamera("Camera Follow Drone", 0);
end

function onSceneEnd_zoneA()

		blendInCamera("scene_transition", 1.0);
		pausePlayerToggle();
		cinematicMode();

end

function onSceneEnd_zoneA_to_basilisc_courtyard()

		loadScene("scene_basilic_courtyard");

end