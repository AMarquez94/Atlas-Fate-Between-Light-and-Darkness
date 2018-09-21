#include "common.fx"

//--------------------------------------------------------------------------------------
struct VS_TEXTURED_OUTPUT
{
  float4 Pos:     SV_POSITION;

  float3 decal_top_left : TEXCOORD0;
  float3 decal_axis_x : TEXCOORD1;
  float3 decal_axis_z : TEXCOORD2;

  float  opacity: TEXCOORD3;
  float2 uv:      TEXCOORD4;
	
	float3 normal:   NORMAL0;
	float4 tangent:   NORMAL1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_TEXTURED_OUTPUT VS(

  // From stream 0 we read the instanced mesh (the billboard)
  in float4 iPos     : POSITION           // Stream 0
, in float3 iNormal  : NORMAL0
, in float2 iTex0    : TEXCOORD0
, in float2 iTex1    : TEXCOORD1
, in float4 iTangent : NORMAL1

  // From stream 1 we read the instance information 
, in TInstanceWorldData instance_data     // Stream 1
)
{
  VS_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;
  
  // Build a World matrix from the instance information
  float4x4 instance_world = getWorldOfInstance(instance_data);

  // Transform local vertex of the box to the world coordinates
  float4 world_pos = mul(float4(iPos.xyz,1), instance_world);
 
  // Concatenate with the instance world transform  
  // world_pos = mul(world_pos, obj_world);

  // Transform it to camera projective space
	output.Pos = mul( world_pos, camera_view_proj );

  // From the world matrix, extract the main axis x & z, and the center
  float3 center  = float3( instance_world[3][0], instance_world[3][1], instance_world[3][2] );
  float3 decal_x = float3( instance_world[0][0], instance_world[0][1], instance_world[0][2] );
	float3 decal_y = float3( instance_world[1][0], instance_world[1][1], instance_world[1][2] );
  float3 decal_z = float3( instance_world[2][0], instance_world[2][1], instance_world[2][2] );
		
  // Precompute in the vs the axis to compute the local coords from world coords 
  output.decal_top_left = center - decal_x * 0.5 - decal_z * 0.5;
  float decal_inv_size = 1 / ( dot( decal_x, decal_x ) );
	float decal_inv_size_x = 1 / (length(decal_x) * length(decal_x));
	float decal_inv_size_z = 1 / (length(decal_z) * length(decal_z));
	
  output.decal_axis_x = decal_x * decal_inv_size_x;
  output.decal_axis_z = decal_z * decal_inv_size_z;

	float3x3 fMatrix = { decal_x / length(decal_x), decal_y/ length(decal_y), decal_z/length(decal_z) };   
									 
  // Blendout in the last TimeBlendingOut secs of TimeToLife
  //float TimeToLife = InstanceXtras.x;
  //float TimeBlendingOut = InstanceXtras.y;
  //output.opacity = smoothstep( 0, TimeBlendingOut, TimeToLife );
  output.opacity = 1;

	output.normal = mul(iNormal, (float3x3)fMatrix);
	output.tangent.xyz = mul(iTangent.xyz, (float3x3)fMatrix);
	output.tangent.w = iTangent.w;
  output.uv = iTex0;

  return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
void PS(
		in VS_TEXTURED_OUTPUT input
	, out float4 o_albedo : SV_Target0
	, out float4 o_normal : SV_Target1
  )
{
  float2 iPosition = input.Pos.xy;
  int3 ss_load_coords = uint3(iPosition, 0);

  // Recuperar la posicion de mundo para ese pixel
  float  zlinear = txGBufferLinearDepth.Load(ss_load_coords).x;
  float3 wPos = getWorldCoords(iPosition, zlinear);

  // Convert to local unit decal space
  float3 decal_top_left_to_wPos = wPos - input.decal_top_left;
  float amount_of_x = dot( decal_top_left_to_wPos, input.decal_axis_x); 
  float amount_of_z = dot( decal_top_left_to_wPos, input.decal_axis_z); 

  float4 decal_color = txAlbedo.Sample(samBorderLinear, float2(amount_of_x,amount_of_z));
	float roughness = txRoughness.Sample(samLinear, float2(amount_of_x,amount_of_z)).r;	
	float3 N = computeNormalMap(input.normal, input.tangent, float2(amount_of_x,amount_of_z));
	o_normal = encodeNormal(N, roughness * decal_color.a);
  o_albedo.xyz = decal_color.xyz;
  o_albedo.a = decal_color.a;//txMetallic.Sample(samLinear, float2(amount_of_x,amount_of_z)).r;
 
  // Change to true 'see' the boxes 
  if( false ) {
    o_albedo.a += 0.3;
   
    if( (input.uv.x < 0.01 || input.uv.x > 0.99 ) || (input.uv.y < 0.01 || input.uv.y > 0.99 ) )
      o_albedo.a = 1.0;
  }
}

