#ifndef INC_RENDER_CTE_BUFFER_H_
#define INC_RENDER_CTE_BUFFER_H_

// -----------------------------------------
class CCteBuffer {

protected:
	ID3D11Buffer * cb = nullptr;
	int                slot = -1;
	bool createData(UINT num_bytes, const char* new_name);

public:

	void destroy();
	void activate() const;
};
// -----------------------------------------
// I'm a join combination of the CPU struct (TPOD) and 
// the struct (CCteBuffer) which have the members to have a 
// version of myself in the GPU.
// -----------------------------------------
template< typename TPOD >
class CRenderCte : public TPOD, public CCteBuffer {
	const char* name = nullptr;
public:

	CRenderCte(const char* new_name) : name(new_name) {
	}

	bool create(int new_slot) {
		slot = new_slot;
		return createData(sizeof(TPOD), name);
	}

	void updateGPU() const {
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

