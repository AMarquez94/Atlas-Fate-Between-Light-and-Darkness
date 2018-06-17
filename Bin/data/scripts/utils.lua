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

function deleteEnemies()
	SLB.deleteEnemies()
end

function postFXToggle()
	SLB.postFXToggle()
end

function blendInCamera(cameraName, blendInTime)
	SLB.blendInCamera(cameraName, blendInTime)
end

function blendOutCamera(cameraName, blendOutTime)
	SLB.blendOutCamera(cameraName, blendOutTime)
end

function setPlayerInhibited(inhibited)
	playerController.inhibited = inhibited;
end

function loadCheckpoint()
	SLB.loadCheckpoint();
end

function infiniteStamineToggle()
	SLB.infiniteStamineToggle()
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

function noClipToggle()
	SLB.noClipToggle()
end

function spotlightsToggle()
	SLB.spotlightsToggle()
end

function lanternsDisable(disable)
	SLB.lanternsDisable(disable)
end

function shadowsToggle()
	SLB.shadowsToggle()
end

function debugToggle()
	SLB.debugToggle()
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

function renderNavmeshToggle()
	SLB.renderNavmeshToggle()
end