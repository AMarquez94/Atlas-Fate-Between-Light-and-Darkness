#ifndef INC_RENDER_CTES_H_
#define INC_RENDER_CTES_H_

#define CB_CAMERA       0
#define CB_OBJECT       1
#define CB_SKIN_BONES	2
#define CB_LIGHT		3

// -------------------------------------------------
// Texture Slots
// Materials
#define TS_ALBEDO              0
#define TS_LIGHTMAP            1
#define TS_NORMAL              2
// .. Other slots of the material
#define TS_NUM_MATERIALS_SLOTS 3

// Here goes the slots not associated to a material
#define TS_LIGHT_PROJECTOR     5
#define TS_LIGHT_SHADOW_MAP    6 
#define TS_ENVIRONMENT_MAP     7

// -------------------------------------------------
#define MAX_SUPPORTED_BONES        128

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
#define SLOT(slot_number)                 : register(t##slot_number)

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

	// Align x4
	float light_shadows_step;
	float light_shadows_inverse_resolution;     // 1.0f / 256.0
	float light_shadows_step_with_inv_res;      // light_shadows_step * light_shadows_inverse_resolution
	float light_dummy2;                           // To keep aligment at 4 floats
												  // Align x4
	MAT44 light_view_proj_offset;

};

CB_DEF(CCteSkinBones, CB_SKIN_BONES)
{
	MAT44 Bones[MAX_SUPPORTED_BONES];
};

#endif
