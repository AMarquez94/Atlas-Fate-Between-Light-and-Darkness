function animation_attack_start(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	playerController:pauseEnemy();
end

function animation_attack_attack(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	playerController:stunEnemy();
end