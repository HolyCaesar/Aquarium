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

	if( input.position.y >= terrain_height_sand_start && input.position.y <= terrain_height_sand_end )
	{
		output.alphaFlag = 1;
	}
	else if( input.position.y >= terrain_height_grass_start && input.position.y <= terrain_height_grass_end )
	{
		output.alphaFlag = 2;
	}
	else if( input.position.y >= terrain_height_trees_start && input.position.y <= terrain_height_trees_end )
	{
		output.alphaFlag = 3;
	}
	else if( input.position.y >= terrain_height_underwater_start && input.position.y <= terrain_height_underwater_end )
	{
		output.alphaFlag = 4;
	}
	else if( input.position.y >= terrain_height_sand_end && input.position.y <= terrain_slope_grass_start )
	{
		output.alphaFlag = 5;
	}
	else if( input.position.y >= terrain_height_rocks_start && input.position.y <= terrain_slope_rocks_start )
	{
		output.alphaFlag = 6;
	}

	input.position.w = 1;
	output.position = mul( input.position, mWorld );
	output.position = mul( output.position, mView );
	output.position = mul( output.position, mProjection );
	output.texcoord = input.texcoord;
	output.normal   = float3( 0, 1, 0 );


	return output;
}



float4 RenderTerrainPS( PSINPUT input ) : SV_Target
{
	//return float4( 1, 1, 1, 0 );
	float4 final_color = float4( 0, 0, 0, 0 );
	if( input.alphaFlag == 1 )
	{
		final_color = m_pSand_diffuse_textureSRV.Sample( samGeneral, input.texcoord );
	}
	else if( input.alphaFlag == 2 )
	{
		final_color = m_pGrass_diffuse_textureSRV.Sample( samGeneral, input.texcoord );
	}
	else if( input.alphaFlag == 3 )
	{
		final_color = m_pGrass_diffuse_textureSRV.Sample( samGeneral, input.texcoord );
	}
	else if( input.alphaFlag == 4 )
	{
		final_color = m_pSand_diffuse_textureSRV.Sample( samGeneral, input.texcoord );
	}
	else if( input.alphaFlag == 5 )
	{
		final_color = m_pSlope_diffuse_textureSRV.Sample( samGeneral, input.texcoord );
	}
	else if( input.alphaFlag == 6 )
	{
		final_color = m_pRock_diffuse_textureSRV.Sample( samGeneral, input.texcoord );
	}

	return final_color;
}