#ifndef INC_RENDER_CTE_BUFFER_H_
#define INC_RENDER_CTE_BUFFER_H_

// -----------------------------------------
class CCteBuffer {

protected:
  ID3D11Buffer*      cb = nullptr;
  int                slot = -1;
  bool createData(UINT num_bytes);

public:

  void destroy();
  void activate();
};

// -----------------------------------------
// I'm a join combination of the CPU struct (TPOD) and 
// the struct (CCteBuffer) which have the members to have a 
// version of myself in the GPU.
// -----------------------------------------
template< typename TPOD >
class CRenderCte : public TPOD, public CCteBuffer {
public:
  bool create(int new_slot) {
    slot = new_slot;
    return createData(sizeof(TPOD));
  }

  void updateGPU() {
    const TPOD* pod = this;
    Render.ctx->UpdateSubresource(cb, 0, NULL, pod, 0, 0);
  }
};

/*
CRenderCte<CCteCamera>  cb_camera;
cb_camera.create( SLOT_CAMERA );
cb_camera.world = ...
cb_camera.updateGPU();
cb_camera.activate();
*/

#endif

