SLB.using(SLB)

--Prints in Visual Console the Lua Log since the game was started until now
function printLogicLog()
	logic:printLog()
end

function execDelayedScript(script, delay)
	SLB.execDelayedScript(script, delay)
end