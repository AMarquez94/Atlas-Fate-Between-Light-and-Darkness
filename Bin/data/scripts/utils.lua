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

function blendInCamera(cameraName, blendInTime)
	SLB.blendInCamera(cameraName, blendInTime)
end

function blendOutCamera(cameraName, blendOutTime)
	SLB.blendOutCamera(cameraName, blendOutTime)
end

function fpsToggle()
	SLB.fpsToggle()
end

function debugToggle()
	SLB.debugToggle()
end

function pauseEnemies()
	SLB.pauseEnemies()
end

function deleteEnemies()
	SLB.deleteEnemies()
end

function movePlayer(x,y,z)
	SLB.movePlayer(x,y,z)
end

function staminaInfinite()
	SLB.staminaInfinite()
end

function immortal()
	SLB.immortal()
end

function inShadows()
	SLB.inShadows()
end

function speedBoost(speed)
	SLB.speedBoost(speed)
end

function playerInvisible()
	SLB.playerInvisible()
end

function spotlightsToggle()
	SLB.spotlightsToggle()
end

function lanternToggle()
	SLB.lanternToggle()
end

function shadowsToggle()
	SLB.shadowsToggle()
end

function loadscene(level)
	SLB.loadscene(level)
end

function cg_drawlights(type)
	SLB.cg_drawlights(type)
end

function spawn()
	SLB.spawn()
end