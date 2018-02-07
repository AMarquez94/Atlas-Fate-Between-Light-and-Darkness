#include "mcv_platform.h"

VEC3 getVectorFromYaw(float yaw) {
	return VEC3(sinf(yaw), 0.f, cosf(yaw));
}

float getYawFromVector(VEC3 front) {
	return atan2f(front.x, front.z);
}

VEC3 getVectorFromYawPitch(float yaw, float pitch) {
	return VEC3(
		  sinf(yaw) * cosf( pitch )
		,             sinf( pitch )
		, cosf(yaw) * cosf( pitch )
	);
}

void getYawPitchFromVector(VEC3 front, float* yaw, float* pitch) {
	*yaw = atan2f(front.x, front.z);
	float mdo_xz = sqrtf(front.x*front.x + front.z*front.z);
	*pitch = atan2f(front.y, mdo_xz);
}

