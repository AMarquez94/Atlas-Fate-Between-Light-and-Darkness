function animation_attack_start(sender)
	playerController:pauseEnemy();
end

function animation_attack_attack(sender)
	playerController:stunEnemy();
end

function animation_step_left(sender)
	playerController:playPlayerStep(true);
	--playerNoiseEmitter:makeNoise(float noiseRadius, float timeToRepeat, bool isNoise, bool isOnlyOnce, bool isArtificial);
	playerNoiseEmitter:makeNoise(2.0, 10.0, true, true, false);
end

function animation_step_right(sender)
	playerController:playPlayerStep(false);
	playerNoiseEmitter:makeNoise(2.0, 10.0, true, true, false);
end

function animation_run_step_left(sender)
	playerController:playPlayerStep(true);
	playerNoiseEmitter:makeNoise(2.0, 10.0, true, true, false);
end

function animation_run_step_right(sender)
	playerController:playPlayerStep(false);
	playerNoiseEmitter:makeNoise(2.0, 10.0, true, true, false);
end

function animation_crouch_step_left(sender)
	h = playerController:getLeftWeapon();
	t_audio = toAudio(toEntity(h):getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Steps/NormalSteps", true);
end

function animation_crouch_step_right(sender)
	--playerController:playPlayerStep(false);
	--playerNoiseEmitter:makeNoise(2.0, 10.0, true, true, false);
	h = playerController:getLeftWeapon();
	t_audio = toAudio(toEntity(h):getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Steps/NormalSteps", true);
end

function animation_attack_swingLeft(sender)
	h = playerController:getLeftWeapon();
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Weapons/WeaponAttack_1", true);
	
	t_particles = toParticles(e:getCompByName("particles"));
	t_particles:setSystemState(true);
	
end

function animation_attack_swingRight(sender)
	h = playerController:getRightWeapon();
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Weapons/WeaponAttack_2", true);
	
	t_particles = toParticles(e:getCompByName("particles"));
	t_particles:setSystemState(true);
	
end

function animation_attack_strikeLeft(sender)
	h = playerController:getLeftWeapon();
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Weapons/WeaponClash_1", true);
	
	t_particles = toParticles(e:getCompByName("particles"));
	t_particles:setSystemState(false);
	
end

function animation_attack_strikeParticleLeft(sender)

	h = playerController:getLeftWeapon();
	e = toEntity(h);
	t_transform = toTransform(e:getCompByName("transform"));	
	pos = t_transform:getPosition();
	particles:launchDynamicSystem("data/particles/def_attack_explosion.particles", pos, true);
	particles:launchDynamicSystem("data/particles/def_attack_explosion_large.particles", pos, true);
	particles:launchDynamicSystem("data/particles/def_attack_explosion_core.particles", pos, true);
		
end

function animation_attack_strikeParticleRight(sender)

	h = playerController:getLeftWeapon();
	e = toEntity(h);
	t_transform = toTransform(e:getCompByName("transform"));	
	pos = t_transform:getPosition();
	particles:launchDynamicSystem("data/particles/def_attack_explosion.particles", pos, true);
	particles:launchDynamicSystem("data/particles/def_attack_explosion_large.particles", pos, true);
	particles:launchDynamicSystem("data/particles/def_attack_explosion_core.particles", pos, true);
			
end

function animation_attack_strikeRight(sender)

	h = playerController:getRightWeapon();
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Weapons/WeaponClash_2", true);

	t_particles = toParticles(e:getCompByName("particles"));
	t_particles:setSystemState(false);
end

function animation_enter_merge(sender)
	h = getEntityByName("The Player");
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/SM/EnterMerge", true);
end

function animation_exit_merge(sender)
	h = getEntityByName("The Player");
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/SM/ExitMerge", true);
end

function animation_soft_land(sender)
	h = getEntityByName("The Player");
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Steps/NormalSteps", false);
	playerController:playLandParticles(true);
	playerController:playLandParticles(false);
	playerNoiseEmitter:makeNoise(6.0, 10, true, true, false);
end

function animation_hard_land(sender)
	h = getEntityByName("The Player");
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Land/Land", false);
	t_audio:playEvent("event:/Sounds/Player/Hurt/Hurt", false);
	playerController:playLandParticles(true);
	playerController:playLandParticles(false);
	playerNoiseEmitter:makeNoise(9.0, 10.0, true, true, true);
end

function animation_grab_enemy(sender)
	playerController:playSMSpirals();
end

function animation_sonda_normal(sender)
	playerNoiseEmitter:makeNoise(12, 10.0, true, true, true);
end

function animation_sonda_crouch(sender)
	playerNoiseEmitter:makeNoise(12, 10.0, true, true, true);
end