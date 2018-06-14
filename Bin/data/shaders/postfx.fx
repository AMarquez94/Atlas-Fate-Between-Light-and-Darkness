#include "common.fx"

// PostFX Exponential distance fog.
float4 PS_PostFXFog(in float4 iPosition : SV_POSITION , in float2 iTex0 : TEXCOORD0) : SV_Target
{
	int3 ss_load_coords = uint3(iPosition.xy, 0);
	float depth = txGBufferLinearDepth.Load(ss_load_coords).x;
	float3 wPos = getWorldCoords(iPosition.xy, depth);
  float4 in_color = txAlbedo.Sample(samClampLinear, iTex0.xy);
	
	float3 frag_dir = (wPos - camera_pos.xyz);
	float dist = abs(length(frag_dir));
			
	float fog_factor = 1.0 /exp( (dist * global_fog_density)* (dist * global_fog_density));	
	float4 final_color = lerp(float4(global_fog_color,1), in_color, saturate(fog_factor));
	
	//You have to tweak this values
	 float be = 0.225 * smoothstep(0.0, 6.0, 32.0 - wPos.y);
	 float bi = 0.375* smoothstep(0.0, 80, 10.0 - wPos.y);
	 
	float ext = exp(-dist * be);
	float insc = exp(-dist * bi);

	return in_color + float4(global_fog_color,1) * (1 - insc);
}

// Adaptative lighting methods
