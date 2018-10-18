SLB.using(SLB)

function onGameStart()
	console = getConsole();
	logic = getLogic();
	particles = getParticles();
	addAllFunctionsToConsole();
	restartCinematics();
end

function restartCinematics()

	cinematicsEnabled = false;
	
	intro_intro_cinematicExecuted = false;
	intro_inhibitor_cinematicExecuted = false;
	Capsules_CinematicExecuted = false;
	cinematicCorridorToZoneAExecuted = false;
	cinematicCorridorToBasilicExecuted = false;
	cinematic_enter_zone_aExecuted = false;
	cinematic_tower_activatedExecuted = false;
	col_trigger_district_7Executed = false;
	zone_a_door_activated = false;
end

--Add all funtions to the console in order to have autocomplete. TODO: Only add custom functions
function addAllFunctionsToConsole()
	for key, item in pairs(_G) do
		if type(item) == "function" then
			console:addCommand(tostring(key));
		end
	end
end
