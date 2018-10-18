#include "mcv_platform.h"
#include "module_ia.h"


void CModuleIA::update(float delta)
{

}

void CModuleIA::render()
{

}

void CModuleIA::clearSharedBoards()
{
	patrolSB.stunnedPatrols.clear();
    patrolSB.patrolsWithLight.clear();
}
