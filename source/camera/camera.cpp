#include "mcv_platform.h"
#include "camera.h"

CCamera::CCamera() {
	aspect_ratio = 1.f;
	setPerspective(deg2rad(60.f), 1.0f, 1000.f);
	lookAt(VEC3(1, 0, 0), VEC3(0, 0, 0), VEC3(0, 1, 0));
}

void CCamera::updateViewProj() {
  view_proj = view * proj;
}

// 
void CCamera::lookAt(VEC3 new_pos, VEC3 new_target, VEC3 new_up_aux) {
  
	pos = new_pos;
	target = new_target;
	up_aux = new_up_aux;

	front = (target - pos);
	front.Normalize();

	left = new_up_aux.Cross(front);
	left.Normalize();

	up = front.Cross(left);

	view = MAT44::CreateLookAt(pos, target, up_aux);
	updateViewProj();
}

void CCamera::setPerspective(float new_fov_vertical, float new_z_near, float new_z_far) {
	fov_vertical = new_fov_vertical;
	z_near = new_z_near;
	z_far = new_z_far;
	assert(z_far > z_near);
	proj = MAT44::CreatePerspectiveFieldOfView(new_fov_vertical, (float)Render.width / (float)Render.height, new_z_near, new_z_far);
	updateViewProj();
}

bool CCamera::getScreenCoordsOfWorldCoord(VEC3 world_pos, VEC3* result) const {

	// It's also dividing by w  -> [-1..1]
	VEC3 pos_in_homo_space = VEC3::Transform(world_pos, getViewProjection());

	VEC3 pos_in_screen_space(
		(pos_in_homo_space.x + 1.0f) * 0.5f * Render.width,
		(1.0f - pos_in_homo_space.y) * 0.5f * Render.height,
		pos_in_homo_space.z
	);

	assert(result);
	*result = pos_in_screen_space;

	// Return true if the coord is inside the frustum
	return pos_in_homo_space.x >= -1.0f && pos_in_homo_space.x <= 1.0f
		&& pos_in_homo_space.y >= -1.0f && pos_in_homo_space.y <= 1.0f
		&& pos_in_homo_space.z >= 0.0f && pos_in_homo_space.z <= 1.0f
		;
}

