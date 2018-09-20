function animation_LaunchInhibitor(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:launchInhibitor();
end

function animation_Attack(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:attackPlayer();	
	ai_patrol:shakeCamera(0.7, 10, 0.2);
	ai_patrol:playSlamParticle();
end

function animation_Patrol_Walk_Left(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:playStepParticle(true);
	ai_patrol:shakeCamera(0.7, 10, 0.2);
end

function animation_Patrol_Walk_Right(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:playStepParticle(false);
	ai_patrol:shakeCamera(0.7, 10, 0.2);
end

function animation_Patrol_Run_Left(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:shakeCamera(0.7, 10, 0.2);
end

function animation_Patrol_Run_Right(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:shakeCamera(0.7, 10, 0.2);
end