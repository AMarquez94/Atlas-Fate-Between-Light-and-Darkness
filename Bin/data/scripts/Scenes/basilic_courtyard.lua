function onSceneStart_scene_basilic_courtyard()

end

function onScenePartialStart_scene_basilic_courtyard()
	onSceneStart_scene_basilic_courtyard();
	movePlayerToRefPos("bc_suelo001", i_ref_pos);
	--execScriptDelayed("toDoor(toEntity(getEntityByName(\"bc_marco_puerta001\")):getCompByName(\"door\")):open();", 0.5);
end


function transition_basilic_courtyard_to_interior()
	execScriptDelayed("loadScene(\"scene_basilic_interior\")", 2);
end

function onTriggerEnter_TriggerVictory_player()
	changeGamestate("main_menu");
end

function onTriggerEnter_BC_trigger_opendoor_bc_player()
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"bc_marco_puerta001\")):getCompByName(\"door\")):open()", 0.5);
	getEntityByName("BC_trigger_opendoor_bc"):destroy();
end