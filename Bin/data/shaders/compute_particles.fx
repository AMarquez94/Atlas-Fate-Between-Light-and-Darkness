#include "common.fx"

struct TParticle {
  float3 pos;
  float size;
  
  float3 speed;
  float unit_time;

  float time_factor;
  float dummy1;
  float dummy2;
  float dummy3;
};

// --------------------------------------------------------------
// This is used as a ctebuffer, but not global
struct TCoreSystem {
  float3 center;
  float  delta_time;
  
  float3 speed;
  float  radius;

  float  height;
  float  life_time;
  float  min_size;
  float  max_size;
};


// --------------------------------------------------------------
struct TGpuParticleInstance {
  float3 pos;
  float  size;
};

// --------------------------------------------------------------
[numthreads(8, 8, 1)]
void move_particles(
  uint2 gid : SV_DispatchThreadID,
  StructuredBuffer<TParticle> inState : register(t0),
  StructuredBuffer<TCoreSystem> core : register(t1),      // Like a cte buffer
  RWStructuredBuffer<TParticle> outState : register(u0),
  RWStructuredBuffer<TGpuParticleInstance> outRender : register(u1)
)
{
  float dt = core[0].delta_time;
  uint id = gid.x;

  float3 v = inState[id].pos - core[0].center;
  float3 v_perp = float3(v.z, 0, -v.x);   // Rotate 90o
  v_perp = normalize(v_perp) * 25;

  outState[id].pos = inState[id].pos;// + v_perp * dt;
  outState[id].size = inState[id].size;
  outState[id].speed = v_perp;
  outState[id].unit_time = inState[id].unit_time + dt * inState[id].time_factor;
  outState[id].time_factor = inState[id].time_factor;

  outRender[id].pos = outState[id].pos;
  outRender[id].size = outState[id].size;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
void VS(
  // From Stream 0 from unit_quad_pos_xy.mesh
  in float4 iPos : POSITION

  // From Stream 1 from particles_instanced.mesh
  , in float4 iCenter : TEXCOORD1   // Scale comes in the .w of the iCenter

  // outs
  , out float4 oPos : SV_POSITION
  , out float2 oUV : TEXCOORD0
  , out float4 oColor : TEXCOORD1

)
{
  float3 local_pos = iPos.xyz * 2. - 1.;
  local_pos.xy *= iCenter.w;

  float2 uv = iPos.xy;

  float4 world_pos = float4(iCenter.xyz + float3(0,40,0) + camera_pos
    + (local_pos.x * camera_left + local_pos.y * camera_up)
    , 1);

  oPos = mul(world_pos, camera_view_proj);
  oUV = 1 - uv;
  oColor = float4(1, 1, 1, 1); 
}

//--------------------------------------------------------------------------------------
float4 PS(
  float4 iPos : SV_POSITION
  , float2 iUV : TEXCOORD0
  , float4 iColor : TEXCOORD1
) : SV_Target
{
  float a = txAlbedo.Sample(samLinear, iUV).r;
  return pow( a, 3 );// * sin(global_world_time * 3) * 2;
  float4 texture_color = iColor;
  return texture_color;
}