#include "mcv_platform.h"
#include "module_debug.h"
#include "render/render_objects.h"
#include "input/devices/mouse.h"
#include "windows/app.h"
#include "components/comp_camera.h"
#include "components/comp_transform.h"


bool CModuleDebug::start() {

    return true;
}

bool CModuleDebug::stop() {

    return true;
}

void CModuleDebug::update(float delta) {

    if (EngineInput["btR_btMouseRClick"].getsPressed() || EngineInput["btR_btMouseLClick"].getsPressed()) {

        POINT mouse_loc;
        GetCursorPos(&mouse_loc);
        ScreenToClient(CApp::get().getWnd(), &mouse_loc);
        VEC2 mouse_pos = VEC2(Clamp((int)mouse_loc.x, 0, CApp::get().xres), Clamp((int)mouse_loc.y, 0, CApp::get().yres));

        mouse_pos.x = mapInRange(-1, 1, 0, CApp::get().xres, mouse_pos.x);
        mouse_pos.y = -mapInRange(-1, 1, 0, CApp::get().yres, mouse_pos.y);
       
        VEC3 origin = VEC3(mouse_pos.x, mouse_pos.y, 0);
        VEC3 dest = VEC3(mouse_pos.x, mouse_pos.y, 1);

        VEC3 raycast_origin;
        VEC3 raycast_dest;

        CEntity* eCurrentCamera = EngineCameras.getCurrentCamera();
        TCompCamera* camera = eCurrentCamera->get< TCompCamera >();
        camera->getWorldCoordOfScreenCoords(origin, &raycast_origin);
        camera->getWorldCoordOfScreenCoords(dest, &raycast_dest);

        /* Lanzar rayo para determinar posicion del mouse */
        VEC3 pos = VEC3::Zero;
        VEC3 dir = (raycast_dest - raycast_origin).Normalized();
        physx::PxRaycastHit hit;
        if (EnginePhysics.Raycast(raycast_origin, dir, INFINITE, hit)) {
            pos = PXVEC3_TO_VEC3(hit.position);
            //CHandle hitCollider;
            //hitCollider.fromVoidPtr(hit.actor->userData);
            //if (hitCollider.isValid()) {
            //    CEntity* entityShooted = hitCollider.getOwner();
            //    dbg("Entity shooted %s\n", entityShooted->getName());
            //}
            recalculateNavmesh = true;
        }
        
        if (EngineInput["btR_btMouseLClick"].getsPressed()) {
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