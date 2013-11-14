#pragma once

#include "Helpers.h"

class SkyBox
{
public:
	SkyBox(void);
	~SkyBox(void);

	HRESULT Initialization( ID3D11Device* device, float fSize, ID3D11Texture2D* pCubeTexture, ID3D11ShaderResourceView* pCubeRV );
	void    Resized( const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );
    void    RenderSkyBox( XMMATRIX* pmWorldViewProj, ID3D11DeviceContext* pd3dImmediateContext );
    void    OnD3D11ReleasingSwapChain();
    void    OnD3D11DestroyDevice();

	/*
	* Setters and Getters
	*/
    ID3D11Texture2D* GetD3D11EnvironmentMap() { return m_pEnvironmentMap; }
    ID3D11ShaderResourceView* GetD3D11EnvironmentMapRV() { return m_pEnvironmentRV; }
    void SetD3D11EnvironmentMap( ID3D11Texture2D* pCubeTexture ) { m_pEnvironmentMap = pCubeTexture; }

private:
	struct CB_SKYBOX
	{
		 XMMATRIX m_WorldViewProj;
	};

	struct VERTEX_SKYBOX
	{
		XMFLOAT4 pos;
	};

private:
    ID3D11Texture2D* m_pEnvironmentMap;
    ID3D11ShaderResourceView* m_pEnvironmentRV;
    ID3D11Device* m_pd3dDevice;

    ID3D11VertexShader* m_pVertexShader;
    ID3D11PixelShader* m_pPixelShader;
    ID3D11SamplerState* m_pSampleState;

    ID3D11InputLayout* m_pVertexLayout;
    ID3D11Buffer* m_pcbVSPerObject;
    ID3D11Buffer* m_pVertexBuffer;
    ID3D11DepthStencilState* m_pDepthStencilState;
    ID3D11RasterizerState* m_pRasterizerState;

	// Variables for storing d3d states
	ID3D11DepthStencilState* m_pDepthStencilStateStored;
    UINT m_StencilRefStored;
    ID3D11RasterizerState* m_pRasterizerStateStored;
    ID3D11BlendState* m_pBlendStateStored;
    float m_BlendFactorStored[4];
    UINT m_SampleMaskStored;
    ID3D11SamplerState* m_pSamplerStateStored;

	float m_fSize;

private:
	void StoreD3D11State( ID3D11DeviceContext* );
	void RestoreD3D11State( ID3D11DeviceContext* );

};

