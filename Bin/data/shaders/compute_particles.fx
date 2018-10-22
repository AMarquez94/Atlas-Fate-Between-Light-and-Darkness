#include "common.fx"

struct TParticle {
  float3 pos;
  float size;
  
  float3 speed;
  float unit_time;

  float time_factor;
  int pid;
  float rtime;
  float etime;
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
	
	float elapsed_time;
	float ratio_time;
};


// --------------------------------------------------------------
struct TGpuParticleInstance {
  float3 pos;
  float  size;
	float2 data;
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
	outState[id].pid = inState[id].pid;
	outState[id].rtime = inState[id].rtime;
	outState[id].etime = inState[id].etime + dt;
	
  outRender[id].pos = outState[id].pos;
  outRender[id].size = outState[id].size;
	
	// Dirty trick
	float value = (outState[id].etime > outState[id].rtime) ? 1 : 0;
	float total_value = (outState[id].etime) / (outState[id].rtime + 1);
	outState[id].etime = (total_value > 1) ? 0 : outState[id].etime;
	outRender[id].data = float2(value * clamp(outState[id].etime - outState[id].rtime, 0, 1), dt);
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
void VS(
  // From Stream 0 from unit_quad_pos_xy.mesh
  in float4 iPos : POSITION

  // From Stream 1 from particles_instanced.mesh
  , in float4 iCenter : TEXCOORD1   // Scale comes in the .w of the iCenter
	, in float2 iSize : TEXCOORD2   // Some extra data to tweak the intensity
	
  // outs
  , out float4 oPos : SV_POSITION
  , out float2 oUV : TEXCOORD0
  , out float4 oColor : TEXCOORD1
	, out float2 oSize : TEXCOORD2
)
{
  float3 local_pos = iPos.xyz * 2. - 1.;
  local_pos.xy *= iCenter.w + iSize.x * 0.05;

  float2 uv = iPos.xy;

  float4 world_pos = float4(iCenter.xyz + float3(0,40,0) + camera_pos
    + (local_pos.x * camera_left + local_pos.y * camera_up)
    , 1);

  oPos = mul(world_pos, camera_view_proj);
  oUV = 1 - uv;
  oColor = float4(1, 1, 1, 1);
	oSize = iSize;
}

//--------------------------------------------------------------------------------------
float4 PS(
  float4 iPos : SV_POSITION
  , float2 iUV : TEXCOORD0
  , float4 iColor : TEXCOORD1
	, float2 iSize : TEXCOORD2
) : SV_Target
{
  float a = txAlbedo.Sample(samLinear, iUV).r;
	float d = txGBufferLinearDepth.Load(uint3(iPos.xy, 0)).x;
			
  return pow( a, 3 )* ( 1 + sin(iSize.x * 3) * 3) * d;
}