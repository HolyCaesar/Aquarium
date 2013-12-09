#pragma once
#include "Helpers.h"

#define terrain_gridpoints					512
#define terrain_numpatches_1d				64
#define terrain_geometry_scale				1.0f
#define terrain_maxheight					40.0f 
#define terrain_minheight					-50.0f 
#define terrain_fractalfactor				0.68f;
#define terrain_fractalinitialvalue			100.0f
#define terrain_smoothfactor1				0.99f
#define terrain_smoothfactor2				0.10f
#define terrain_rockfactor					0.95f
#define terrain_smoothsteps					40

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

#define terrain_far_range terrain_gridpoints*terrain_geometry_scale

#define shadowmap_resource_buffer_size_xy				4096
#define water_normalmap_resource_buffer_size_xy			2048
#define terrain_layerdef_map_texture_size				1024
#define terrain_depth_shadow_map_texture_size			512

#define main_buffer_size_multiplier			1.1f
#define reflection_buffer_size_multiplier   1.1f
#define refraction_buffer_size_multiplier   1.1f

#define scene_z_near						1.0f
#define scene_z_far							25000.0f
#define camera_fov							110.0f

class SkyBox;

class Terrain
{
	struct TERRAIN_VERTEX
	{
		XMFLOAT4 position;
		XMFLOAT3 normal;
		XMFLOAT2 texcoord;
	};

	struct CONSTANT_BUFFER
	{
		XMMATRIX mView;
		XMMATRIX mWorld;
		XMMATRIX mProjection;

		// Control variable
		XMFLOAT2 mWaterTexcoordShift;

		XMMATRIX mModelViewMatrix;
		XMMATRIX mModelViewProjectionMatrix;
		XMMATRIX mModelViewProjectionMatrixInv;
		XMVECTOR mCameraPosition;
		XMVECTOR mCameraDirection;
		XMMATRIX mLightModelViewProjectionMatrix;
		XMMATRIX mLightModelViewProjectionMatrixInv;
		float fHalfSpaceCullSign;
		float fHalfSpaceCullPosition;
		XMFLOAT2 fScreenSizeInv;
	};
public:
	Terrain(void);
	~Terrain(void);

	HRESULT Initialize( ID3D11Device* device );
	void OnD3D11DestroyDevice();
	void ReCreateBuffers();
	HRESULT LoadTextures();
	void Render( CModelViewerCamera *cam, ID3D11DeviceContext* pd3dImmediateContext, XMMATRIX* pRotate, float fTime, SkyBox *sb );
	HRESULT CreateTerrain();

	float DynamicTesselationFactor;
	float StaticTesselationFactor;
	void SetupNormalView( CFirstPersonCamera *cam );
	void SetupReflectionView( CFirstPersonCamera *cam );
	void SetupRefractionView( CFirstPersonCamera *cam );
	void SetupLightView( CFirstPersonCamera *cam );

	void SetupNormalView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam );
	void SetupReflectionView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam );
	void SetupRefractionView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam );
	void SetupLightView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam );

	HRESULT CreateRenderState( ID3D11Device* device ); 
	float BackbufferWidth;
	float BackbufferHeight;

	/*
	* Getters and Setters
	*/
	ID3D11InputLayout* GetTerrainInputLayout()
	{
		return m_pHeightfield_inputlayout;
	}
	ID3D11Buffer* GetTerrainVertexBuffer()
	{
		return m_pHeightfield_vertexbuffer;
	}
	ID3D11Buffer* GetTerrainIndexBuffer()
	{
		return m_pHeightfield_indexbuffer;
	}
	unsigned int GetIndexCount()
	{
		return m_iIndexCount;
	}

	UINT MultiSampleCount;
	UINT MultiSampleQuality;

	//ID3D11Texture2D		*m_pRock_bump_texture;
	ID3D11ShaderResourceView *m_pRock_bump_textureSRV;

	//ID3D11Texture2D		*m_pRock_microbump_texture;
	ID3D11ShaderResourceView *m_pRock_microbump_textureSRV;

	//ID3D11Texture2D		*m_pRock_diffuse_texture;
	ID3D11ShaderResourceView *m_pRock_diffuse_textureSRV;	

	//ID3D11Texture2D		*m_pSand_bump_texture;
	ID3D11ShaderResourceView *m_pSand_bump_textureSRV;

	//ID3D11Texture2D		*m_pSand_microbump_texture;
	ID3D11ShaderResourceView *m_pSand_microbump_textureSRV;

	//ID3D11Texture2D		*m_pSand_diffuse_texture;
	ID3D11ShaderResourceView *m_pSand_diffuse_textureSRV;	

	//ID3D11Texture2D		*m_pGrass_diffuse_texture;
	ID3D11ShaderResourceView *m_pGrass_diffuse_textureSRV;	

	//ID3D11Texture2D		*m_pSlope_diffuse_texture;
	ID3D11ShaderResourceView *m_pSlope_diffuse_textureSRV;	

	//ID3D11Texture2D		*m_pWater_bump_texture;
	ID3D11ShaderResourceView *m_pWater_bump_textureSRV;	

	ID3D11Texture2D			 *m_pReflection_color_resource;
	ID3D11ShaderResourceView *m_pReflection_color_resourceSRV;
	ID3D11RenderTargetView   *m_pReflection_color_resourceRTV;
	ID3D11Texture2D			 *m_pRefraction_color_resource;
	ID3D11ShaderResourceView *m_pRefraction_color_resourceSRV;
	ID3D11RenderTargetView   *m_pRefraction_color_resourceRTV;

	ID3D11Texture2D			 *m_pShadowmap_resource;
	ID3D11ShaderResourceView *m_pShadowmap_resourceSRV;
	ID3D11DepthStencilView   *m_pShadowmap_resourceDSV;

	ID3D11Texture2D			 *m_pReflection_depth_resource;
	ID3D11DepthStencilView   *m_pReflection_depth_resourceDSV;

	ID3D11Texture2D			 *m_pRefraction_depth_resource;
	ID3D11RenderTargetView   *m_pRefraction_depth_resourceRTV;
	ID3D11ShaderResourceView *m_pRefraction_depth_resourceSRV;

	ID3D11Texture2D			 *m_pWater_normalmap_resource;
	ID3D11ShaderResourceView *m_pWater_normalmap_resourceSRV;
	ID3D11RenderTargetView   *m_pWater_normalmap_resourceRTV;

	ID3D11Texture2D			 *m_pMain_color_resource;
	ID3D11ShaderResourceView *m_pMain_color_resourceSRV;
	ID3D11RenderTargetView   *m_pMain_color_resourceRTV;
	ID3D11Texture2D			 *m_pMain_depth_resource;
	ID3D11DepthStencilView   *m_pMain_depth_resourceDSV;
	ID3D11ShaderResourceView *m_pMain_depth_resourceSRV;
	ID3D11Texture2D			 *m_pMain_color_resource_resolved;
	ID3D11ShaderResourceView *m_pMain_color_resource_resolvedSRV;

	ID3D11Device* m_pDevice;
	ID3D11RasterizerState* m_pRasterizerState;

	ID3D11SamplerState*	m_pGeneralTexSS;

	float				height[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];
	XMFLOAT3			normal[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];
	XMFLOAT3			tangent[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];
	XMFLOAT3			binormal[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];

	ID3D11Texture2D		*m_pHeightmap_texture;
	ID3D11ShaderResourceView *m_pHeightmap_textureSRV;

	ID3D11Texture2D		*m_pLayerdef_texture;
	ID3D11ShaderResourceView *m_pLayerdef_textureSRV;

	ID3D11Texture2D		*m_pDepthmap_texture;
	ID3D11ShaderResourceView *m_pDepthmap_textureSRV;

	ID3D11Buffer		*m_pHeightfield_vertexbuffer;
	ID3D11Buffer		*m_pHeightfield_indexbuffer;
	ID3D11Buffer		*m_pWater_vertexbuffer;
	ID3D11Buffer		*m_pWater_indexbuffer;

	unsigned int         m_iIndexCount;
	unsigned int         m_iWaterIndexCount;

	ID3D11InputLayout   *m_pHeightfield_inputlayout;
	ID3D11InputLayout   *m_pTrianglestrip_inputlayout;

	ID3D11VertexShader  *m_pRenderTerrainVS;
	ID3D11PixelShader   *m_pRenderTerrainPS;

	ID3D11VertexShader  *m_pRenderWaterVS;
	ID3D11PixelShader   *m_pRenderWaterPS;

	ID3D11VertexShader  *m_pRenderMainVS;
	ID3D11PixelShader   *m_pRenderMainPS;

	CONSTANT_BUFFER		m_CBallInOne;
	ID3D11Buffer*		m_pCBallInOne;

	/*
	* Water Stuff
	*/
	ID3D11SamplerState *m_pAnisotropicWrapTexSS;
};

float bilinear_interpolation( float fx, float fy, float a, float b, float c, float d );