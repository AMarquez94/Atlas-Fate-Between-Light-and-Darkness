SLB.using(SLB)

--Prints in Visual Console the Lua Log since the game was started until now
function printLogicLog()
	logic:printLog()
end

function printAllScripts()
	console:printAllScripts()
end

function execScriptDelayed(script, delay)
	SLB.execScriptDelayed(script, delay)
end

function pauseGame(pause)
	SLB.pauseGame(pause)
end

function pausePlayerToggle()
	SLB.pausePlayerToggle()
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

function blendOutActiveCamera(blendOutTime)
	SLB.blendOutActiveCamera(blendOutTime)
end

function setPlayerInhibited(inhibited)
	playerController.inhibited = inhibited;
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

function loadScene(level)
	SLB.loadScene(level)
end

function unloadScene()
	SLB.unloadScene()
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

function move(name, pos, lookAt)
	SLB.move(name, pos, lookAt)
end

function wireframeToggle()
	SLB.wireframeToggle()
end

function collidersToggle(onlyDynamics)
	SLB.collidersToggle(onlyDynamics)
end

function movePlayer(pos)
	SLB.movePlayer(pos)
end

function renderNavmeshToggle()
	SLB.renderNavmeshToggle()
end

function playSound2D(soundName)
	SLB.playSound2D(soundName)
end

function exeShootImpactSound()
	SLB.exeShootImpactSound()
end

function cinematicModeToggle()
	SLB.cinematicModeToggle()
end

function fpsToggle(bool)
	SLB.fpsToggle(bool)
end