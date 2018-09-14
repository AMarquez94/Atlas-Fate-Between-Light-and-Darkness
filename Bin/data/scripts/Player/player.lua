function animation_attack_start(sender)
	playerController:pauseEnemy();
end

function animation_attack_attack(sender)
	playerController:stunEnemy();
end

function animation_attack_swingLeft(sender)
	h = playerController:getLeftWeapon();
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Weapons/WeaponAttack_1", true);
end

function animation_attack_swingRight(sender)
	h = playerController:getRightWeapon();
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/Weapons/WeaponAttack_2", true);
end

function animation_enter_merge()
	h = getEntityByName("The Player");
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/SM/EnterMerge", true);
end

function animation_exit_merge()
	h = getEntityByName("The Player");
	e = toEntity(h);
	t_audio = toAudio(e:getCompByName("audio"));
	t_audio:playEvent("event:/Sounds/Player/SM/ExitMerge", true);
end