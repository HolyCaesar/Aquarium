/*
* Terrain render shader
*/

// Static textures
Texture2D HeightfieldTex;
Texture2D LayerdefTex;
Texture2D NormalTex;

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
	centroid float3 positionWS : TEXCOORD1;
	centroid float4 layerdef : TEXCOORD2;
	centroid float4 depthmap_scaler : TEXCOORD3;
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

/*
* Constant buffers
*/

PSINPUT RenderTerrainVS( VSINPUT input )
{
	PSINPUT output = ( PSINPUT )0;
	input.position.w = 1;
	output.position = mul( input.position, mWorld );
	output.position = mul( output.position, mView );
	output.position = mul( output.position, mProjection );
	output.texcoord = input.texcoord;
	output.normal   = float3( 0, 0, 0 );
	output.positionWS = input.position;
	output.layerdef  = float4( 0, 0, 0, 0 );
	output.depthmap_scaler = float4( 0, 0, 0, 0);
	return output;
}



float4 RenderTerrainPS( PSINPUT input ) : SV_Target
{
	return float4( 1, 1, 1, 0 );
}