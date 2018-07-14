#ifndef INC_RENDER_CTES_H_
#define INC_RENDER_CTES_H_

#define CB_CAMERA       0
#define CB_OBJECT       1
#define CB_SKIN_BONES	2
#define CB_LIGHT		3
#define CB_GLOBALS      4
#define CB_MATERIAL     5
#define CB_BLUR         6
#define CB_BLOOM        7
#define CB_GUI		    8
#define CB_PARTICLE		9
#define CB_OUTLINE      10
#define CB_PLAYER       11
#define CB_POSTFX       12

// -------------------------------------------------
// Texture Slots
// Materials
#define TS_ALBEDO              0
#define TS_NORMAL              1
#define TS_METALLIC            2
#define TS_ROUGHNESS           3
#define TS_EMISSIVE            4
#define TS_AOCCLUSION          5
#define TS_HEIGHT              6
#define TS_LIGHTMAP            7

// .. Other slots of the material
#define TS_NUM_MATERIALS_SLOTS 8

// Here goes the slots not associated to a material
#define TS_LIGHT_PROJECTOR     9
#define TS_LIGHT_SHADOW_MAP    10 
#define TS_LIGHT_VOLUME_MAP    11 
#define TS_ENVIRONMENT_MAP     12
#define TS_IRRADIANCE_MAP      13
#define TS_NOISE_MAP           14
#define TS_NOISE_MAP2          15

#define TS_DEFERRED_ALBEDOS           16
#define TS_DEFERRED_NORMALS           17
#define TS_DEFERRED_LINEAR_DEPTH      18
#define TS_DEFERRED_ACC_LIGHTS        19
#define TS_DEFERRED_SELF_ILLUMINATION 20
#define TS_DEFERRED_AO                21
#define TS_LUT_COLOR_GRADING          22

#define TS_FIRST_SLOT_MATERIAL_0      TS_ALBEDO
#define TS_FIRST_SLOT_MATERIAL_1      TS_ALBEDO1
#define TS_FIRST_SLOT_MATERIAL_2      TS_ALBEDO2

#define TS_ALBEDO1 23
#define TS_NORMAL1 24
#define TS_ALBEDO2 25
#define TS_NORMAL2 26

#define TS_MIX_BLEND_WEIGHTS          25

// -------------------------------------------------
#define RO_COMPLETE     0
#define RO_ALBEDO       1
#define RO_NORMAL       2
#define RO_ROUGHNESS    3
#define RO_METALLIC     4
#define RO_WORLD_POS    5
#define RO_DEPTH_LINEAR 6
#define RO_AO			7

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
#define VEC2                      float2
#define SLOT(slot_number)                 : register(t##slot_number)

#endif

CB_DEF(CCteCamera, CB_CAMERA)   // Generates the b0
{
    MAT44 camera_view;
    MAT44 camera_proj;
    MAT44 camera_view_proj;
    MAT44 camera_screen_to_world;
    MAT44 camera_proj_with_offset;      // Used by ao
    VEC3  camera_pos;
    float camera_dummy1;
    VEC3  camera_front;
    float camera_dummy2;
    VEC3  camera_left;
    float camera_dummy3;
    VEC3  camera_up;
    float camera_dummy4;

    float camera_zfar;
    float camera_znear;
    float camera_tan_half_fov;
    float camera_aspect_ratio;

    VEC2  camera_inv_resolution;
    float camera_dummy5;
    float camera_dummy6;
};

CB_DEF(CCteObject, CB_OBJECT)   // Generates the b1
{
    MAT44  obj_world;
    VEC4   obj_color;
    VEC4   self_color;
    float  self_intensity;
    float  self_opacity;
    float  self_dummy2;
    float  self_dummy3;
};

CB_DEF(CCteLight, CB_LIGHT)
{
    VEC4  light_color;
    VEC4  light_direction;
    VEC3  light_pos;
    float light_intensity;

    // Align x4
    float light_shadows_step;
    float light_shadows_inverse_resolution;     // 1.0f / 256.0
    float light_shadows_step_with_inv_res;      // light_shadows_step * light_shadows_inverse_resolution
    float light_radius;                         // Used by point lights

    //Some spotlight related values.
    float light_angle;
    float light_inner_cut;
    float light_outer_cut;
    float far_atten;
    
    float inner_atten;
    float dummy_light1;
    float dummy_light2;
    float dummy_light3;

    MAT44 light_view_proj_offset;
    MAT44 light_view_proj;
};

CB_DEF(CCteGlobals, CB_GLOBALS)   // Generates the b1
{
    float global_world_time;
    float global_exposure_adjustment;
    float global_ambient_adjustment;
    float global_hdr_enabled;
    float global_gamma_correction_enabled;
    float global_tone_mapping_mode;
    int   global_render_output;
    float global_fog_density;
    VEC3  global_fog_color;

    float global_shared_fx_amount;
    float global_shared_fx_val1;
    float global_shared_fx_val2;
    float global_shared_fx_val3;

    float global_self_intensity;
    float global_fog_start;
    float global_fog_end;
    float global_player_speed;
    float global_fog_end3;
};

CB_DEF(CCteMaterial, CB_MATERIAL)
{
    float  scalar_roughness;
    float  scalar_metallic;
    float  scalar_irradiance_vs_mipmaps;
    float  scalar_emission;
    VEC4   color_emission;
    VEC4   color_material;

    float  mix_boost_r;
    float  mix_boost_g;
    float  mix_boost_b;
    float  mat_alpha_outline;
};

CB_DEF(CCteBlur, CB_BLUR)
{
    VEC4 blur_w;        // weights
    VEC4 blur_d;        // distances
    VEC2 blur_step;     // Extra modifier
    VEC2 blur_center; // To keep aligned x4
};

CB_DEF(CCteSkinBones, CB_SKIN_BONES)
{
    MAT44 Bones[MAX_SUPPORTED_BONES];
    float BonesScale;
    float BonesDummy1;
    float BonesDummy2;
    float BonesDummy3;
};

CB_DEF(CCteBloom, CB_BLOOM)
{
    VEC4  bloom_weights;
    float bloom_threshold_min;
    float bloom_threshold_max;
    float bloom_pad1;
    float bloom_pad2;
};

CB_DEF(CCteGUI, CB_GUI)
{
	VEC2 minUV;
	VEC2 maxUV;
	VEC4 tint_color;
};

CB_DEF(CCteParticle, CB_PARTICLE)
{
    VEC2 particle_minUV;
    VEC2 particle_maxUV;
    VEC4 particle_color;
};

CB_DEF(CCteOutline, CB_OUTLINE)
{
    float  linear_time;
    float  outline_alpha;
    float  outline_dummy1;
    float  outline_dummy2;
};

CB_DEF(CCtePlayer, CB_PLAYER)
{
    float  player_fade;
    float  player_aberration;
    float  player_speed;
    float  player_dummy2;
};

CB_DEF(CCtePostFX, CB_POSTFX)
{
    float  postfx_ca_amount;
    float  postfx_ca_offset;
    float  postfx_cs_offset;
    float  postfx_dummy3;
};


#endif
