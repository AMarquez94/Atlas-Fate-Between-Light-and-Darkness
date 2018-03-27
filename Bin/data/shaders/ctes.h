
#define CB_CAMERA    0
#define CB_OBJECT    1
#define CB_LIGHT     3
#ifdef WIN32

#define CB_DEF(xName,idx)   struct xName

#else

// The register ctes in DX11 MUST be called b0, b1, b2, b3....
// b##idx generates the value b##0 -> b0  (## joins to texts)
// CB_CAMERA is defined to be 0 in the top of this document
#define CB_DEF(xName,idx)   cbuffer xName : register(b ## idx)
// cbuffer ConstantBuffer : register(b0)
#define MAT44                     matrix
#define VEC4                      float4
#define VEC3                      float3

#endif

CB_DEF(CCteCamera, CB_CAMERA)   // Generates the b0
{
  MAT44 camera_view;
  MAT44 camera_proj;
  VEC3  camera_pos;
  float camera_dummy;
};

CB_DEF(CCteObject, CB_OBJECT)   // Generates the b1
{
  MAT44 obj_world;
  VEC4  obj_color;
};

CB_DEF(CCteLight, CB_LIGHT)
{
	VEC4  light_color;
	VEC3  light_pos;
	float light_intensity;
	MAT44 light_view_proj_offset;
};