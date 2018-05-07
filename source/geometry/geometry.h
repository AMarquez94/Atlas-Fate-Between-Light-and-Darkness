#ifndef INC_GEOMETRY_GEOMETRY_H_
#define INC_GEOMETRY_GEOMETRY_H_

#include <d3d11.h>
#include "SimpleMath.h"
using namespace DirectX::SimpleMath;

typedef Vector2     VEC2;
typedef Vector3     VEC3;
typedef Vector4     VEC4;
typedef Matrix      MAT44;
typedef Quaternion  QUAT;
typedef DirectX::BoundingBox AABB;

#include "angular.h"

VEC2 loadVEC2(const json& j);
VEC3 loadVEC3(const json& j);
VEC4 loadVEC4(const json& j);
QUAT loadQUAT(const json& j);

#include "geometry/transform.h"

template<typename T>
T clamp(const T& value, const T& minValue, const T& maxValue)
{
	return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

template<typename T>
T lerp(const T& minValue, const T& maxValue, float ratio)
{
	return minValue + (maxValue - minValue) * ratio;
}

#endif

