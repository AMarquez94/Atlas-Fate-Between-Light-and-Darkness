function onTriggerEnter_BI_TriggerKill_player()
	playerController:die();
end

function turn_off_and_on_light_interior()
	toggle_spotlight("Spot002");
	execScriptDelayed("toggle_spotlight(\"Spot002\")", 15);
end