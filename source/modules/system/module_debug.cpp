#include "mcv_platform.h"
#include "module_debug.h"
#include "render/render_objects.h"
#include "input/devices/mouse.h"
#include "windows/app.h"


bool CModuleDebug::start() {

    return true;
}

bool CModuleDebug::stop() {

    return true;
}

void CModuleDebug::update(float delta) {
    //POINT mouse_loc;
    //GetCursorPos(&mouse_loc);
    //ScreenToClient(CApp::get().getWnd(), &mouse_loc);
    //VEC2 mouse_pos = VEC2(mouse_loc.x, mouse_loc.y);
    VEC3 raycast_begin;
    //raycast_begin.
    //dbg("Mouse pos: (%f, %f)\n", mouse_pos.x, mouse_pos.y);
    if (EngineInput["btMouseLClick"].getsPressed() || EngineInput["btMouseRClick"].getsPressed()) {

        /* Lanzar rayo para determinar posicion del mouse */
        VEC3 pos = VEC3::Zero;
        recalculateNavmesh = true;
        if (EngineInput["btMouseLClick"].getsPressed()) {
            p1 = pos;
        }
        else {
            p2 = pos;
        }
    }

    if (p1 != VEC3::Zero && p2 != VEC3::Zero && recalculateNavmesh) {
        navmeshPath = EngineNavmeshes.findPath(p1, p2);
    }
}

void CModuleDebug::render() {
    if (p1 != VEC3::Zero && p2 != VEC3::Zero && navmeshPath.size() > 0) {
        for(int i = 0; i <  navmeshPath.size() - 1; i++){
            renderLine(navmeshPath[i], navmeshPath[i + 1], VEC4(1, 1, 0, 1));
        }
    }
}