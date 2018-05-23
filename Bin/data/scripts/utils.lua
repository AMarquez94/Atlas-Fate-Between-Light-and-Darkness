SLB.using(SLB)

--Prints in Visual Console the Lua Log since the game was started until now
function printLogicLog()
	logic:printLog()
end

function execDelayedScript(script, delay)
	SLB.execDelayedScript(script, delay)
end

function pauseGame(pause)
	SLB.pauseGame(pause)
end

function pauseEnemies(pause)
	SLB.pauseEnemies(pause)
end

function blendInCamera(cameraName, blendInTime)
	SLB.blendInCamera(cameraName, blendInTime)
end

function blendOutCamera(cameraName, blendOutTime)
	SLB.blendOutCamera(cameraName, blendOutTime)
end

function setInfiniteStamine(set)
	SLB.setInfiniteStamine(set)
end

function setPlayerInhibited(inhibited)
	playerController.inhibited = inhibited;
end