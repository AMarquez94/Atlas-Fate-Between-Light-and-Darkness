#include "mcv_platform.h"
#include "comp_camera.h"
#include "comp_transform.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("camera", TCompCamera);

void TCompCamera::debugInMenu() {

	float fov_deg = rad2deg(getFov());
	float new_znear = getZNear();
	float new_zfar = getZFar();
	float orto_size = getOrtosize();

	if (type == CCamera::CType::PERSPECTIVE) {
		bool changed = ImGui::DragFloat("Fov", &fov_deg, 0.1f, 30.f, 175.f);
		changed |= ImGui::DragFloat("Z Near", &new_znear, 0.001f, 0.01f, 1.0f);
		changed |= ImGui::DragFloat("Z Far", &new_zfar, 1.0f, 2.0f, 3000.0f);
		if (changed)
			setPerspective(deg2rad(fov_deg), new_znear, new_zfar);
	}
	else {
		bool changed = ImGui::DragFloat("Ortosize", &orto_size, 1.f, 1.f, 200.f);
		changed |= ImGui::DragFloat("Z Near", &new_znear, 0.001f, 0.01f, 1.0f);
		changed |= ImGui::DragFloat("Z Far", &new_zfar, 1.0f, 2.0f, 3000.0f);
		if (changed)
			setOrtographic(orto_size, new_znear, new_zfar);
	}

	ImGui::LabelText("AspectRatio", "%f", getAspectRatio());
}

void TCompCamera::renderDebug() {

	MAT44 inv_view_proj = getViewProjection().Invert();

	auto mesh = Resources.get("unit_frustum.mesh")->as<CRenderMesh>();
	mesh->activate();
	
	// Sample several times to 'view' the z distribution along the 3d space
	const int nsamples = 10;
	for (int i = 1; i < nsamples; ++i) {
		float f = (float)i / (float)(nsamples - 1);
		MAT44 world = MAT44::CreateScale(1.f, 1.f, f) * inv_view_proj;
		setWorldTransform(world, VEC4(1, 1, 1, 1));
		mesh->render();
	}

}


void TCompCamera::load(const json& j, TEntityParseContext& ctx) {

	float fov_deg = j.value("fov", rad2deg(getFov()));
	float z_near = j.value("z_near", getZNear());
	float z_far = j.value("z_far", getZFar());
	float orto_size = j.value("ortosize", 10.f);
	const std::string type = j.value("proj_type", "perspective");

	if (type == "perspective") {
		this->type = PERSPECTIVE;
		setPerspective(deg2rad(fov_deg), z_near, z_far);
	}
	else {
		this->type = ORTOGRAPHIC;
		setOrtographic(orto_size, z_near, z_far);
	}

}

void TCompCamera::update(float dt) {

	TCompTransform* c = get<TCompTransform>();
	if (!c)
		return;
	this->lookAt(c->getPosition(), c->getPosition() + c->getFront(), c->getUp());

}

