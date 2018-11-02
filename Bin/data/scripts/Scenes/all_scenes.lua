function onSceneStart()
	playerController = getPlayerController();
	playerNoiseEmitter = getPlayerNoiseEmitter();
	gameManager = getGameManager();
	immortal(true);
	--cg_drawfps(false);
	execScriptDelayed("immortal(false)", 1);
	execScriptDelayed("resetPatrolLights()", 0.1);

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
end

function onScenePartialEnd()
	
end

function onTriggerEnter_TriggerKill_player()
	playerController:die();
end

function setCorridorInvisible(particles_to_destroy)
	makeVisibleByTag("corridor", false);
	makeVisibleByTag("corridor_door", true);
	makeVisibleByTag("dir_light", true);
	destroyHandleByTag(particles_to_destroy);
end

function startCinematicMode(time_to_end)
	subClear();
	isInCinematicMode(true);
	execScriptDelayed("isInCinematicMode(false);", time_to_end);
end

function startTransmission(time_to_end)	
	setEnemyHudState(true);
	execScriptDelayed("setEnemyHudState(false)", time_to_end);
end

function openDoorOnEnter(name, is_big)
	e = toEntity(getEntityByName(name));
	toDoor(e:getCompByName("door")):open();
	
	t_transform = toTransform(e:getCompByName("transform"));	
	pos = t_transform:getPosition();
	rot = t_transform:getRotation();
	if is_big then
		particles:launchDynamicSystemRot("data/particles/def_gate_smoke.particles", pos, rot, true);
		particles:launchDynamicSystemRot("data/particles/def_gate_smoke_base.particles", pos, rot, true);
		particles:launchDynamicSystemRot("data/particles/def_gate_smoke_middle.particles", pos, rot, true);
	else 
		particles:launchDynamicSystemRot("data/particles/def_door_smoke.particles", pos, rot, true);
		particles:launchDynamicSystemRot("data/particles/def_door_smoke_base.particles", pos, rot, true);
	end
end