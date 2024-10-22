#pragma once

#define deg2rad(_deg)  ( (_deg) * (float) M_PI / 180.f)
#define rad2deg(_rad)  ( (_rad) / (float) M_PI * 180.f)

VEC3 getVectorFromYaw(float yaw);
float getYawFromVector(VEC3 front);

VEC3 getVectorFromYawPitch(float yaw, float pitch);
void getYawPitchFromVector(VEC3 front, float* yaw, float* pitch);

VEC3 projectVector(const VEC3 & vector, const VEC3 & normal);
QUAT createLookAt(const VEC3& origin, const VEC3& front, const VEC3& up);

VEC3 rotateVectorAroundAxis(const VEC3& vecToRotate, const VEC3& axisToRotate, float deltaRotation);