SLB.using(SLB)

function onGameStart()
	console = getConsole();
	logic = getLogic();
	particles = getParticles();
	addAllFunctionsToConsole();
end

--Add all funtions to the console in order to have autocomplete. TODO: Only add custom functions
function addAllFunctionsToConsole()
	for key, item in pairs(_G) do
		if type(item) == "function" then
			console:addCommand(tostring(key));
		end
	end
end
