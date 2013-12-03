/*
* Terrain render shader
*/

// Static textures
Texture2D HeightfieldTex : register( t0 );
Texture2D LayerdefTex : register( t1 );
Texture2D m_pRock_bump_textureSRV : register( t2 );
Texture2D m_pRock_microbump_textureSRV : register( t3 );
Texture2D m_pRock_diffuse_textureSRV : register( t4 );
Texture2D m_pSand_bump_textureSRV : register( t5 );
Texture2D m_pSand_microbump_textureSRV : register( t6 );
Texture2D m_pSand_diffuse_textureSRV : register( t7 );
Texture2D m_pGrass_diffuse_textureSRV : register( t8 );
Texture2D m_pSlope_diffuse_textureSRV : register( t9 );
Texture2D m_pWater_bump_textureSRV : register( t10 );
Texture2D NormalTex;

SamplerState samGeneral : register( s0 );

// rendertarget textures
Texture2D MainTex;

/*
* Terrain shader input/output data structures
*/
struct VSINPUT
{
	float4 position : POSITION;
	float3 normal   : NORMAL;
	float2 texcoord : TEXCOORD;
};

struct PSINPUT
{
	float4 position : SV_Position;
	centroid float2 texcoord : TEXCOORD0;
	centroid float3 normal   : NORMAL;
	float z_value : HEIGHT;
	int  alphaFlag    : ALPHA;
};

/*
* Constant buffers
*/
cbuffer ConstantMatrix : register( b0 )
{
	matrix mView;
	matrix mWorld;
	matrix mProjection;
};

#define terrain_height_underwater_start		-100.0f
#define terrain_height_underwater_end		-8.0f
#define terrain_height_sand_start			-30.0f
#define terrain_height_sand_end				1.7f
#define terrain_height_grass_start			1.7f
#define terrain_height_grass_end			30.0f
#define terrain_height_rocks_start			-2.0f
#define terrain_height_trees_start			4.0f
#define terrain_height_trees_end			30.0f
#define terrain_slope_grass_start			0.96f
#define terrain_slope_rocks_start			0.85f

PSINPUT RenderTerrainVS( VSINPUT input )
{
	PSINPUT output = ( PSINPUT )0;

	input.position.w = 1;
	output.position = mul( input.position, mWorld );
	output.position = mul( output.position, mView );
	output.position = mul( output.position, mProjection );
	output.texcoord = input.texcoord;
	output.normal   = float3( 0, 1, 0 );
	output.z_value = input.position.y;

	return output;
}

float4 RenderTerrainPS( PSINPUT input ) : SV_Target
{
	//return float4( 1, 1, 1, 0 );
	float4 final_color = float4( 0, 0, 0, 0 );
	float alpha = ( input.z_value + 3.3 ) / 10.0;
	alpha = clamp( alpha, 0, 1 );

	final_color = m_pGrass_diffuse_textureSRV.Sample( samGeneral, input.texcoord )*alpha + (1.0f-alpha)* m_pSand_diffuse_textureSRV.Sample( samGeneral, input.texcoord );
	
	return final_color;
}