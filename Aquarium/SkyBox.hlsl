cbuffer cbPerObject : register( b0 )
{
	row_major matrix g_mWorldViewProjection : packoffset( c0 );
}

TextureCube g_EnvironmentTexture : register( t0 );
SamplerState g_sampleState : register( s0 );

struct SkyBoxVS_Input
{
	float4 Pos : POSITION;
};

struct SkyBoxVS_Output
{
	float4 Pos : SV_POSITION;
	float3 Tex : TEXCOORD0;
};

SkyBoxVS_Output SkyBoxVS( SkyBoxVS_Input input )
{
	SkyBoxVS_Output output;

	output.Pos = input.Pos;
	output.Tex = normalize( mul( input.Pos, g_mWorldViewProjection ) );
	output.Tex.yz = output.Tex.zy;

	return output;
}

float4 SkyBoxPS( SkyBoxVS_Output input ) : SV_TARGET
{
	float4 color = g_EnvironmentTexture.Sample( g_sampleState, input.Tex );
	return color;
}
