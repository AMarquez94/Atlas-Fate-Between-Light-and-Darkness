function animation_launchinhibitor(sender)
	e_patrol = toEntity(sender);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:launchinhibitor();
	--patrol = sender:getCompByName(sender);
	--patrol:launchinhibitor();
	--loadScene("scene_map_intro");
	--patrol = sender:getCompByName("ai_patrol");
	--patrol:launchinhibitor();
end