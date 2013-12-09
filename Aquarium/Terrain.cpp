#include "Terrain.h"
#include "SkyBox.h"

int gp_wrap( int a )
{
	if( a < 0 )
	{
		return ( a + terrain_gridpoints );
	}
	if( a >= terrain_gridpoints )
	{
		return ( a - terrain_gridpoints );
	}
	return a;
}

Terrain::Terrain(void)
{
	//m_pRock_bump_texture = NULL;
	m_pRock_bump_textureSRV = NULL;

	//m_pRock_microbump_texture = NULL;
	m_pRock_microbump_textureSRV = NULL;

	//m_pRock_diffuse_texture = NULL;
	m_pRock_diffuse_textureSRV = NULL;	

	//m_pSand_bump_texture = NULL;
	m_pSand_bump_textureSRV = NULL;

	//m_pSand_microbump_texture = NULL;
	m_pSand_microbump_textureSRV = NULL;

	//m_pSand_diffuse_texture = NULL;
	m_pSand_diffuse_textureSRV = NULL;	

	//m_pGrass_diffuse_texture = NULL;
	m_pGrass_diffuse_textureSRV = NULL;	

	//m_pSlope_diffuse_texture = NULL;
	m_pSlope_diffuse_textureSRV = NULL;	

	//m_pWater_bump_texture = NULL;
	m_pWater_bump_textureSRV = NULL;	

	m_pReflection_color_resource = NULL;
	m_pReflection_color_resourceSRV = NULL;
	m_pReflection_color_resourceRTV = NULL;
	m_pRefraction_color_resource = NULL;
	m_pRefraction_color_resourceSRV = NULL;
	m_pRefraction_color_resourceRTV = NULL;

	m_pShadowmap_resource = NULL;
	m_pShadowmap_resourceSRV = NULL;
	m_pShadowmap_resourceDSV = NULL;

	m_pReflection_depth_resource = NULL;
	m_pReflection_depth_resourceDSV = NULL;

	m_pRefraction_depth_resource = NULL;
	m_pRefraction_depth_resourceRTV = NULL;
	m_pRefraction_depth_resourceSRV = NULL;

	m_pWater_normalmap_resource = NULL;
	m_pWater_normalmap_resourceSRV = NULL;
	m_pWater_normalmap_resourceRTV = NULL;

	m_pMain_color_resource = NULL;
	m_pMain_color_resourceSRV = NULL;
	m_pMain_color_resourceRTV = NULL;
	m_pMain_depth_resource = NULL;
	m_pMain_depth_resourceDSV = NULL;
	m_pMain_depth_resourceSRV = NULL;
	m_pMain_color_resource_resolved = NULL;
	m_pMain_color_resource_resolvedSRV = NULL;

	m_pDevice = NULL;
	m_pRasterizerState = NULL;

	//height[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];
	//normal[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];
	//tangent[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];
	//binormal[ terrain_gridpoints + 1 ][ terrain_gridpoints + 1 ];

	m_pHeightmap_texture = NULL;
	m_pHeightmap_textureSRV = NULL;

	m_pLayerdef_texture = NULL;
	m_pLayerdef_textureSRV = NULL;

	m_pDepthmap_texture = NULL;
	m_pDepthmap_textureSRV = NULL;

	m_pHeightfield_vertexbuffer = NULL;
	m_pHeightfield_indexbuffer = NULL;

	m_pHeightfield_inputlayout = NULL;
	m_pTrianglestrip_inputlayout = NULL;

	m_pRenderTerrainVS = NULL;
	m_pRenderTerrainPS = NULL;
	m_pRenderWaterVS = NULL;
	m_pRenderWaterPS = NULL;

	m_pCBallInOne = NULL;
	m_iIndexCount = 0;
	m_iWaterIndexCount = 0;

	m_pGeneralTexSS = NULL;
	m_pAnisotropicWrapTexSS = NULL;

	MultiSampleCount = 1;
	MultiSampleQuality = 0;
}

Terrain::~Terrain(void)
{
}

HRESULT Terrain::CreateRenderState( ID3D11Device* device )
{
	HRESULT hr = S_OK;

	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID /*D3D11_FILL_WIREFRAME*/;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FrontCounterClockwise = false;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = false;
	rasterizerState.MultisampleEnable = true;
	rasterizerState.AntialiasedLineEnable = false;

	device->CreateRasterizerState( &rasterizerState, &m_pRasterizerState );

	return hr;
}

HRESULT Terrain::Initialize( ID3D11Device* device )
{
	HRESULT hr = S_OK;
	m_pDevice = device;

	const D3D11_INPUT_ELEMENT_DESC TriangleLayout[] = 
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3DBlob* pVSBlob = NULL, *pPSBlob = NULL;
	V_RETURN( DXUTCompileFromFile( L"Terrain.hlsl", nullptr, "RenderTerrainVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob ) );
	V_RETURN( device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pRenderTerrainVS ) );
	V_RETURN( device->CreateInputLayout( TriangleLayout, ARRAYSIZE( TriangleLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pHeightfield_inputlayout ) );
	//V_RETURN( device->CreateInputLayout( TriangleLayout, ARRAYSIZE( TerrainLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pHeightfield_inputlayout ) );
	V_RETURN( DXUTCompileFromFile( L"Terrain.hlsl", nullptr, "RenderTerrainPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob ) );
	V_RETURN( device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pRenderTerrainPS ) );

	pVSBlob->Release();
	pPSBlob->Release();

	V_RETURN( DXUTCompileFromFile( L"Terrain.hlsl", nullptr, "RenderWaterVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob ) );
	V_RETURN( device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pRenderWaterVS ) );
	V_RETURN( device->CreateInputLayout( TriangleLayout, ARRAYSIZE( TriangleLayout ), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pTrianglestrip_inputlayout ) );
	V_RETURN( DXUTCompileFromFile( L"Terrain.hlsl", nullptr, "RenderWaterPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob ) );
	V_RETURN( device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pRenderWaterPS ) );

	pVSBlob->Release();
	pPSBlob->Release();

	V_RETURN( DXUTCompileFromFile( L"Terrain.hlsl", nullptr, "FullScreenQuadVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob ) );
	V_RETURN( device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pRenderMainVS ) );
	V_RETURN( DXUTCompileFromFile( L"Terrain.hlsl", nullptr, "MainToBackBufferPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob ) );
	V_RETURN( device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pRenderMainPS ) );

	pVSBlob->Release();
	pPSBlob->Release();

	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = sizeof( CONSTANT_BUFFER );
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;
	V_RETURN( m_pDevice->CreateBuffer( &bd, NULL, &m_pCBallInOne ) );
	DXUT_SetDebugName( m_pCBallInOne, "m_pCBallInOne");

	hr = CreateTerrain();
	hr = CreateRenderState( device );
	hr = LoadTextures();

	return hr;
}

void Terrain::ReCreateBuffers()
{
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;
	D3D11_DEPTH_STENCIL_VIEW_DESC DSV_desc;

	SAFE_RELEASE( m_pMain_color_resource );
	SAFE_RELEASE( m_pMain_color_resourceSRV );
	SAFE_RELEASE( m_pMain_color_resourceRTV );

	SAFE_RELEASE( m_pMain_color_resource_resolved );
	SAFE_RELEASE( m_pMain_color_resource_resolvedSRV );

	SAFE_RELEASE( m_pMain_depth_resource );
	SAFE_RELEASE( m_pMain_depth_resourceDSV );
	SAFE_RELEASE( m_pMain_depth_resourceSRV );

	SAFE_RELEASE( m_pReflection_color_resource );
	SAFE_RELEASE( m_pReflection_color_resourceSRV );
	SAFE_RELEASE( m_pReflection_color_resourceRTV );
	SAFE_RELEASE( m_pRefraction_color_resource );
	SAFE_RELEASE( m_pRefraction_color_resourceSRV );
	SAFE_RELEASE( m_pRefraction_color_resourceRTV );

	SAFE_RELEASE( m_pReflection_depth_resource );
	SAFE_RELEASE( m_pReflection_depth_resourceDSV );
	SAFE_RELEASE( m_pRefraction_depth_resource );
	SAFE_RELEASE( m_pRefraction_depth_resourceRTV );
	SAFE_RELEASE( m_pRefraction_depth_resourceSRV );

	SAFE_RELEASE( m_pShadowmap_resource );
	SAFE_RELEASE( m_pShadowmap_resourceDSV );
	SAFE_RELEASE( m_pShadowmap_resourceSRV );

	SAFE_RELEASE( m_pWater_normalmap_resource );
	SAFE_RELEASE( m_pWater_normalmap_resourceSRV );
	SAFE_RELEASE( m_pWater_normalmap_resourceRTV );

	HRESULT hr = S_OK;
	// recreating main color buffer
	ZeroMemory( &textureSRV_desc,sizeof(textureSRV_desc) );
	ZeroMemory( &tex_desc,sizeof(tex_desc) );

	tex_desc.Width              = (UINT)(BackbufferWidth * main_buffer_size_multiplier );
	tex_desc.Height             = (UINT)(BackbufferHeight * main_buffer_size_multiplier );
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	hr = m_pDevice->CreateTexture2D         ( &tex_desc, NULL, &m_pMain_color_resource );
	hr = m_pDevice->CreateShaderResourceView( m_pMain_color_resource, &textureSRV_desc, &m_pMain_color_resourceSRV );
	hr = m_pDevice->CreateRenderTargetView  ( m_pMain_color_resource, NULL, &m_pMain_color_resourceRTV );

	ZeroMemory( &textureSRV_desc,sizeof(textureSRV_desc) );
	ZeroMemory( &tex_desc,sizeof(tex_desc) );

	tex_desc.Width              = (UINT)(BackbufferWidth * main_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight * main_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	m_pDevice->CreateTexture2D         ( &tex_desc, NULL, &m_pMain_color_resource_resolved );
	m_pDevice->CreateShaderResourceView( m_pMain_color_resource_resolved, &textureSRV_desc, &m_pMain_color_resource_resolvedSRV );

	// recreating main depth buffer
	ZeroMemory( &textureSRV_desc,sizeof(textureSRV_desc) );
	ZeroMemory( &tex_desc,sizeof(tex_desc) );

	tex_desc.Width              = (UINT)( BackbufferWidth * main_buffer_size_multiplier );
	tex_desc.Height             = (UINT)( BackbufferHeight * main_buffer_size_multiplier );
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count   = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	DSV_desc.Format  = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	textureSRV_desc.Texture2D.MipLevels		  = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	m_pDevice->CreateTexture2D( &tex_desc, NULL, &m_pMain_depth_resource );
	m_pDevice->CreateDepthStencilView( m_pMain_depth_resource, &DSV_desc, &m_pMain_depth_resourceDSV );
	m_pDevice->CreateShaderResourceView( m_pMain_depth_resource, &textureSRV_desc, &m_pMain_depth_resourceSRV );

	// recreating reflection and refraction color buffers
	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	tex_desc.Width              = (UINT)(BackbufferWidth * reflection_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight * reflection_buffer_size_multiplier);
	tex_desc.MipLevels          = (UINT)max(1,log(max((float)tex_desc.Width,(float)tex_desc.Height))/(float)log(2.0f));
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	m_pDevice->CreateTexture2D         ( &tex_desc, NULL, &m_pReflection_color_resource );
	m_pDevice->CreateShaderResourceView( m_pReflection_color_resource, &textureSRV_desc, &m_pReflection_color_resourceSRV );
	m_pDevice->CreateRenderTargetView  ( m_pReflection_color_resource, NULL, &m_pReflection_color_resourceRTV );


	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	tex_desc.Width              = (UINT)(BackbufferWidth * refraction_buffer_size_multiplier );
	tex_desc.Height             = (UINT)(BackbufferHeight * refraction_buffer_size_multiplier );
	tex_desc.MipLevels          = (UINT)max( 1,log(max((float)tex_desc.Width,(float)tex_desc.Height))/(float)log(2.0f) );
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	m_pDevice->CreateTexture2D         ( &tex_desc, NULL, &m_pRefraction_color_resource );
	m_pDevice->CreateShaderResourceView( m_pRefraction_color_resource, &textureSRV_desc, &m_pRefraction_color_resourceSRV );
	m_pDevice->CreateRenderTargetView  ( m_pRefraction_color_resource, NULL, &m_pRefraction_color_resourceRTV );

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	// recreating reflection and refraction depth buffers

	tex_desc.Width              = (UINT)(BackbufferWidth*reflection_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*reflection_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	DSV_desc.Format  = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	m_pDevice->CreateTexture2D( &tex_desc, NULL, &m_pReflection_depth_resource );
	m_pDevice->CreateDepthStencilView( m_pReflection_depth_resource, &DSV_desc, &m_pReflection_depth_resourceDSV );


	tex_desc.Width              = (UINT)(BackbufferWidth*refraction_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*refraction_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_FLOAT;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels       = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	m_pDevice->CreateTexture2D( &tex_desc, NULL, &m_pRefraction_depth_resource );
	m_pDevice->CreateRenderTargetView  ( m_pRefraction_depth_resource, NULL, &m_pRefraction_depth_resourceRTV );
	m_pDevice->CreateShaderResourceView( m_pRefraction_depth_resource, &textureSRV_desc, &m_pRefraction_depth_resourceSRV );

	// recreating shadowmap resource
	tex_desc.Width              = shadowmap_resource_buffer_size_xy;
	tex_desc.Height             = shadowmap_resource_buffer_size_xy;
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	DSV_desc.Format  = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice=0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels       = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	m_pDevice->CreateTexture2D( &tex_desc, NULL, &m_pShadowmap_resource);	
	m_pDevice->CreateShaderResourceView( m_pShadowmap_resource, &textureSRV_desc, &m_pShadowmap_resourceSRV );
	m_pDevice->CreateDepthStencilView( m_pShadowmap_resource, &DSV_desc, &m_pShadowmap_resourceDSV );

	// recreating water normalmap buffer

	tex_desc.Width              = water_normalmap_resource_buffer_size_xy;
	tex_desc.Height             = water_normalmap_resource_buffer_size_xy;
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	m_pDevice->CreateTexture2D         ( &tex_desc, NULL, &m_pWater_normalmap_resource );
	m_pDevice->CreateShaderResourceView( m_pWater_normalmap_resource, &textureSRV_desc, &m_pWater_normalmap_resourceSRV );
	m_pDevice->CreateRenderTargetView  ( m_pWater_normalmap_resource, NULL, &m_pWater_normalmap_resourceRTV );
}

void Terrain::OnD3D11DestroyDevice()
{
	SAFE_RELEASE( m_pHeightmap_texture );
	SAFE_RELEASE( m_pHeightmap_textureSRV );

	//SAFE_RELEASE( m_pRock_bump_texture );
	SAFE_RELEASE( m_pRock_bump_textureSRV );

	//SAFE_RELEASE( m_pRock_microbump_texture );
	SAFE_RELEASE( m_pRock_microbump_textureSRV );

	//SAFE_RELEASE( m_pRock_diffuse_texture );
	SAFE_RELEASE( m_pRock_diffuse_textureSRV );

	//SAFE_RELEASE( m_pSand_bump_texture );
	SAFE_RELEASE( m_pSand_bump_textureSRV );

	//SAFE_RELEASE( m_pSand_microbump_texture );
	SAFE_RELEASE( m_pSand_microbump_textureSRV );

	//SAFE_RELEASE( m_pSand_diffuse_texture );
	SAFE_RELEASE( m_pSand_diffuse_textureSRV );

	//SAFE_RELEASE( m_pSlope_diffuse_texture );
	SAFE_RELEASE( m_pSlope_diffuse_textureSRV );

	//SAFE_RELEASE( m_pGrass_diffuse_texture );
	SAFE_RELEASE( m_pGrass_diffuse_textureSRV );

	SAFE_RELEASE( m_pLayerdef_texture );
	SAFE_RELEASE( m_pLayerdef_textureSRV );

	//SAFE_RELEASE( m_pWater_bump_texture );
	SAFE_RELEASE( m_pWater_bump_textureSRV );

	SAFE_RELEASE( m_pDepthmap_texture );
	SAFE_RELEASE( m_pDepthmap_textureSRV );

	SAFE_RELEASE( m_pMain_color_resource );
	SAFE_RELEASE( m_pMain_color_resourceSRV );
	SAFE_RELEASE( m_pMain_color_resourceRTV );

	SAFE_RELEASE( m_pMain_color_resource_resolved );
	SAFE_RELEASE( m_pMain_color_resource_resolvedSRV );

	SAFE_RELEASE( m_pMain_depth_resource );
	SAFE_RELEASE( m_pMain_depth_resourceDSV );
	SAFE_RELEASE( m_pMain_depth_resourceSRV );

	SAFE_RELEASE( m_pReflection_color_resource );
	SAFE_RELEASE( m_pReflection_color_resourceSRV );
	SAFE_RELEASE( m_pReflection_color_resourceRTV );
	SAFE_RELEASE( m_pRefraction_color_resource );
	SAFE_RELEASE( m_pRefraction_color_resourceSRV );
	SAFE_RELEASE( m_pRefraction_color_resourceRTV );

	SAFE_RELEASE( m_pReflection_depth_resource );
	SAFE_RELEASE( m_pReflection_depth_resourceDSV );
	SAFE_RELEASE( m_pRefraction_depth_resource );
	SAFE_RELEASE( m_pRefraction_depth_resourceRTV );
	SAFE_RELEASE( m_pRefraction_depth_resourceSRV );

	SAFE_RELEASE( m_pShadowmap_resource );
	SAFE_RELEASE( m_pShadowmap_resourceDSV );
	SAFE_RELEASE( m_pShadowmap_resourceSRV );

	SAFE_RELEASE( m_pHeightfield_indexbuffer );
	SAFE_RELEASE( m_pTrianglestrip_inputlayout );
	SAFE_RELEASE( m_pWater_indexbuffer );
	SAFE_RELEASE( m_pWater_vertexbuffer );

	SAFE_RELEASE( m_pHeightfield_vertexbuffer );
	SAFE_RELEASE( m_pHeightfield_inputlayout );

	SAFE_RELEASE( m_pWater_normalmap_resource );
	SAFE_RELEASE( m_pWater_normalmap_resourceSRV );
	SAFE_RELEASE( m_pWater_normalmap_resourceRTV );

	SAFE_RELEASE( m_pRenderTerrainVS );
	SAFE_RELEASE( m_pRenderTerrainPS );
	SAFE_RELEASE( m_pRenderWaterVS );
	SAFE_RELEASE( m_pRenderWaterPS );
	SAFE_RELEASE( m_pRenderMainVS );
	SAFE_RELEASE( m_pRenderMainPS );

	SAFE_RELEASE( m_pCBallInOne );
	SAFE_RELEASE( m_pRasterizerState );

	SAFE_RELEASE( m_pGeneralTexSS );
	SAFE_RELEASE( m_pAnisotropicWrapTexSS );
}

HRESULT Terrain::CreateTerrain()
{
	HRESULT hr = S_OK;
	int i,j,k,l;
	float x,z;
	int ix,iz;
	float * backterrain;
	XMFLOAT3 vec1,vec2,vec3;
	int currentstep = terrain_gridpoints;
	float mv,rm;
	float offset = 0,yscale = 0,maxheight = 0,minheight = 0;

	float *height_linear_array;
	float *patches_rawdata;
	HRESULT result;
	D3D11_SUBRESOURCE_DATA subresource_data;
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc; 

	backterrain = (float *) malloc( ( terrain_gridpoints + 1 ) * (terrain_gridpoints + 1 ) * sizeof(float) );
	rm = terrain_fractalinitialvalue;
	backterrain[ 0 ] = 0;
	backterrain[ 0 + terrain_gridpoints * terrain_gridpoints ] = 0;
	backterrain[ terrain_gridpoints ] = 0;
	backterrain[ terrain_gridpoints + terrain_gridpoints * terrain_gridpoints ] = 0;
	currentstep = terrain_gridpoints;
	srand(12);

	// generating fractal terrain using square-diamond method
	while( currentstep > 1 )
	{
		//square step;
		i=0;
		j=0;

		while( i < terrain_gridpoints )
		{
			j=0;
			while ( j < terrain_gridpoints )
			{
				mv = backterrain[ i + terrain_gridpoints * j ];
				mv += backterrain[ ( i + currentstep ) + terrain_gridpoints * j ];
				mv += backterrain[ ( i + currentstep ) + terrain_gridpoints * ( j + currentstep ) ];
				mv += backterrain[ i + terrain_gridpoints * ( j + currentstep ) ];
				mv /= 4.0;
				backterrain[ i + currentstep / 2 + terrain_gridpoints * ( j + currentstep / 2 ) ] = (float)( mv + rm * ( (rand()%1000) / 1000.0f - 0.5f ) );
				j+=currentstep;
			}
			i += currentstep;
		}

		//diamond step;
		i=0;
		j=0;
		while ( i < terrain_gridpoints)
		{
			j=0;
			while ( j < terrain_gridpoints)
			{
				mv = 0;
				mv = backterrain[ i + terrain_gridpoints * j ];
				mv += backterrain[ ( i + currentstep ) + terrain_gridpoints * j ];
				mv += backterrain[ ( i + currentstep / 2 ) + terrain_gridpoints * ( j + currentstep / 2 ) ];
				mv += backterrain[ i + currentstep / 2 + terrain_gridpoints * gp_wrap( j - currentstep / 2 ) ];
				mv /= 4;
				backterrain[ i + currentstep / 2 + terrain_gridpoints * j ] = (float)( mv + rm * ( (rand()%1000 ) / 1000.0f - 0.5f ) );

				mv=0;
				mv=backterrain[i+terrain_gridpoints*j];
				mv+=backterrain[i+terrain_gridpoints*(j+currentstep)];
				mv+=backterrain[(i+currentstep/2)+terrain_gridpoints*(j+currentstep/2)];
				mv+=backterrain[gp_wrap(i-currentstep/2)+terrain_gridpoints*(j+currentstep/2)];
				mv/=4;
				backterrain[i+terrain_gridpoints*(j+currentstep/2)]=(float)(mv+rm*((rand()%1000)/1000.0f-0.5f));

				mv=0;
				mv=backterrain[i+currentstep+terrain_gridpoints*j];
				mv+=backterrain[i+currentstep+terrain_gridpoints*(j+currentstep)];
				mv+=backterrain[(i+currentstep/2)+terrain_gridpoints*(j+currentstep/2)];
				mv+=backterrain[gp_wrap(i+currentstep/2+currentstep)+terrain_gridpoints*(j+currentstep/2)];
				mv/=4;
				backterrain[i+currentstep+terrain_gridpoints*(j+currentstep/2)]=(float)(mv+rm*((rand()%1000)/1000.0f-0.5f));

				mv=0;
				mv=backterrain[i+currentstep+terrain_gridpoints*(j+currentstep)];
				mv+=backterrain[i+terrain_gridpoints*(j+currentstep)];
				mv+=backterrain[(i+currentstep/2)+terrain_gridpoints*(j+currentstep/2)];
				mv+=backterrain[i+currentstep/2+terrain_gridpoints*gp_wrap(j+currentstep/2+currentstep)];
				mv/=4;
				backterrain[i+currentstep/2+terrain_gridpoints*(j+currentstep)]=(float)(mv+rm*((rand()%1000)/1000.0f-0.5f));
				j+=currentstep;
			}
			i += currentstep;
		}
		//changing current step;
		currentstep/=2;
		rm*=terrain_fractalfactor;
	}	

	// scaling to minheight..maxheight range
	for( i = 0 ; i < terrain_gridpoints + 1; i++ )
	{
		for( j = 0; j < terrain_gridpoints + 1; j++ )
		{
			height[i][j] = backterrain[ i + terrain_gridpoints * j ];
		}
	}

	maxheight=height[0][0];
	minheight=height[0][0];
	for( i = 0; i < terrain_gridpoints + 1; i++ )
	{
		for( j = 0; j < terrain_gridpoints + 1; j++ )
		{
			if(height[i][j]>maxheight) maxheight=height[i][j];
			if(height[i][j]<minheight) minheight=height[i][j];
		}
	}
	offset = minheight - terrain_minheight;
	yscale = ( terrain_maxheight - terrain_minheight ) / ( maxheight - minheight );

	for( i = 0; i < terrain_gridpoints + 1; i++ )
	{
		for( j = 0; j < terrain_gridpoints + 1; j++ )
		{
			height[i][j] -= minheight;
			height[i][j] *= yscale;
			height[i][j] += terrain_minheight;
		}
	}
	// moving down edges of heightmap	
	for( i = 0; i < terrain_gridpoints + 1; i++ )
	{
		for (j=0;j<terrain_gridpoints+1;j++)
		{
			mv=(float)((i-terrain_gridpoints/2.0f)*(i-terrain_gridpoints/2.0f)+(j-terrain_gridpoints/2.0f)*(j-terrain_gridpoints/2.0f));
			rm=(float)((terrain_gridpoints*0.8f)*(terrain_gridpoints*0.8f)/4.0f);
			if(mv>rm)
			{
				height[i][j]-=((mv-rm)/1000.0f)*terrain_geometry_scale;
			}
			if(height[i][j]<terrain_minheight)
			{
				height[i][j]=terrain_minheight;
			}
		}
	}

	// terrain banks
	for(k=0;k<10;k++)
	{	
		for(i=0;i<terrain_gridpoints+1;i++)
			for(j=0;j<terrain_gridpoints+1;j++)
			{
				mv=height[i][j];
				if((mv)>0.02f) 
				{
					mv-=0.02f;
				}
				if(mv<-0.02f) 
				{
					mv+=0.02f;
				}
				height[i][j]=mv;
			}
	}

	// smoothing 
	for(k=0;k<terrain_smoothsteps;k++)
	{	
		for(i=0;i<terrain_gridpoints+1;i++)
			for(j=0;j<terrain_gridpoints+1;j++)
			{

				vec1.x=2*terrain_geometry_scale;
				vec1.y=terrain_geometry_scale*(height[gp_wrap(i+1)][j]-height[gp_wrap(i-1)][j]);
				vec1.z=0;
				vec2.x=0;
				vec2.y=-terrain_geometry_scale*(height[i][gp_wrap(j+1)]-height[i][gp_wrap(j-1)]);
				vec2.z=-2*terrain_geometry_scale;

				XMVECTOR tmp = XMVector3Cross( XMLoadFloat3( &vec1 ), XMLoadFloat3( &vec2 ) );
				tmp = XMVector3Normalize( tmp );
				XMStoreFloat3( &vec3, tmp );

				if(((vec3.y>terrain_rockfactor)||(height[i][j]<1.2f))) 
				{
					rm=terrain_smoothfactor1;
					mv=height[i][j]*(1.0f-rm) +rm*0.25f*(height[gp_wrap(i-1)][j]+height[i][gp_wrap(j-1)]+height[gp_wrap(i+1)][j]+height[i][gp_wrap(j+1)]);
					backterrain[i+terrain_gridpoints*j]=mv;
				}
				else
				{
					rm=terrain_smoothfactor2;
					mv=height[i][j]*(1.0f-rm) +rm*0.25f*(height[gp_wrap(i-1)][j]+height[i][gp_wrap(j-1)]+height[gp_wrap(i+1)][j]+height[i][gp_wrap(j+1)]);
					backterrain[i+terrain_gridpoints*j]=mv;
				}

			}
			for (i=0;i<terrain_gridpoints+1;i++)
				for (j=0;j<terrain_gridpoints+1;j++)
				{
					height[i][j]=(backterrain[i+terrain_gridpoints*j]);
				}
	}
	for(i=0;i<terrain_gridpoints+1;i++)
	{
		for(j=0;j<terrain_gridpoints+1;j++)
		{
			rm=0.5f;
			mv=height[i][j]*(1.0f-rm) +rm*0.25f*(height[gp_wrap(i-1)][j]+height[i][gp_wrap(j-1)]+height[gp_wrap(i+1)][j]+height[i][gp_wrap(j+1)]);
			backterrain[i+terrain_gridpoints*j]=mv;
		}
	}
	for (i=0;i<terrain_gridpoints+1;i++)
	{
		for (j=0;j<terrain_gridpoints+1;j++)
		{
			height[i][j]=(backterrain[i+terrain_gridpoints*j]);
		}
	}

	free(backterrain);

	//calculating normals
	for (i=0;i<terrain_gridpoints+1;i++)
	{
		for (j=0;j<terrain_gridpoints+1;j++)
		{
			vec1.x=2*terrain_geometry_scale;
			vec1.y=terrain_geometry_scale*(height[gp_wrap(i+1)][j]-height[gp_wrap(i-1)][j]);
			vec1.z=0;
			vec2.x=0;
			vec2.y=-terrain_geometry_scale*(height[i][gp_wrap(j+1)]-height[i][gp_wrap(j-1)]);
			vec2.z=-2*terrain_geometry_scale;

			XMVECTOR tmp = XMVector3Cross( XMLoadFloat3( &vec1 ), XMLoadFloat3( &vec2 ) );
			tmp = XMVector3Normalize( tmp );
			XMStoreFloat3( &normal[i][j], tmp );
		}
	}

	// buiding layerdef 
	byte* temp_layerdef_map_texture_pixels=(byte *)malloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size*4);
	byte* layerdef_map_texture_pixels=(byte *)malloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size*4);
	for(i=0;i<terrain_layerdef_map_texture_size;i++)
	{
		for(j=0;j<terrain_layerdef_map_texture_size;j++)
		{
			x=(float)(terrain_gridpoints)*((float)i/(float)terrain_layerdef_map_texture_size);
			z=(float)(terrain_gridpoints)*((float)j/(float)terrain_layerdef_map_texture_size);
			ix=(int)floor(x);
			iz=(int)floor(z);
			rm=bilinear_interpolation(x-ix,z-iz,height[ix][iz],height[ix+1][iz],height[ix+1][iz+1],height[ix][iz+1])*terrain_geometry_scale;

			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
			temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;

			if((rm>terrain_height_underwater_start)&&(rm<=terrain_height_underwater_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
			}

			if((rm>terrain_height_sand_start)&&(rm<=terrain_height_sand_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
			}

			if((rm>terrain_height_grass_start)&&(rm<=terrain_height_grass_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=255;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
			}

			// TODO is normal[ix][iz][1] = normal[ix][iz].y?????
			mv=bilinear_interpolation(x-ix,z-iz,normal[ix][iz].y, normal[ix+1][iz].y, normal[ix+1][iz+1].y, normal[ix][iz+1].y );

			if((mv<terrain_slope_grass_start)&&(rm>terrain_height_sand_end))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
			}

			if((mv<terrain_slope_rocks_start)&&(rm>terrain_height_rocks_start))
			{
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=255;
			}
		}
	}
	for(i=0;i<terrain_layerdef_map_texture_size;i++)
	{
		for(j=0;j<terrain_layerdef_map_texture_size;j++)
		{
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3];
		}
	}
	for(i=2;i<terrain_layerdef_map_texture_size-2;i++)
	{
		for(j=2;j<terrain_layerdef_map_texture_size-2;j++)
		{
			int n1=0;
			int n2=0;
			int n3=0;
			int n4=0;
			for(k=-2;k<3;k++)
				for(l=-2;l<3;l++)
				{
					n1+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+0];
					n2+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+1];
					n3+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+2];
					n4+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+3];
				}
				layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=(byte)(n1/25);
				layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=(byte)(n2/25);
				layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=(byte)(n3/25);
				layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=(byte)(n4/25);
		}
	}

	// putting the generated data to textures
	subresource_data.pSysMem = layerdef_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_layerdef_map_texture_size*4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_layerdef_map_texture_size;
	tex_desc.Height = terrain_layerdef_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1; 
	tex_desc.SampleDesc.Quality = 0; 
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result = m_pDevice->CreateTexture2D(&tex_desc,&subresource_data,&m_pLayerdef_texture );

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	textureSRV_desc.Format=tex_desc.Format;
	textureSRV_desc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels=tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip=0;
	m_pDevice->CreateShaderResourceView( m_pLayerdef_texture,&textureSRV_desc,&m_pLayerdef_textureSRV );

	free(temp_layerdef_map_texture_pixels);
	free(layerdef_map_texture_pixels);

	height_linear_array = new float [ terrain_gridpoints*terrain_gridpoints*4 ];
	patches_rawdata = new float [ terrain_numpatches_1d*terrain_numpatches_1d*4 ];

	for(int i=0;i<terrain_gridpoints;i++)
	{
		for(int j=0; j<terrain_gridpoints;j++)
		{
			height_linear_array[(i+j*terrain_gridpoints)*4+0]=normal[i][j].x;
			height_linear_array[(i+j*terrain_gridpoints)*4+1]=normal[i][j].y;
			height_linear_array[(i+j*terrain_gridpoints)*4+2]=normal[i][j].z;
			height_linear_array[(i+j*terrain_gridpoints)*4+3]=height[i][j];
		}
	}
	subresource_data.pSysMem = height_linear_array;
	subresource_data.SysMemPitch = terrain_gridpoints*4*sizeof(float);
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_gridpoints;
	tex_desc.Height = terrain_gridpoints;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tex_desc.SampleDesc.Count = 1; 
	tex_desc.SampleDesc.Quality = 0; 
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result=m_pDevice->CreateTexture2D(&tex_desc,&subresource_data,&m_pHeightmap_texture);

	free(height_linear_array);

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	textureSRV_desc.Format=tex_desc.Format;
	textureSRV_desc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels=tex_desc.MipLevels;
	m_pDevice->CreateShaderResourceView( m_pHeightmap_texture,&textureSRV_desc,&m_pHeightmap_textureSRV );

	//building depthmap
	byte * depth_shadow_map_texture_pixels=(byte *)malloc(terrain_depth_shadow_map_texture_size*terrain_depth_shadow_map_texture_size*4);
	for(i=0;i<terrain_depth_shadow_map_texture_size;i++)
	{
		for(j=0;j<terrain_depth_shadow_map_texture_size;j++)
		{
			x=(float)(terrain_gridpoints)*((float)i/(float)terrain_depth_shadow_map_texture_size);
			z=(float)(terrain_gridpoints)*((float)j/(float)terrain_depth_shadow_map_texture_size);
			ix=(int)floor(x);
			iz=(int)floor(z);
			rm=bilinear_interpolation(x-ix,z-iz,height[ix][iz],height[ix+1][iz],height[ix+1][iz+1],height[ix][iz+1])*terrain_geometry_scale;

			if(rm>0)
			{
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+0]=0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+1]=0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+2]=0;
			}
			else
			{
				float no=(1.0f*255.0f*(rm/(terrain_minheight*terrain_geometry_scale)))-1.0f;
				if(no>255) no=255;
				if(no<0) no=0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+0]=(byte)no;

				no=(10.0f*255.0f*(rm/(terrain_minheight*terrain_geometry_scale)))-40.0f;
				if(no>255) no=255;
				if(no<0) no=0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+1]=(byte)no;

				no=(100.0f*255.0f*(rm/(terrain_minheight*terrain_geometry_scale)))-300.0f;
				if(no>255) no=255;
				if(no<0) no=0;
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+2]=(byte)no;
			}
			depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+3]=0;
		}
	}

	subresource_data.pSysMem = depth_shadow_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_depth_shadow_map_texture_size*4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_depth_shadow_map_texture_size;
	tex_desc.Height = terrain_depth_shadow_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1; 
	tex_desc.SampleDesc.Quality = 0; 
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result = m_pDevice->CreateTexture2D(&tex_desc,&subresource_data,&m_pDepthmap_texture);

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	textureSRV_desc.Format=tex_desc.Format;
	textureSRV_desc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels=tex_desc.MipLevels;
	m_pDevice->CreateShaderResourceView( m_pDepthmap_texture,&textureSRV_desc,&m_pDepthmap_textureSRV);

	free(depth_shadow_map_texture_pixels);

	// creating terrain vertex buffer
	UINT g_dwNumIndices = terrain_gridpoints * terrain_gridpoints * 6;
	D3D11_BUFFER_DESC ibDesc;
	ibDesc.ByteWidth = g_dwNumIndices * sizeof( unsigned int );
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	unsigned long* pIndexData = new unsigned long[ g_dwNumIndices ];
	if( !pIndexData )
		return E_OUTOFMEMORY;
	unsigned long* pIndices = pIndexData;
	for( unsigned int y = 1; y < terrain_gridpoints + 1; y++ )
	{
		for( unsigned int x = 1; x < terrain_gridpoints + 1; x++ )
		{
			pIndices[ m_iIndexCount++ ] = ( unsigned int )( ( y - 1 ) * ( terrain_gridpoints + 1 ) + ( x - 1 ) );
			pIndices[ m_iIndexCount++ ] = ( unsigned int )( ( y - 0 ) * ( terrain_gridpoints + 1 ) + ( x - 1 ) );
			pIndices[ m_iIndexCount++ ] = ( unsigned int )( ( y - 1 ) * ( terrain_gridpoints + 1 ) + ( x - 0 ) );

			pIndices[ m_iIndexCount++ ] = ( unsigned int )( ( y - 1 ) * ( terrain_gridpoints + 1 ) + ( x - 0 ) );
			pIndices[ m_iIndexCount++ ] = ( unsigned int )( ( y - 0 ) * ( terrain_gridpoints + 1 ) + ( x - 1 ) );
			pIndices[ m_iIndexCount++ ] = ( unsigned int )( ( y - 0 ) * ( terrain_gridpoints + 1 ) + ( x - 0 ) );
			//m_iIndexCount += 6;
		}
	}

	D3D11_SUBRESOURCE_DATA ibInitData;
	ZeroMemory( &ibInitData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	ibInitData.pSysMem = pIndexData;
	V_RETURN( m_pDevice->CreateBuffer( &ibDesc, &ibInitData, &m_pHeightfield_indexbuffer ) );
	SAFE_DELETE_ARRAY( pIndexData );

	// Create a Vertex Buffer
	UINT g_dwNumVertices = ( terrain_gridpoints + 1 ) * ( terrain_gridpoints + 1 );
	D3D11_BUFFER_DESC vbDesc;
	vbDesc.ByteWidth = g_dwNumVertices * sizeof( TERRAIN_VERTEX );
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	TERRAIN_VERTEX* pVertData = new TERRAIN_VERTEX[ g_dwNumVertices ];
	if( !pVertData )
		return E_OUTOFMEMORY;
	TERRAIN_VERTEX* pVertices = pVertData;
	float texInterval = 50.0 / ( terrain_gridpoints + 1 );

	// For text
	float highMax = -10000, highMin = 10000;


	for( int y = 0; y < terrain_gridpoints + 1; y++ )
	{
		for( int x = 0; x < terrain_gridpoints + 1; x++ )
		{
			( *pVertices ).position = XMFLOAT4( ( ( float )x / ( float )( terrain_gridpoints + 1 - 1 ) - 0.5f ) * /*XM_PI * 100*/512,
				height[ y ][ x ],
				( ( float )y / ( float )( terrain_gridpoints + 1 - 1 ) - 0.5f ) * /*XM_PI * 100*/512,
				0 );

			//cout << "Pos " << ( *pVertices ).position.x << " " << ( *pVertices ).position.y << " " << ( *pVertices ).position.z << endl;
			( *pVertices ).normal = XMFLOAT3( 0.0, 1.0, 0.0 );

			( *pVertices++ ).texcoord = XMFLOAT2( y * texInterval, x * texInterval );

			if( height[ y ][ x ] >= highMax )
			{
				highMax = height[ y ][ x ];
			}
			if( height[ y ][ x ] <= highMin )
			{
				highMin = height[ y ][ x ];
			}
		}
	}

	cout << "Max high is " << highMax << " and Min high is " << highMin << endl;
	D3D11_SUBRESOURCE_DATA vbInitData;
	ZeroMemory( &vbInitData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	vbInitData.pSysMem = pVertData;
	V_RETURN( m_pDevice->CreateBuffer( &vbDesc, &vbInitData, &m_pHeightfield_vertexbuffer ) );
	SAFE_DELETE_ARRAY( pVertData );

	delete [] pIndexData;
	delete [] pVertData;

	// Water vertex ( a square water )
	unsigned int waterW = ( terrain_gridpoints / 2 );
	unsigned int waterH = ( terrain_gridpoints / 2 );
	g_dwNumIndices = waterW * waterH * 6;
	ibDesc.ByteWidth = g_dwNumIndices * sizeof( unsigned int );
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;

	pIndexData = new unsigned long[ g_dwNumIndices ];
	if( !pIndexData )
		return E_OUTOFMEMORY;
	pIndices = pIndexData;
	for( unsigned int y = 1; y < waterW + 1; y++ )
	{
		for( unsigned int x = 1; x < waterH + 1; x++ )
		{
			pIndices[ m_iWaterIndexCount++ ] = ( unsigned int )( ( y - 1 ) * ( waterW + 1 ) + ( x - 1 ) );
			pIndices[ m_iWaterIndexCount++ ] = ( unsigned int )( ( y - 0 ) * ( waterW + 1 ) + ( x - 1 ) );
			pIndices[ m_iWaterIndexCount++ ] = ( unsigned int )( ( y - 1 ) * ( waterW + 1 ) + ( x - 0 ) );

			pIndices[ m_iWaterIndexCount++ ] = ( unsigned int )( ( y - 1 ) * ( waterW + 1 ) + ( x - 0 ) );
			pIndices[ m_iWaterIndexCount++ ] = ( unsigned int )( ( y - 0 ) * ( waterW + 1 ) + ( x - 1 ) );
			pIndices[ m_iWaterIndexCount++ ] = ( unsigned int )( ( y - 0 ) * ( waterW + 1 ) + ( x - 0 ) );
		}
	}

	ZeroMemory( &ibInitData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	ibInitData.pSysMem = pIndexData;
	V_RETURN( m_pDevice->CreateBuffer( &ibDesc, &ibInitData, &m_pWater_indexbuffer ) );
	SAFE_DELETE_ARRAY( pIndexData );

	// Create a Vertex Buffer
	g_dwNumVertices = ( waterW + 1 ) * ( waterH + 1 );
	vbDesc.ByteWidth = g_dwNumVertices * sizeof( TERRAIN_VERTEX );
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;

	pVertData = new TERRAIN_VERTEX[ g_dwNumVertices ];
	if( !pVertData )
		return E_OUTOFMEMORY;
	pVertices = pVertData;
	texInterval = 50.0 / ( waterW + 1 );
	for( int y = 0; y < waterW + 1; y++ )
	{
		for( int x = 0; x < waterW + 1; x++ )
		{
			( *pVertices ).position = XMFLOAT4( ( ( float )x / ( float )( waterW + 1 - 1 ) - 0.5f ) * /*XM_PI * 100*/waterW,
				2.0f,
				( ( float )y / ( float )( waterW + 1 - 1 ) - 0.5f ) * /*XM_PI * 100*/waterH,
				0 );

			( *pVertices ).normal = XMFLOAT3( 0.0, 1.0, 0.0 );

			( *pVertices++ ).texcoord = XMFLOAT2( y * texInterval, x * texInterval );
		}
	}

	ZeroMemory( &vbInitData, sizeof( D3D11_SUBRESOURCE_DATA ) );
	vbInitData.pSysMem = pVertData;
	V_RETURN( m_pDevice->CreateBuffer( &vbDesc, &vbInitData, &m_pWater_vertexbuffer ) );
	SAFE_DELETE_ARRAY( pVertData );

	delete [] pIndexData;
	delete [] pVertData;
	pIndices = NULL;
	pVertices = NULL;

	return hr;
}

void Terrain::Render( CModelViewerCamera *cam, ID3D11DeviceContext* pd3dImmediateContext, XMMATRIX* pRotate, float fTime, SkyBox *sb )
{
	UINT stride = sizeof( TERRAIN_VERTEX );
	UINT offset = 0;
	UINT cRT = 1;

	// Multi Pass rendering
	ID3D11RenderTargetView *colorBuffer = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView  *backBuffer = DXUTGetD3D11DepthStencilView();
	D3D11_VIEWPORT currentViewport;
	D3D11_VIEWPORT reflection_Viewport;
	D3D11_VIEWPORT refraction_Viewport;
	D3D11_VIEWPORT shadowmap_resource_viewport;
	D3D11_VIEWPORT water_normalmap_resource_viewport;
	D3D11_VIEWPORT main_Viewport;

	float ClearColor[ 4 ] = { 0.176f, 0.196f, 0.667f, 0.0f };
	float RefractionClearColor[ 4 ] = { 0.5f, 0.5f, 0.5f, 1.0f };

	reflection_Viewport.Width=(float)BackbufferWidth*reflection_buffer_size_multiplier;
	reflection_Viewport.Height=(float)BackbufferHeight*reflection_buffer_size_multiplier;
	reflection_Viewport.MaxDepth=1;
	reflection_Viewport.MinDepth=0;
	reflection_Viewport.TopLeftX=0;
	reflection_Viewport.TopLeftY=0;

	refraction_Viewport.Width=(float)BackbufferWidth*refraction_buffer_size_multiplier;
	refraction_Viewport.Height=(float)BackbufferHeight*refraction_buffer_size_multiplier;
	refraction_Viewport.MaxDepth=1;
	refraction_Viewport.MinDepth=0;
	refraction_Viewport.TopLeftX=0;
	refraction_Viewport.TopLeftY=0;

	main_Viewport.Width=(float)BackbufferWidth*main_buffer_size_multiplier;
	main_Viewport.Height=(float)BackbufferHeight*main_buffer_size_multiplier;
	main_Viewport.MaxDepth=1;
	main_Viewport.MinDepth=0;
	main_Viewport.TopLeftX=0;
	main_Viewport.TopLeftY=0;

	shadowmap_resource_viewport.Width=shadowmap_resource_buffer_size_xy;
	shadowmap_resource_viewport.Height=shadowmap_resource_buffer_size_xy;
	shadowmap_resource_viewport.MaxDepth=1;
	shadowmap_resource_viewport.MinDepth=0;
	shadowmap_resource_viewport.TopLeftX=0;
	shadowmap_resource_viewport.TopLeftY=0;

	water_normalmap_resource_viewport.Width = water_normalmap_resource_buffer_size_xy;
	water_normalmap_resource_viewport.Height = water_normalmap_resource_buffer_size_xy;
	water_normalmap_resource_viewport.MaxDepth = 1;
	water_normalmap_resource_viewport.MinDepth = 0;
	water_normalmap_resource_viewport.TopLeftX = 0;
	water_normalmap_resource_viewport.TopLeftY = 0;

	XMMATRIX view = cam->GetViewMatrix();
	XMMATRIX world = cam->GetWorldMatrix();
	XMMATRIX proj = cam->GetProjMatrix();
	XMMATRIX mWorldViewProjection;
	mWorldViewProjection = XMMatrixMultiply( view, proj );
	//world = world*(*pRotate);
	m_CBallInOne.mView = XMMatrixTranspose( view );
	m_CBallInOne.mWorld = XMMatrixTranspose( world);
	m_CBallInOne.mProjection = XMMatrixTranspose( cam->GetProjMatrix() );
	m_CBallInOne.mWaterTexcoordShift = XMFLOAT2( fTime * 1.5f, fTime * 0.75 );
	m_CBallInOne.fScreenSizeInv = XMFLOAT2( 1.0f / ( BackbufferWidth * main_buffer_size_multiplier ), 1.0f / ( BackbufferHeight * main_buffer_size_multiplier ) );
	pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );

	// Saving scene color buffer and back buffer to constants
	pd3dImmediateContext->RSGetViewports( &cRT, &currentViewport );
	pd3dImmediateContext->OMGetRenderTargets( 1, &colorBuffer, &backBuffer );

	// TODO shadow_map rendertarget


	// Setting up reflection rendertarget
	pd3dImmediateContext->RSSetViewports( 1, &reflection_Viewport );
	pd3dImmediateContext->OMSetRenderTargets( 1, &m_pReflection_color_resourceRTV, m_pReflection_depth_resourceDSV );
	pd3dImmediateContext->ClearRenderTargetView( m_pReflection_color_resourceRTV, RefractionClearColor );
	pd3dImmediateContext->ClearDepthStencilView( m_pReflection_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	SetupReflectionView( pd3dImmediateContext, cam );

	// Rendering sky to reflection RT
	sb->RenderSkyBox( &m_CBallInOne.mModelViewProjectionMatrix, pd3dImmediateContext );

	// Rendering terrain to reflection RT
	//tex_variable=pEffect->GetVariableByName("g_DepthTexture")->AsShaderResource();
	//tex_variable->SetResource(shadowmap_resourceSRV);

	pd3dImmediateContext->IASetInputLayout( m_pHeightfield_inputlayout );
	pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pHeightfield_vertexbuffer, &stride, &offset );
	pd3dImmediateContext->IASetIndexBuffer( m_pHeightfield_indexbuffer, DXGI_FORMAT_R32_UINT, 0 );
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS );
	pd3dImmediateContext->RSSetState( m_pRasterizerState );
	pd3dImmediateContext->VSSetShader( m_pRenderTerrainVS, NULL, 0 );
	pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
	pd3dImmediateContext->PSSetShader( m_pRenderTerrainPS, NULL, 0 );
	pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pHeightmap_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pLayerdef_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 2, 1, &m_pRock_bump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 3, 1, &m_pRock_microbump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 4, 1, &m_pRock_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 5, 1, &m_pSand_bump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 6, 1, &m_pSand_microbump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 7, 1, &m_pSand_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 8, 1, &m_pGrass_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 9, 1, &m_pSlope_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 10, 1, &m_pWater_bump_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pDepthmap_textureSRV );
	pd3dImmediateContext->DrawIndexed( m_iIndexCount, 0, 0 );

	// Setting up main rendertarget
	pd3dImmediateContext->RSSetViewports( 1, &main_Viewport );
	pd3dImmediateContext->OMSetRenderTargets( 1, &m_pMain_color_resourceRTV, m_pMain_depth_resourceDSV );
	pd3dImmediateContext->ClearRenderTargetView( m_pMain_color_resourceRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( m_pMain_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	SetupNormalView( pd3dImmediateContext, cam );

	// Rendering terrain to main buffer
	//tex_variable=pEffect->GetVariableByName("g_DepthTexture")->AsShaderResource();
	//tex_variable->SetResource(shadowmap_resourceSRV);

	//tex_variable=pEffect->GetVariableByName("g_WaterNormalMapTexture")->AsShaderResource();
	//tex_variable->SetResource(water_normalmap_resourceSRV);

	pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS );
	pd3dImmediateContext->RSSetState( m_pRasterizerState );
	pd3dImmediateContext->VSSetShader( m_pRenderTerrainVS, NULL, 0 );
	pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
	pd3dImmediateContext->PSSetShader( m_pRenderTerrainPS, NULL, 0 );
	pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pHeightmap_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pLayerdef_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 2, 1, &m_pRock_bump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 3, 1, &m_pRock_microbump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 4, 1, &m_pRock_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 5, 1, &m_pSand_bump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 6, 1, &m_pSand_microbump_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 7, 1, &m_pSand_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 8, 1, &m_pGrass_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 9, 1, &m_pSlope_diffuse_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 10, 1, &m_pWater_bump_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 11, 1, &m_pWater_normalmap_resourceSRV );
	//pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pHeightmap_textureSRV );
	pd3dImmediateContext->DrawIndexed( m_iIndexCount, 0, 0 );

	// TODO

	// Getting back to rendering to main buffer
	pd3dImmediateContext->RSSetViewports( 1, &main_Viewport );
	pd3dImmediateContext->OMSetRenderTargets( 1, &m_pMain_color_resourceRTV, m_pMain_depth_resourceDSV );

	// Rendering water surface to main buffer
	pd3dImmediateContext->IASetInputLayout( m_pHeightfield_inputlayout );
	pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pWater_vertexbuffer, &stride, &offset );
	pd3dImmediateContext->IASetIndexBuffer( m_pWater_indexbuffer, DXGI_FORMAT_R32_UINT, 0 );
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS );
	pd3dImmediateContext->RSSetState( m_pRasterizerState );
	pd3dImmediateContext->VSSetShader( m_pRenderWaterVS, NULL, 0 );
	pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
	pd3dImmediateContext->PSSetShader( m_pRenderWaterPS, NULL, 0 );
	pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pLayerdef_textureSRV );
	pd3dImmediateContext->PSSetShaderResources( 12, 1, &m_pReflection_color_resourceSRV );

	pd3dImmediateContext->DrawIndexed( m_iWaterIndexCount, 0, 0 );

	// Rendering sky to main buffer
	sb->RenderSkyBox( &mWorldViewProjection, pd3dImmediateContext );

	//restoring scene color buffer and back buffer
	pd3dImmediateContext->OMSetRenderTargets( 1, &colorBuffer, backBuffer );
    pd3dImmediateContext->RSSetViewports( 1, &currentViewport );

	//resolving main buffer 
	pd3dImmediateContext->ResolveSubresource( m_pMain_color_resource_resolved, 0, m_pMain_color_resource, 0, DXGI_FORMAT_R8G8B8A8_UNORM );

	//drawing main buffer to back buffer
	pd3dImmediateContext->IASetInputLayout( m_pTrianglestrip_inputlayout );
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	pd3dImmediateContext->VSSetShader( m_pRenderMainVS, NULL, 0 );
	pd3dImmediateContext->PSSetShader( m_pRenderMainPS, NULL, 0 );
	pd3dImmediateContext->PSSetShaderResources( 14, 1, &m_pMain_color_resource_resolvedSRV /*m_pReflection_color_resourceSRV*/ );
	pd3dImmediateContext->Draw( 4, 0 );
	//pEffect->GetTechniqueByName("MainToBackBuffer")->GetPassByIndex(0)->Apply(0, pContext);
	//stride=sizeof(float)*6;
	//pContext->IASetVertexBuffers(0,1,&heightfield_vertexbuffer,&stride,&offset);
	//pContext->Draw(4, 0); // just need to pass 4 vertices to shader

	//tex_variable=pEffect->GetVariableByName("g_MainTexture")->AsShaderResource();
	//tex_variable->SetResource(NULL);

	//pEffect->GetTechniqueByName("Default")->GetPassByIndex(0)->Apply(0, pContext);
	// TODO need another shader for this stage

    SAFE_RELEASE ( colorBuffer );
    SAFE_RELEASE ( backBuffer );






	//pd3dImmediateContext->IASetInputLayout( m_pHeightfield_inputlayout );
	//pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pHeightfield_vertexbuffer, &stride, &offset );
	//pd3dImmediateContext->IASetIndexBuffer( m_pHeightfield_indexbuffer, DXGI_FORMAT_R32_UINT, 0 );
	//pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	//XMMATRIX view = cam->GetViewMatrix();
	//XMMATRIX world = cam->GetWorldMatrix();
	////world = world*(*pRotate);
	//m_CBallInOne.mView = XMMatrixTranspose( view );
	//m_CBallInOne.mWorld = XMMatrixTranspose( world);
	//m_CBallInOne.mProjection = XMMatrixTranspose( cam->GetProjMatrix() );
	//m_CBallInOne.mModelViewProjectionMatrix = XMMatrixTranspose( XMMatrixMultiply( view, cam->GetProjMatrix() ) );
	//m_CBallInOne.mWaterTexcoordShift = XMFLOAT2( fTime * 1.5f, fTime * 0.75 );
	//pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );

	//pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS );
	//pd3dImmediateContext->RSSetState( m_pRasterizerState );
	//pd3dImmediateContext->VSSetShader( m_pRenderTerrainVS, NULL, 0 );
	//pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
	//pd3dImmediateContext->PSSetShader( m_pRenderTerrainPS, NULL, 0 );
	//pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pHeightmap_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pLayerdef_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 2, 1, &m_pRock_bump_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 3, 1, &m_pRock_microbump_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 4, 1, &m_pRock_diffuse_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 5, 1, &m_pSand_bump_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 6, 1, &m_pSand_microbump_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 7, 1, &m_pSand_diffuse_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 8, 1, &m_pGrass_diffuse_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 9, 1, &m_pSlope_diffuse_textureSRV );
	//pd3dImmediateContext->PSSetShaderResources( 10, 1, &m_pWater_bump_textureSRV );
	////pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pHeightmap_textureSRV );
	////pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pHeightmap_textureSRV );
	//pd3dImmediateContext->DrawIndexed( m_iIndexCount, 0, 0 );


	//pd3dImmediateContext->IASetInputLayout( m_pHeightfield_inputlayout );
	//pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pWater_vertexbuffer, &stride, &offset );
	//pd3dImmediateContext->IASetIndexBuffer( m_pWater_indexbuffer, DXGI_FORMAT_R32_UINT, 0 );
	//pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	//pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS );
	//pd3dImmediateContext->RSSetState( m_pRasterizerState );
	//pd3dImmediateContext->VSSetShader( m_pRenderWaterVS, NULL, 0 );
	//pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
	//pd3dImmediateContext->PSSetShader( m_pRenderWaterPS, NULL, 0 );
	////pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pLayerdef_textureSRV );

	//pd3dImmediateContext->DrawIndexed( m_iWaterIndexCount, 0, 0 );
}

HRESULT Terrain::LoadTextures()
{
	HRESULT hr = S_OK;
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/rock_bump6.dds", &m_pRock_bump_textureSRV ) );
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/terrain_rock4.dds", &m_pRock_diffuse_textureSRV ) );
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/sand_diffuse.dds", &m_pSand_diffuse_textureSRV ) );
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/rock_bump4.dds", &m_pSand_bump_textureSRV ) );
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/terrain_grass.dds", &m_pGrass_diffuse_textureSRV ) );
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/terrain_slope.dds", &m_pSlope_diffuse_textureSRV ) );
	//V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/lichen1_normal.dds", &m_pSand_microbump_textureSRV ) );
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/rock_bump4.dds", &m_pRock_microbump_textureSRV ) );
	V_RETURN( DXUTCreateShaderResourceViewFromFile( m_pDevice, L"TerrainTextures/water_bump.dds", &m_pWater_bump_textureSRV ) );

	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	V_RETURN( m_pDevice->CreateSamplerState( &sampDesc, &m_pGeneralTexSS ) );

	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.MaxAnisotropy = 16;
	V_RETURN( m_pDevice->CreateSamplerState( &sampDesc, &m_pAnisotropicWrapTexSS ) );
}

void Terrain::SetupReflectionView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam )
{

	float aspectRatio = BackbufferWidth / BackbufferHeight;

	XMFLOAT3 EyePoint;
    XMFLOAT3 LookAtPoint;

	XMStoreFloat3( &EyePoint, cam->GetEyePt() );
	XMStoreFloat3( &LookAtPoint, cam->GetLookAtPt() );
	EyePoint.y = -1.0f * EyePoint.y + 1.0f;
	LookAtPoint.y = -1.0f * LookAtPoint.y + 1.0f;

	XMMATRIX mView;
	XMMATRIX mProj;
	XMMATRIX mViewProj;
	XMMATRIX mViewProjInv;

	XMMATRIX mWorld;
	mView = cam->GetViewMatrix();
	mWorld = cam->GetWorldMatrix();

	XMFLOAT4X4 tmp_mWorld;
	XMStoreFloat4x4( &tmp_mWorld, mWorld );

	tmp_mWorld._42 = -tmp_mWorld._42 - 1.0f;
	
	tmp_mWorld._21 *= -1.0f;
	tmp_mWorld._23 *= -1.0f;
	tmp_mWorld._32 *= -1.0f;

	mWorld = XMLoadFloat4x4( &tmp_mWorld );

	mView = XMMatrixInverse( NULL, mWorld );
	mProj = XMMatrixPerspectiveFovLH( camera_fov * XM_PI / 360.0f, aspectRatio, scene_z_near,scene_z_far );
	mViewProj = mView * mProj;
	mViewProjInv = XMMatrixInverse( NULL, mViewProj );

	XMVECTOR direction = XMVectorSubtract( XMLoadFloat3( &LookAtPoint ), XMLoadFloat3( &EyePoint ) );
	XMVECTOR normalized_direction = XMVector3Normalize( direction );

	m_CBallInOne.mModelViewProjectionMatrix = XMMatrixTranspose( mViewProj );
	m_CBallInOne.mCameraPosition = XMLoadFloat3( &EyePoint );
	m_CBallInOne.mCameraDirection = normalized_direction;
	m_CBallInOne.fHalfSpaceCullSign = 1.0f;
	m_CBallInOne.fHalfSpaceCullPosition = -0.6;
	pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );
}

void Terrain::SetupRefractionView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam )
{
	//pEffect->GetVariableByName("g_HalfSpaceCullSign")->AsScalar()->SetFloat(-1.0f);
	//pEffect->GetVariableByName("g_HalfSpaceCullPosition")->AsScalar()->SetFloat(terrain_minheight);

	m_CBallInOne.fHalfSpaceCullSign = -1.0f;
	m_CBallInOne.fHalfSpaceCullPosition = terrain_minheight;
	pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );
}

void Terrain::SetupLightView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam )
{

	XMFLOAT3 EyePoint= XMFLOAT3( -10000.0f, 6500.0f, 10000.0f );
    XMFLOAT3 LookAtPoint = XMFLOAT3( terrain_far_range / 2.0f, 0.0f, terrain_far_range / 2.0f );
	XMFLOAT3 lookUp = XMFLOAT3( 0, 1, 0 );
	XMFLOAT3 cameraPosition;
	XMStoreFloat3( &cameraPosition, cam->GetEyePt() );

	float nr, fr;
	nr = sqrt( EyePoint.x * EyePoint.x + EyePoint.y * EyePoint.y + EyePoint.z * EyePoint.z ) - terrain_far_range * 0.7f;
	fr = sqrt( EyePoint.x * EyePoint.x + EyePoint.y * EyePoint.y + EyePoint.z * EyePoint.z ) + terrain_far_range*0.7f;

    XMMATRIX mView = XMMatrixLookAtLH( XMLoadFloat3( &EyePoint ), XMLoadFloat3( &LookAtPoint ), XMLoadFloat3( &lookUp ) );
    XMMATRIX mProjMatrix = XMMatrixOrthographicLH( terrain_far_range * 1.5, terrain_far_range, nr, fr );
    XMMATRIX mViewProj = mView * mProjMatrix;
    XMMATRIX mViewProjInv;
	mViewProjInv = XMMatrixInverse( NULL, mViewProj );

	//pEffect->GetVariableByName("g_ModelViewProjectionMatrix")->AsMatrix()->SetMatrix(mViewProj);
	//pEffect->GetVariableByName("g_LightModelViewProjectionMatrix")->AsMatrix()->SetMatrix(mViewProj);
	//pEffect->GetVariableByName("g_LightModelViewProjectionMatrixInv")->AsMatrix()->SetMatrix(mViewProjInv);
	//pEffect->GetVariableByName("g_CameraPosition")->AsVector()->SetFloatVector(cameraPosition);
	XMVECTOR direction = cam->GetLookAtPt() - cam->GetEyePt();
	XMVECTOR normalized_direction = XMVector3Normalize( direction );
	//pEffect->GetVariableByName("g_CameraDirection")->AsVector()->SetFloatVector(normalized_direction);

	//pEffect->GetVariableByName("g_HalfSpaceCullSign")->AsScalar()->SetFloat(1.0);
	//pEffect->GetVariableByName("g_HalfSpaceCullPosition")->AsScalar()->SetFloat(terrain_minheight*2);

	m_CBallInOne.mModelViewProjectionMatrix = XMMatrixTranspose( mViewProj );
	m_CBallInOne.mLightModelViewProjectionMatrix = XMMatrixTranspose( mViewProj );
	m_CBallInOne.mLightModelViewProjectionMatrixInv = XMMatrixTranspose( mViewProjInv );
	m_CBallInOne.mCameraPosition = XMLoadFloat3( &cameraPosition );
	m_CBallInOne.mCameraDirection = normalized_direction;
	m_CBallInOne.fHalfSpaceCullSign = 1.0f;
	m_CBallInOne.fHalfSpaceCullPosition = terrain_minheight * 2;
	pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );
}

void Terrain::SetupNormalView( ID3D11DeviceContext* pd3dImmediateContext, CModelViewerCamera *cam )
{
	XMVECTOR EyePoint;
    XMVECTOR LookAtPoint;

	EyePoint =cam->GetEyePt();
	LookAtPoint = cam->GetLookAtPt();
    XMMATRIX mView = cam->GetViewMatrix();
    XMMATRIX mProjMatrix = cam->GetProjMatrix();
    XMMATRIX mViewProj = mView * mProjMatrix;
    XMMATRIX mViewProjInv;
	mViewProjInv = XMMatrixInverse( NULL, mViewProj );
    XMVECTOR cameraPosition = cam->GetEyePt();

	//pEffect->GetVariableByName("g_ModelViewMatrix")->AsMatrix()->SetMatrix(mView);
	//pEffect->GetVariableByName("g_ModelViewProjectionMatrix")->AsMatrix()->SetMatrix(mViewProj);
	//pEffect->GetVariableByName("g_ModelViewProjectionMatrixInv")->AsMatrix()->SetMatrix(mViewProjInv);
	//pEffect->GetVariableByName("g_CameraPosition")->AsVector()->SetFloatVector(cameraPosition);
	XMVECTOR direction = LookAtPoint - EyePoint;
	XMVECTOR normalized_direction = XMVector3Normalize( direction );
	//pEffect->GetVariableByName("g_CameraDirection")->AsVector()->SetFloatVector(normalized_direction);
	//pEffect->GetVariableByName("g_HalfSpaceCullSign")->AsScalar()->SetFloat(1.0);
	//pEffect->GetVariableByName("g_HalfSpaceCullPosition")->AsScalar()->SetFloat(terrain_minheight*2);

	m_CBallInOne.mModelViewMatrix = XMMatrixTranspose( mView );
	m_CBallInOne.mModelViewProjectionMatrix = XMMatrixTranspose( mViewProj );
	m_CBallInOne.mModelViewProjectionMatrixInv = XMMatrixTranspose( mViewProjInv );
	m_CBallInOne.mCameraPosition = cameraPosition;
	m_CBallInOne.mCameraDirection = normalized_direction;
	m_CBallInOne.fHalfSpaceCullSign = 1.0f;
	m_CBallInOne.fHalfSpaceCullPosition = terrain_minheight * 2;
	pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );
}

float bilinear_interpolation(float fx, float fy, float a, float b, float c, float d)
{
	float s1,s2,s3,s4;
	s1=fx*fy;
	s2=(1-fx)*fy;
	s3=(1-fx)*(1-fy);
	s4=fx*(1-fy);
	return((a*s3+b*s4+c*s1+d*s2));
}
