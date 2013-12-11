cbuffer cbBuffer : register( cb0 )
{
	matrix mWorldMatrix;
	matrix mViewMatrix;
	matrix mProjectionMatrix; 
	matrix mRefelctionMatrix;

	// Control Variables
	float fWaterTranslation;
	float fReflectRefractScale;
	float2 padding;
	float4 clipPlane;
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

	reflectProjectWorld = mul( mWorldMatrix, mRefelctionMatrix );
	reflectProjectWorld = mul( reflectProjectWorld, mProjectionMatrix );

	output.reflectionPosition = mul( input.position, reflectProjectWorld );

	viewProjectWorld = mul( mViewMatrix, mProjectionMatrix );
	viewProjectWorld = mul( mWorldMatrix, viewProjectWorld );

	output.refractionPosition = mul( input.position, viewProjectWorld );

	return output;
}


float4 FlatWaterPS( PSINPUT input ) : SV_TARGET
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
	//refractionColor = refractionTexture.Sample( SampleGen, refractTexCoord );

	// Combine the reflection and refraction pixel using a linear interpolation
	//final_color = lerp( reflectionColor, refractionColor, 0.6f );
	final_color = reflectionColor * 0.6;

	return final_color;
}


// Refraction shader
struct REFRAC_PSINPUT
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float  clip : SV_ClipDistance0;
};

Texture2D shaderTexture : register( t3 );
//SamplerState SampleType;


REFRAC_PSINPUT RefractWaterVS( VSINPUT input )
{
	REFRAC_PSINPUT output = ( REFRAC_PSINPUT )0;

	input.position.w = 1.0f;

	output.position = mul( input.position, mWorldMatrix );
	output.position = mul( output.position, mViewMatrix );
	output.position = mul( output.position, mProjectionMatrix );

	output.tex = input.tex;

	output.normal = mul( input.normal, (float3x3)mWorldMatrix );

	// Normalize the normal
	output.normal = normalize( output.normal );

	// Set up the clipping plane
	output.clip = dot( mul( input.position, mWorldMatrix ), clipPlane );

	return output;
}

float4 RefractWaterPS( REFRAC_PSINPUT input) : SV_TARGET
{
	float4 ambientColor = { 0.1, 0.1, 0.1, 0.1 };
	float4 diffuseColor = { 0.1, 0.1, 0.1, 0.1 };
	float3 lightDirection = { 0.0, -1.0, 0.0 };
	float4 textureColor;
	float3 lightDir;
	float lightIntensity;
	float4 final_color;

	// Sample the texture pixel at this location
	textureColor = shaderTexture.Sample( SampleGen, input.tex );
	final_color = ambientColor;
	lightDir = -lightDirection;

	lightIntensity = saturate( dot( input.normal, lightDir ) );

	if( lightIntensity > 0.0f )
	{
		final_color += ( diffuseColor * lightIntensity );
	}

	// Saturate the final light color
	final_color = saturate( final_color );

	final_color = final_color * textureColor;

	return final_color;
}

cbuffer QuadObject : register( cb1 )
{
    static const float2 QuadVertices[4] =
    {
        {-1.0, -1.0},
        { 1.0, -1.0},
        {-1.0,  1.0},
        { 1.0,  1.0}
    };

    static const float2 QuadTexCoordinates[4] =
    {
        {0.0, 1.0},
        {1.0, 1.0},
        {0.0, 0.0},
        {1.0, 0.0}
    };
}

struct PSIn_Quad
{
    float4 position     : SV_Position;
    float2 texcoord     : TEXCOORD0;
};

PSIn_Quad FullScreenQuadVS( uint VertexId: SV_VertexID )
{
    PSIn_Quad output;

	output.position = float4( QuadVertices[ VertexId ], 0, 1 );
    output.texcoord = QuadTexCoordinates[ VertexId ];
    
    return output;
}

float4 MainToBackBufferPS(PSIn_Quad input) : SV_Target
{
	float4 color;
	float MainBufferSizeMultiplier = 1.1;
	color.rgb = shaderTexture.SampleLevel( SampleGen, float2( ( input.texcoord.x - 0.5 ) / MainBufferSizeMultiplier + 0.5f, ( input.texcoord.y - 0.5 ) / MainBufferSizeMultiplier + 0.5f ), 0 ).rgb;
	//color = shaderTexture.Sample( SampleGen, input.texcoord );
	color.a=0;

	//color = float4( 1.0, 0.0, 0.0, 0.0 );

	return color;
}