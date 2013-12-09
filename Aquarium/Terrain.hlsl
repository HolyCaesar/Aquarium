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


Texture2D DepthTexture : register( t11 );
Texture2D ReflectionTexture : register( t12 );
Texture2D WaterNormalMapTexture : register( t13 );
Texture2D MainTexture : register( t14 );
Texture2D RefractionTexture;
Texture2D NormalTex;


SamplerState samGeneral : register( s0 );
SamplerState samAnisotropicWarp : register( s1 );

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
	float  alpha_value  : ALPHA;
};

struct WATER_PSINPUT
{
	float4 position : SV_Position;
	centroid float2 texcoord : TEXCOORD0;
	centroid float3 normal   : NORMAL;
	float3 positionW : TEXCOORD1;
};

/*
* Constant buffers
*/
cbuffer ConstantMatrix : register( b0 )
{
	matrix mView;
	matrix mWorld;
	matrix mProjection;
	float2 cbWaterBumpTexcoordShift;

	matrix mModelViewMatrix;
	matrix mModelViewProjectionMatrix;
	matrix mModelViewProjectionMatrixInv;
	float3 mCameraPosition;
	float3 mCameraDirection;
	matrix mLightModelViewProjectionMatrix;
	matrix mLightModelViewProjectionMatrixInv;
	float fHalfSpaceCullSign;
	float fHalfSpaceCullPosition;
	float2 fScreenSizeInv;
};

cbuffer QuadObject
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
	color.rgb = MainTexture.SampleLevel( samGeneral, float2( ( input.texcoord.x - 0.5 ) / MainBufferSizeMultiplier + 0.5f, ( input.texcoord.y - 0.5 ) / MainBufferSizeMultiplier + 0.5f ), 0 ).rgb;
	color.a=0;
	return color;
}


PSINPUT RenderTerrainVS( VSINPUT input )
{
	PSINPUT output = ( PSINPUT )0;

	input.position.w = 1;
	//output.position = mul( input.position, mWorld );
	//output.position = mul( output.position, mView );
	//output.position = mul( output.position, mProjection );

	output.position = mul( input.position, mModelViewProjectionMatrix );

	output.texcoord = input.texcoord;
	output.alpha_value = input.position.y;
	output.normal   = float3( 0, 1, 0 );

	return output;
}


float4 RenderTerrainPS( PSINPUT input ) : SV_Target
{
	float4 final_color = float4( 0, 0, 0, 0 );

	// Sand and grass (Alpha texture mixing)
	float alpha = ( input.alpha_value - 0.7 ) / 2;
	alpha = clamp( alpha, 0, 1 );
	final_color = ( 1.0f - alpha ) * m_pSand_diffuse_textureSRV.Sample( samGeneral, input.texcoord ) + alpha * m_pGrass_diffuse_textureSRV.Sample( samGeneral, input.texcoord );

	// Multi-texture 
	if( input.alpha_value > terrain_slope_grass_start )
	{
		final_color = final_color * 0.55 + m_pSlope_diffuse_textureSRV.Sample( samGeneral, input.texcoord ) * 0.45;
	}

	if( input.alpha_value > 4.0 )
	{
		final_color = final_color * 0.3 + m_pRock_diffuse_textureSRV.Sample( samGeneral, input.texcoord ) * 0.7;
	}

	return final_color;
}

//*********************************
// Water shader
//*********************************
WATER_PSINPUT RenderWaterVS( VSINPUT input )
{
	WATER_PSINPUT output = ( WATER_PSINPUT )0;

	input.position.w = 1;
	//output.position = mul( input.position, mWorld );
	//output.position = mul( output.position, mView );
	//output.position = mul( output.position, mProjection );

	output.position = mul( input.position, mModelViewProjectionMatrix );

	output.texcoord = input.texcoord;
	output.positionW = input.position.xyz;
	output.normal   = float3( 0, 1, 0 );

	return output;
}


float4 RenderWaterPS( WATER_PSINPUT input ) : SV_Target
{
	// Temp variables
	float3 LightPosition = float3( -2.0, 5.0, 3.0 );
	float3 WaterDeepColor = float3( 0.1, 0.4, 0.7 );

	float4 final_color = float4( 1, 1, 1, 1 );
	float3 pixel_to_light_vector = normalize( LightPosition - input.positionW );
	float3 pixel_to_eye_vector = normalize( mCameraPosition - input.positionW );
	float3 reflected_eye_to_pixel_vector;
	float3 microbump_normal; 
	float3x3 normal_rotation_matrix;

	float fresnel_factor;
	float diffuse_factor;
	float specular_factor;
	////float scatter_factor;
	////float4 refraction_color;
	float4 reflection_color;
	float4 disturbance_eyespace;

	////float water_depth;
	float4 water_color;

	// Control parameters
	float   TerrainSpecularIntensity = 0.5;
	float3  WaterScatterColor={ 0.3, 0.7, 0.6 };
	float3  WaterSpecularColor={ 1, 1, 1 };
	float   WaterSpecularIntensity = 350.0;

	float   WaterSpecularPower = 1000;
	float2  WaterColorIntensity = { 0.1, 0.2 };

	// calculating pixel position in light space
	float4 positionLS = mul( float4( input.positionW, 1 ), mLightModelViewProjectionMatrix );
	positionLS.xyz /= positionLS.w;
	positionLS.x = ( positionLS.x + 1 ) * 0.5;
	positionLS.y = ( 1 - positionLS.y ) * 0.5;

	// calculating shadow multiplier to be applied to diffuse/scatter/specular light components
	//float dsf = 1.0f / 4096.0f;
	//float shadow_factor = 0.2 * g_DepthTexture.SampleCmp( SamplerDepthAnisotropic,positionLS.xy,positionLS.z* 0.995f ).r;
	//shadow_factor += 0.2 * g_DepthTexture.SampleCmp( SamplerDepthAnisotropic, positionLS.xy + float2( dsf, dsf ), positionLS.z * 0.995f ).r;
	//shadow_factor += 0.2 * g_DepthTexture.SampleCmp( SamplerDepthAnisotropic, positionLS.xy + float2( -dsf, dsf ), positionLS.z * 0.995f ).r;
	//shadow_factor += 0.2 * g_DepthTexture.SampleCmp( SamplerDepthAnisotropic, positionLS.xy + float2( dsf, -dsf ), positionLS.z * 0.995f ).r;
	//shadow_factor += 0.2 * g_DepthTexture.SampleCmp( SamplerDepthAnisotropic, positionLS.xy + float2( -dsf, -dsf ), positionLS.z * 0.995f ).r;

	// TODO shadow_factor temperarily
	float shadow_factor = 1.0;

	// need more high frequency bumps for plausible water surface, so creating normal defined by 2 instances of same bump texture
	microbump_normal = normalize( 2 * m_pWater_bump_textureSRV.Sample( samAnisotropicWarp, input.texcoord - cbWaterBumpTexcoordShift * 0.2 ).gbr - float3( 1, -8, 1 ) );
	microbump_normal+= normalize( 2 * m_pWater_bump_textureSRV.Sample( samAnisotropicWarp, input.texcoord * 0.5 + cbWaterBumpTexcoordShift * 0.05 ).gbr - float3( 1, -8, 1 ) );

	// calculating base normal rotation matrix
	normal_rotation_matrix[ 1 ]=input.normal.xyz;
	normal_rotation_matrix[ 2 ]=normalize( cross( float3(0.0,0.0,-1.0),normal_rotation_matrix[1] ) );
	normal_rotation_matrix[ 0 ]=normalize( cross( normal_rotation_matrix[2],normal_rotation_matrix[1] ) );

	// applying base normal rotation matrix to high frequency bump normal
	microbump_normal=mul( normalize( microbump_normal ), normal_rotation_matrix );

	//// simulating scattering/double refraction: light hits the side of wave, travels some distance in water, and leaves wave on the other side
	//// it's difficult to do it physically correct without photon mapping/ray tracing, so using simple but plausible emulation below
	//
	//// only the crests of water waves generate double refracted light
	//scatter_factor=2.5*max(0,input.positionWS.y*0.25+0.25);

	//// the waves that lie between camera and light projection on water plane generate maximal amount of double refracted light 
	//scatter_factor*=shadow_factor*pow(max(0.0,dot(normalize(float3(pixel_to_light_vector.x,0.0,pixel_to_light_vector.z)),-pixel_to_eye_vector)),2.0);
	//
	//// the slopes of waves that are oriented back to light generate maximal amount of double refracted light 
	//scatter_factor*=pow(max(0.0,1.0-dot(pixel_to_light_vector,microbump_normal)),8.0);
	//
	//// water crests gather more light than lobes, so more light is scattered under the crests
	//scatter_factor+=shadow_factor*1.5*g_WaterColorIntensity.y*max(0,input.positionWS.y+1)*
	//// the scattered light is best seen if observing direction is normal to slope surface
	//max(0,dot(pixel_to_eye_vector,microbump_normal))*
	//// fading scattered light out at distance and if viewing direction is vertical to avoid unnatural look
	//max(0,1-pixel_to_eye_vector.y)*(300.0/(300+length(g_CameraPosition-input.positionWS)));

	//// fading scatter out by 90% near shores so it looks better
	//scatter_factor*=0.1+0.9*input.depthmap_scaler.g;

	// calculating fresnel factor 
	float r = ( 1.2 - 1.0 ) / ( 1.2 + 1.0 );
	fresnel_factor = max( 0.0, min( 1.0, r + ( 1.0 - r ) * pow( 1.0 - dot( microbump_normal, pixel_to_eye_vector ), 4 ) ) );

	// Calculating specular factor
	reflected_eye_to_pixel_vector = -pixel_to_eye_vector + 2 * dot( pixel_to_eye_vector, microbump_normal ) * microbump_normal;
	specular_factor = shadow_factor * fresnel_factor * pow( max( 0, dot( pixel_to_light_vector, reflected_eye_to_pixel_vector ) ), WaterSpecularPower );

	// calculating diffuse intensity of water surface itself
	diffuse_factor = WaterColorIntensity.x + WaterColorIntensity.y * max( 0, dot( pixel_to_light_vector, microbump_normal ) );

	// calculating disturbance which has to be applied to planar reflections/refractions to give plausible results
	disturbance_eyespace = mul( float4( microbump_normal.x, 0, microbump_normal.z, 0 ), mModelViewMatrix );
	float2 reflection_disturbance = float2( disturbance_eyespace.x, disturbance_eyespace.z ) * 0.03;
	// fading out reflection disturbance at distance so reflection doesn't look noisy at distance
	float2 refraction_disturbance = float2( -disturbance_eyespace.x, disturbance_eyespace.y ) * 0.05  * ( 20.0 / ( 20 + length( mCameraPosition - input.positionW ) ) );

	// calculating correction that shifts reflection up/down according to water wave Y position
	float4 projected_waveheight = mul( float4( input.positionW.x, input.positionW.y, input.positionW.z, 1 ), mModelViewProjectionMatrix );
	float waveheight_correction= -0.5 * projected_waveheight.y / projected_waveheight.w;
	projected_waveheight = mul( float4( input.positionW.x, -0.8, input.positionW.z, 1 ), mModelViewProjectionMatrix );
	waveheight_correction += 0.5 * projected_waveheight.y / projected_waveheight.w;
	reflection_disturbance.y = max( -0.15, waveheight_correction + reflection_disturbance.y );

	//// picking refraction depth at non-displaced point, need it to scale the refraction texture displacement amount according to water depth
	//float refraction_depth=GetRefractionDepth(input.position.xy*g_ScreenSizeInv);
	//refraction_depth=g_ZFar*g_ZNear/(g_ZFar-refraction_depth*(g_ZFar-g_ZNear));
	//float4 vertex_in_viewspace=mul(float4(input.positionWS,1),g_ModelViewMatrix);
	//water_depth=refraction_depth-vertex_in_viewspace.z;
	//float nondisplaced_water_depth=water_depth;
	//
	//// scaling refraction texture displacement amount according to water depth, with some limit
	//refraction_disturbance*=min(2,water_depth);

	//// picking refraction depth again, now at displaced point, need it to calculate correct water depth
	//refraction_depth=GetRefractionDepth(input.position.xy*g_ScreenSizeInv+refraction_disturbance);
	//refraction_depth=g_ZFar*g_ZNear/(g_ZFar-refraction_depth*(g_ZFar-g_ZNear));
	//vertex_in_viewspace=mul(float4(input.positionWS,1),g_ModelViewMatrix);
	//water_depth=refraction_depth-vertex_in_viewspace.z;

	//// zeroing displacement for points where displaced position points at geometry which is actually closer to the camera than the water surface
	//float conservative_refraction_depth=GetConservativeRefractionDepth(input.position.xy*g_ScreenSizeInv+refraction_disturbance);
	//conservative_refraction_depth=g_ZFar*g_ZNear/(g_ZFar-conservative_refraction_depth*(g_ZFar-g_ZNear));
	//vertex_in_viewspace=mul(float4(input.positionWS,1),g_ModelViewMatrix);
	//float conservative_water_depth=conservative_refraction_depth-vertex_in_viewspace.z;

	//if(conservative_water_depth<0)
	//{                     
	//	refraction_disturbance=0;
	//	water_depth=nondisplaced_water_depth;
	//}
	//water_depth=max(0,water_depth);

	// getting reflection and refraction color at disturbed texture coordinates
	reflection_color = ReflectionTexture.SampleLevel( samGeneral, float2( input.position.x * fScreenSizeInv.x, 1.0 - input.position.y * fScreenSizeInv.y ) + reflection_disturbance, 0 );
	//refraction_color=g_RefractionTexture.SampleLevel(SamplerLinearClamp,input.position.xy*g_ScreenSizeInv+refraction_disturbance,0);

	water_color = diffuse_factor * float4( WaterDeepColor, 1 );
	//water_color.rgb = lerp( CalculateFogColor( pixel_to_light_vector, pixel_to_eye_vector ).rgb, water_color.rgb, min( 1, exp( -length( g_CameraPosition - input.positionWS ) * g_FogDensity ) ) );

	//// fading fresnel factor to 0 to soften water surface edges
	//fresnel_factor*=min(1,water_depth*5.0);

	//// fading refraction color to water color according to distance that refracted ray travels in water 
	//refraction_color=lerp(water_color,refraction_color,min(1,1.0*exp(-water_depth/8.0)));
	final_color = water_color;
	final_color.a = 1;
	return final_color;
}