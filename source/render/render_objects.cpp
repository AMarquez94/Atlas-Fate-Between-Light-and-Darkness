#include "mcv_platform.h"
#include "render_objects.h"

CRenderCte<CCteCamera> cb_camera;
CRenderCte<CCteObject> cb_object;

struct TVtxPosClr {
  VEC3 pos;
  VEC4 color;
  TVtxPosClr(VEC3 new_pos, VEC4 new_color) : pos(new_pos), color(new_color){}
};

// ---------------------------------------------------
CRenderMesh* createAxis() {
  CRenderMesh* mesh = new CRenderMesh;
  // Axis aligned X,Y,Z of sizes 1,2,3
  float vertices[] =
  {
    0.0f, 0.0f, 0.0f,  1, 0, 0, 1,
    1.0f, 0.0f, 0.0f,  1, 0, 0, 1,
    0.0f, 0.0f, 0.0f,  0, 1, 0, 1,
    0.0f, 2.0f, 0.0f,  0, 1, 0, 1,
    0.0f, 0.0f, 0.0f,  0, 0, 1, 1,
    0.0f, 0.0f, 3.0f,  0, 0, 1, 1,
  };
  if (!mesh->create(vertices, sizeof(vertices), "PosClr", CRenderMesh::LINE_LIST))
    return nullptr;
  return mesh;
}

// ---------------------------------------------------
CRenderMesh* createGridXZ( int nsteps ) {
  CRenderMesh* mesh = new CRenderMesh;

  std::vector<TVtxPosClr> vtxs;
  VEC4 clr2(0.5f, 0.5f, 0.5f, 1.0f);
  VEC4 clr1(0.25f, 0.25f, 0.25f, 1.0f);
  for (int i = -nsteps; i <= nsteps; ++i) {
    VEC4 clr = (i % 5) ? clr1 : clr2;
    float fi = (float)i;
    float fnsteps = (float)nsteps;
    vtxs.emplace_back(VEC3(fi, 0, fnsteps), clr);
    vtxs.emplace_back(VEC3(fi, 0, -fnsteps), clr);
    vtxs.emplace_back(VEC3(fnsteps, 0, fi), clr);
    vtxs.emplace_back(VEC3(-fnsteps, 0, fi), clr);
  }

  if (!mesh->create(vtxs.data(), vtxs.size() * sizeof(TVtxPosClr), "PosClr", CRenderMesh::LINE_LIST))
    return nullptr;
  return mesh;
}

// --------------------------
void registerMesh( CRenderMesh* new_mesh, const char* name ) {
  new_mesh->setNameAndClass(name, getResourceClassOf<CRenderMesh>());
  Resources.registerResource(new_mesh);
}

bool createRenderObjects() {

  registerMesh(createAxis(), "axis.mesh");
  registerMesh(createGridXZ(20), "grid.mesh");

  return true;
}

void destroyRenderObjects() {
}

void activateCamera(const CCamera& camera) {
  cb_camera.camera_view = camera.getView();
  cb_camera.camera_proj = camera.getProjection();
  cb_camera.camera_pos = camera.getPosition();
  cb_camera.updateGPU();
}

