#pragma once

class CHandle;

class CCamera {

	VEC3 pos;         // eye
	VEC3 target;
	VEC3 up_aux;

	// 3 orthonormals axis in world space
	VEC3 front;
	VEC3 up;
	VEC3 left;

	// Prepective params
	float fov_vertical = deg2rad(60.f);     // in radians!!
	float z_near = 1.f;
	float z_far = 1000.f;
	float aspect_ratio = 1.f;

	// Ortographic params
	float orto_size;

	// Matrix
	MAT44 view;
	MAT44 proj;
	MAT44 view_proj;

	bool is_ortho = false;

	void updateViewProj();

	// viewport
	struct TViewport {
		int x0 = 0;
		int y0 = 0;
		int width = 640;
		int height = 480;
	};
	TViewport viewport;

public:

	enum CType { PERSPECTIVE, ORTOGRAPHIC } type;

	CCamera();

	// 
	static CHandle main;

	VEC3 getPosition() const { return pos; }
	VEC3 getTarget() const { return target; }

	VEC3 getFront() const { return front; }
	VEC3 getUp() const { return up; }
	VEC3 getLeft() const { return left; }

	// -------------------------------------
	MAT44 getView() const {
		return view;
	}

	MAT44 getProjection() const {
		return proj;
	}

	MAT44 getViewProjection() const {
		return view_proj;
	}

    void setFov(float new_fov) {
        fov_vertical = new_fov;
    }

	// -------------------------------------
	float getFov() const { return fov_vertical; }
	float getZNear() const { return z_near; }
	float getZFar() const { return z_far; }
	float getOrtosize() const { return orto_size; }
	float getAspectRatio() const { return aspect_ratio; }

	// -------------------------------------
	void lookAt(VEC3 new_pos, VEC3 new_target, VEC3 new_up_aux = VEC3(0, 1, 0));
	void setPerspective(float new_fov_vertical, float new_z_near, float new_z_far);
	void setOrtographic(float ortosize, float new_z_near, float new_z_far);
	void setOrthographicGUI(float width, float height);
	MAT44 loadOrtho(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
	void setViewport(int x0, int y0, int width, int height);

	bool getScreenCoordsOfWorldCoord(VEC3 world_pos, VEC3 *screen_coords) const;

	bool getWorldCoordOfScreenCoords(VEC3 pos, VEC3 *screen_coords) const;
};