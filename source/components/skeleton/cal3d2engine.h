#ifndef INC_CAL3D_2_ENGINE_H_
#define INC_CAL3D_2_ENGINE_H_

// Cal2DX conversions, VEC3 are the same, QUAT must change the sign of w
CalVector DX2Cal(VEC3 p);
CalQuaternion DX2Cal(QUAT q);
VEC3 Cal2DX(CalVector p);
QUAT Cal2DX(CalQuaternion q);

#endif



