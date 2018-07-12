function onTriggerEnter_BI_TriggerKill_player()
	playerController:die();
end

function turn_off_and_on_light_interior()
	toggleButtonCanBePressed("Button Turn Off Light", false);
	toggle_spotlight("Spot002");
	execScriptDelayed("toggle_spotlight(\"Spot002\")", 20);
	execScriptDelayed("toggleButtonCanBePressed(\"Button Turn Off Light\", true)", 20);
end