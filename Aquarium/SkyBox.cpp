#include "SkyBox.h"


SkyBox::SkyBox(void)
{
	m_pEnvironmentMap = NULL;
    m_pEnvironmentRV = NULL;
    m_pd3dDevice = NULL;

    m_pVertexShader = NULL;
    m_pPixelShader = NULL;
    m_pSampleState = NULL;

    m_pVertexLayout = NULL;
    m_pcbVSPerObject = NULL;
    m_pVertexBuffer = NULL;
    m_pDepthStencilState = NULL;
    m_pRasterizerState = NULL;

	// Variables for storing d3d states
	m_pDepthStencilStateStored = NULL;
    m_StencilRefStored = NULL;
    m_pRasterizerStateStored = NULL;
    m_pBlendStateStored = NULL;
    m_BlendFactorStored[0] = m_BlendFactorStored[1] = m_BlendFactorStored[2] = m_BlendFactorStored[3] = 0.0;
    m_SampleMaskStored;
    m_pSamplerStateStored = NULL;

	m_fSize = 1.0;
}


SkyBox::~SkyBox(void)
{
}

HRESULT SkyBox::Initialization( ID3D11Device* device, float fSize, ID3D11Texture2D* pCubeTexture, ID3D11ShaderResourceView* pCubeRV )
{
	HRESULT hr;

	m_pd3dDevice = device;
	m_fSize = fSize;
	m_pEnvironmentMap = pCubeTexture;
	m_pEnvironmentRV = pCubeRV;

	ID3DBlob* pBlobVS = NULL;
    ID3DBlob* pBlobPS = NULL;

    // Create the shaders
    V_RETURN( DXUTCompileFromFile( L"SkyBox.hlsl", NULL, "SkyBoxVS", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pBlobVS ) );
    V_RETURN( DXUTCompileFromFile( L"SkyBox.hlsl", NULL, "SkyBoxPS", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pBlobPS ) );

    V_RETURN( device->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &m_pVertexShader ) );
    V_RETURN( device->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &m_pPixelShader ) );

    DXUT_SetDebugName( m_pVertexShader, "SkyBoxVS" );
    DXUT_SetDebugName( m_pPixelShader, "SkyBoxPS" );

	const D3D11_INPUT_ELEMENT_DESC g_aVertexLayout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

    // Create an input layout
    V_RETURN( device->CreateInputLayout( g_aVertexLayout, 1, pBlobVS->GetBufferPointer(),
                                             pBlobVS->GetBufferSize(), &m_pVertexLayout ) );
    DXUT_SetDebugName( m_pVertexLayout11, "Primary" );

    SAFE_RELEASE( pBlobVS );
    SAFE_RELEASE( pBlobPS );

    // Query support for linear filtering on DXGI_FORMAT_R32G32B32A32
    UINT FormatSupport = 0;
    V_RETURN( device->CheckFormatSupport( DXGI_FORMAT_R32G32B32A32_FLOAT, &FormatSupport ) );

    // Setup linear or point sampler according to the format Query result
    D3D11_SAMPLER_DESC SamDesc;
    SamDesc.Filter = ( FormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE ) > 0 ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 1;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
    SamDesc.MinLOD = 0;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN( device->CreateSamplerState( &SamDesc, &m_pSampleState ) );  
    DXUT_SetDebugName( m_pSampleState, "Primary" );

    // Setup constant buffer
    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;
	Desc.ByteWidth = sizeof( CB_SKYBOX );
    V_RETURN( device->CreateBuffer( &Desc, NULL, &m_pcbVSPerObject ) );
    DXUT_SetDebugName( m_pcbVSPerObject, "CB_VS_PER_OBJECT" );
    
    // Depth stencil state
    D3D11_DEPTH_STENCIL_DESC DSDesc;
    ZeroMemory( &DSDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
    DSDesc.DepthEnable = FALSE;
    DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
    DSDesc.StencilEnable = FALSE;
    V_RETURN( device->CreateDepthStencilState( &DSDesc, &m_pDepthStencilState ) );
    DXUT_SetDebugName( m_pDepthStencilState, "DepthStencil" );

    // set default render state to msaa enabled
    D3D11_RASTERIZER_DESC drd = {
        D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
        D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
        FALSE, //BOOL FrontCounterClockwise;
        0, //INT DepthBias;
        0.0f,//FLOAT DepthBiasClamp;
        0.0f,//FLOAT SlopeScaledDepthBias;
        FALSE,//BOOL DepthClipEnable;
        FALSE,//BOOL ScissorEnable;
        TRUE,//BOOL MultisampleEnable;
        FALSE//BOOL AntialiasedLineEnable;        
    };
    device->CreateRasterizerState( &drd, &m_pRasterizerState );
	
	return S_OK;
}

void SkyBox::Resized( const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
    HRESULT hr;

    if ( m_pd3dDevice == NULL )
	{
        return;
	}

    // Fill the vertex buffer
    VERTEX_SKYBOX* pVertex = new VERTEX_SKYBOX[ 4 ];
    if ( !pVertex )
        return;

    // Map texels to pixels 
    float fHighW = -1.0f - ( 1.0f / ( float )pBackBufferSurfaceDesc->Width );
    float fHighH = -1.0f - ( 1.0f / ( float )pBackBufferSurfaceDesc->Height );
    float fLowW = 1.0f + ( 1.0f / ( float )pBackBufferSurfaceDesc->Width );
    float fLowH = 1.0f + ( 1.0f / ( float )pBackBufferSurfaceDesc->Height );
    
    pVertex[0].pos = XMFLOAT4( fLowW, fLowH, 1.0f, 1.0f );
    pVertex[1].pos = XMFLOAT4( fLowW, fHighH, 1.0f, 1.0f );
    pVertex[2].pos = XMFLOAT4( fHighW, fLowH, 1.0f, 1.0f );
    pVertex[3].pos = XMFLOAT4( fHighW, fHighH, 1.0f, 1.0f );

    UINT uiVertBufSize = 4 * sizeof( VERTEX_SKYBOX );
    //Vertex Buffer
    D3D11_BUFFER_DESC vbdesc;
    vbdesc.ByteWidth = uiVertBufSize;
    vbdesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbdesc.CPUAccessFlags = 0;
    vbdesc.MiscFlags = 0;    

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pVertex;    
    V( m_pd3dDevice->CreateBuffer( &vbdesc, &InitData, &m_pVertexBuffer ) );
    DXUT_SetDebugName( m_pVertexBuffer, "SkyBox" );
    SAFE_DELETE_ARRAY( pVertex ); 
}

void SkyBox::StoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext )
{
    pd3dImmediateContext->OMGetDepthStencilState( &m_pDepthStencilStateStored, &m_StencilRefStored );
    pd3dImmediateContext->RSGetState( &m_pRasterizerStateStored );
    pd3dImmediateContext->OMGetBlendState( &m_pBlendStateStored, m_BlendFactorStored, &m_SampleMaskStored );
    pd3dImmediateContext->PSGetSamplers( 0, 1, &m_pSamplerStateStored );
}

void SkyBox::RestoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext )
{
    pd3dImmediateContext->OMSetDepthStencilState( m_pDepthStencilStateStored, m_StencilRefStored );
    pd3dImmediateContext->RSSetState( m_pRasterizerStateStored );
    pd3dImmediateContext->OMSetBlendState( m_pBlendStateStored, m_BlendFactorStored, m_SampleMaskStored );
    pd3dImmediateContext->PSSetSamplers( 0, 1, &m_pSamplerStateStored );

    SAFE_RELEASE( m_pDepthStencilStateStored );
    SAFE_RELEASE( m_pRasterizerStateStored );
    SAFE_RELEASE( m_pBlendStateStored );
    SAFE_RELEASE( m_pSamplerStateStored );
}

void SkyBox::RenderSkyBox( XMMATRIX* pmWorldViewProj, ID3D11DeviceContext* pd3dImmediateContext )
{
    HRESULT hr;
    
	StoreD3D11State(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout( m_pVertexLayout );

    UINT uStrides = sizeof( VERTEX_SKYBOX );
    UINT uOffsets = 0;
    ID3D11Buffer* pBuffers[1] = { m_pVertexBuffer };
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &uStrides, &uOffsets );
    pd3dImmediateContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    pd3dImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( pd3dImmediateContext->Map( m_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
	CB_SKYBOX* pVSPerObject = ( CB_SKYBOX* )MappedResource.pData;

	pVSPerObject->m_WorldViewProj =( XMMatrixInverse( NULL, *pmWorldViewProj ) );
    pd3dImmediateContext->Unmap( m_pcbVSPerObject, 0 );
    pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pcbVSPerObject );

    pd3dImmediateContext->PSSetSamplers( 0, 1, &m_pSampleState );
    pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pEnvironmentRV );

    pd3dImmediateContext->OMSetDepthStencilState( m_pDepthStencilState, 0 );
	pd3dImmediateContext->OMSetBlendState(NULL, 0, 0xffffffff);
    pd3dImmediateContext->RSSetState( m_pRasterizerState );

	pd3dImmediateContext->Draw( 4, 0 );

	RestoreD3D11State(pd3dImmediateContext);
}

void SkyBox::OnD3D11ReleasingSwapChain()
{
    SAFE_RELEASE( m_pVertexBuffer );
}

void SkyBox::OnD3D11DestroyDevice()
{
    m_pd3dDevice = NULL;
    SAFE_RELEASE( m_pEnvironmentRV );
    SAFE_RELEASE( m_pEnvironmentMap );
    SAFE_RELEASE( m_pSampleState );
    SAFE_RELEASE( m_pVertexShader );
    SAFE_RELEASE( m_pPixelShader );
    SAFE_RELEASE( m_pVertexLayout );
    SAFE_RELEASE( m_pcbVSPerObject );
    SAFE_RELEASE( m_pDepthStencilState );
    SAFE_RELEASE( m_pRasterizerState );
	SAFE_RELEASE( m_pDepthStencilStateStored );
	m_StencilRefStored = 0;
	SAFE_RELEASE( m_pRasterizerStateStored );
	SAFE_RELEASE( m_pBlendStateStored );
    m_BlendFactorStored[0] = m_BlendFactorStored[1] = m_BlendFactorStored[2] = m_BlendFactorStored[3] = 0.0;
	m_SampleMaskStored = 0;
    SAFE_RELEASE( m_pSamplerStateStored );
}