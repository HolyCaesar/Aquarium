cbuffer cbBuffer
{
	matrix mWorldMatrix;
	matrix mViewMatrix;
	matrix mProjectionMatrix; 
	matrix mRefelctionMatrix;

	// Control Variables
	float fWaterTranslation;
	float fReflectRefractScale;
	//float2 padding;
}

struct VSINPUT
{
	float4 position : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
};

struct PSINPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	float4 reflectionPosition : TEXCOORD1;
	float4 refractionPosition : TEXCOORD2;
};

SamplerState SampleGen : register( s0 );

Texture2D reflectionTexture : register( t0 );
Texture2D refractionTexture : register( t1 );
Texture2D normalTexture : register( t2 );


PSINPUT FlatWaterVS( VSINPUT input )
{
	PSINPUT output = ( PSINPUT )0;
	matrix reflectProjectWorld;
	matrix viewProjectWorld;

	input.position.w = 1.0f;

	output.position = mul( input.position, mWorldMatrix );
	output.position = mul( output.position, mViewMatrix );
	output.position = mul( output.position, mProjectionMatrix );

	output.tex = input.tex;

	reflectProjectWorld = mul( mRefelctionMatrix, mProjectionMatrix );
	reflectProjectWorld = mul( mWorldMatrix, reflectProjectWorld );

	output.reflectionPosition = mul( input.position, reflectProjectWorld );

	viewProjectWorld = mul( mViewMatrix, mProjectionMatrix );
	viewProjectWorld = mul( mWorldMatrix, viewProjectWorld );

	output.refractionPosition = mul( input.position, viewProjectWorld );

	return output;
}


float4 FlatWaterPS( PSINPUT input ) : SV_POSITION
{
	float4 final_color = { 1.0f, 1.0f, 1.0f, 1.0f };
	float2 reflectTexCoord;
	float2 refractTexCoord;
	float4 normalMap;
	float3 normal;
	float4 reflectionColor;
	float4 refractionColor;

	// Move the texture of water
	input.tex.y += fWaterTranslation;

	// Calculate the projected reflectiont texture coordinates
	// TODO divide by 2.0f maybe because 2 is the length of that square water
	reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
	reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;

	// Calculate the projected refraction texture coordinates
	refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
	refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

	// Sample the normal from the normal map texture
	normalMap = normalTexture.Sample( SampleGen, input.tex );
	
	// Expand the range of the normal from (0, 1 ) to ( -1, 1 )
	normal = ( normalMap.xyz * 2.0f ) - 1.0f;

	// Distort the reflection and refraction coordinates by the normal map value for 
	// creating the rippling effect
	reflectTexCoord = reflectTexCoord + ( normal.xy * fReflectRefractScale );
	refractTexCoord = refractTexCoord + ( normal.xy * fReflectRefractScale );

	reflectionColor = reflectionTexture.Sample( SampleGen, reflectTexCoord );
	refractionColor = refractionTexture.Sample( SampleGen, refractTexCoord );

	// Combine the reflection and refraction pixel using a linear interpolation
	final_color = lerp( reflectionColor, refractionColor, 0.6f );

	return final_color;
}