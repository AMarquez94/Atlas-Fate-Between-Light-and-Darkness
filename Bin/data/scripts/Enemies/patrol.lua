function animation_launchinhibitor(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:launchInhibitor();
end