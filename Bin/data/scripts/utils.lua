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

function movePlayer(pos)
	SLB.movePlayer(pos)
end

function loadCheckpoint()
	SLB.loadCheckpoint();
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

function postFXToggle()
	SLB.postFXToggle()
end

function loadscene(level)
	SLB.loadscene(level)
end

function cg_drawlights(type)
	SLB.cg_drawlights(type)
end

function spawn(type, pos, lookAt)
	SLB.spawn(type, pos, lookAt)
end

function animationsToggle()
	SLB.animationsToggle()
end

function noClipToggle()
	SLB.noClipToggle()
end

function move(name, pos, lookAt)
	SLB.move(name, pos, lookAt)
end

function wireframeToggle()
	SLB.wireframeToggle()
end

function collidersToggle(onlyDynamics)
	SLB.collidersToggle(onlyDynamics)
end

function setPlayerInhibited(inhibited)
	playerController.inhibited = inhibited;
end