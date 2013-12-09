#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "DXUTgui.h"

//#include "MultiTexturePresenter.h"
//#include "SpinningFirework.h"
//#include "GlowEffect.h"
//#include "MotionBlurEffect.h"

#include "Helpers.h"
#include "SkyBox.h"
#include "Terrain.h"

#define SUB_TEXTUREWIDTH 1440
#define SUB_TEXTUREHEIGHT 900

//--------------------------------------------------------------------------------------
//Global Variables
//--------------------------------------------------------------------------------------
//MultiTexturePresenter						MultiTexture = MultiTexturePresenter(1,true,SUB_TEXTUREWIDTH,SUB_TEXTUREHEIGHT);
//SpinningFirework							SpinFirework = SpinningFirework(SUB_TEXTUREWIDTH,SUB_TEXTUREHEIGHT);
//GlowEffect									PostEffect_Glow = GlowEffect(SUB_TEXTUREWIDTH,SUB_TEXTUREHEIGHT);
//MotionBlurEffect							PostEffect_Blur = MotionBlurEffect(SUB_TEXTUREWIDTH,SUB_TEXTUREHEIGHT);
SkyBox g_SkyBox;
Terrain g_Terrain;
CFirstPersonCamera g_Camera;                // A model viewing camera
CModelViewerCamera g_MCamera;
CDXUTDialogResourceManager					DialogResourceManager; 
CDXUTDialog									UI;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------

#define IDC_GLOWFACTOR_STATIC		1
#define IDC_GLOWFACTOR_SLIDER		2
#define IDC_GLOWBLENDFACTOR_STATIC	3
#define IDC_GLOWBLENDFACTOR_SLIDER	4
#define IDC_BLURFACTOR_STATIC		5
#define IDC_BLURFACTOR_SLIDER		6

#define IDC_FIREINTERVAL_STATIC		7
#define IDC_FIREINTERVAL_SLIDER		8
#define IDC_NUM_FLY1_STATIC			9
#define IDC_NUM_FLY1_SLIDER			10
#define IDC_MAX_SUBDETONATE_STATIC	11
#define IDC_MAX_SUBDETONATE_SLIDER	12
#define IDC_DETONATE_LIFE_STATIC	13
#define IDC_DETONATE_LIFE_SLIDER	14
#define IDC_FIREFLY_LIFE_STATIC		15
#define IDC_FIREFLY_LIFE_SLIDER		16
#define IDC_SDETONATE_LIFE_STATIC	17
#define IDC_SDETONATE_LIFE_SLIDER	18
#define IDC_FIREFLY2_LIFE_STATIC	19
#define IDC_FIREFLY2_LIFE_SLIDER	20


void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

//--------------------------------------------------------------------------------------
//Initialization
//--------------------------------------------------------------------------------------
HRESULT Initial()
{ 
	HRESULT hr = S_OK;

	if( AllocConsole() )
	{
		HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
		int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
		FILE* hf_out = _fdopen(hCrt, "w");
		setvbuf(hf_out, NULL, _IONBF, 1);
		*stdout = *hf_out;

		HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
		hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
		FILE* hf_in = _fdopen(hCrt, "r");
		setvbuf(hf_in, NULL, _IONBF, 128);
		*stdin = *hf_in;
	}


	//UI.Init( &DialogResourceManager );
	////UI.SetFont
	//UI.SetCallback( OnGUIEvent ); int iY = 10;

	//UI.SetFont( 1, L"Comic Sans MS", 400, 400 );
	//UI.SetFont( 2, L"Courier New", 16, FW_NORMAL );

	//WCHAR sz[100];
	//iY += 24;
	//swprintf_s( sz, 100, L"Glow factor: %0.2f", PostEffect_Glow.m_CBperResize.glow_factor );
	//UI.AddStatic( IDC_GLOWFACTOR_STATIC, sz, 0, iY, 170, 23 );
	//UI.AddSlider( IDC_GLOWFACTOR_SLIDER, 0, iY += 26, 170, 23, 0, 1000, 450 )£»

	// Setup Camera
	g_Camera.SetRotateButtons( true, false, false );
	g_Camera.SetScalers( /*0.003f, 400.0f*/0.005f, 150.0f );
	XMFLOAT3 vecEye(/*1562.24f, 854.291f, -1224.99f*/ 365.0f,  3.0f, 166.0f /*0,  3.0f, -15*/ );
	XMFLOAT3 vecAt (/*1562.91f, 854.113f, -1225.71f*/ 330.0f,-11.0f, 259.0f /*0,  0, -15*/ );
	g_Camera.SetViewParams( XMLoadFloat3( &vecEye ), XMLoadFloat3( &vecAt ) );

	return hr;
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
									  DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	//MultiTexture.ModifyDeviceSettings( pDeviceSettings );
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
									 void* pUserContext )
{
	HRESULT hr = S_OK;
	//ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	//V_RETURN( DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext));

	// Sky box
	ID3D11Texture2D* g_pSkyCubeMap = NULL;
	ID3D11ShaderResourceView* g_pSRV_SkyCube = NULL;
	WCHAR strPath[MAX_PATH];
	DXUTFindDXSDKMediaFileCch(strPath, MAX_PATH, L"misc\\sky_cube.dds");
	ID3D11Device* test = DXUTGetD3D11Device();
	V_RETURN( DXUTCreateShaderResourceViewFromFile( DXUTGetD3D11Device(), strPath, &g_pSRV_SkyCube ) );
	assert(g_pSRV_SkyCube);

	g_pSRV_SkyCube->GetResource( (ID3D11Resource**)&g_pSkyCubeMap );
	assert(g_pSkyCubeMap);

	g_SkyBox.Initialization( DXUTGetD3D11Device(), 50, g_pSkyCubeMap, g_pSRV_SkyCube );

	// Terrain
	g_Terrain.Initialize( DXUTGetD3D11Device() );
	g_Terrain.BackbufferWidth = 1280.0f;
	g_Terrain.BackbufferHeight = 720.0f;
	g_Terrain.ReCreateBuffers();

	XMVECTORF32 vecEye = { 0.0f, 60.0f, -600.0f, 0.f };
	XMVECTORF32 vecAt = { 0.0f, 0.0f, 0.0f, 0.f };
	g_MCamera.SetViewParams( vecEye, vecAt );
	g_MCamera.SetRadius( 60.0f, 1.0f, 200.0f );

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
										 const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;
	//V_RETURN( DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ));
	//UI.SetLocation( pBackBufferSurfaceDesc->Width - 180, 0 );
	//UI.SetSize( 180, 600 );

	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( 3.141592653 / 4, fAspectRatio, 0.1f, 200000.0f );
	XMFLOAT3 vMin = XMFLOAT3( -1000.0f, -1000.0f, -1000.0f );
	XMFLOAT3 vMax = XMFLOAT3( 1000.0f, 1000.0f, 1000.0f );

	//g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetRotateButtons(TRUE, TRUE, TRUE);
	g_Camera.SetScalers( 10.0f, 10.0f );
	g_Camera.SetDrag( true );
	g_Camera.SetEnableYAxisMovement( false );
	g_Camera.SetClipToBoundary( TRUE, &vMin, &vMax );
	g_Camera.FrameMove( 0 );
	g_SkyBox.Resized( pBackBufferSurfaceDesc );
	g_MCamera.SetProjParams( XM_PI / 4, fAspectRatio, 0.1f, 5000.0f );
	g_MCamera.SetWindow(pBackBufferSurfaceDesc->Width,pBackBufferSurfaceDesc->Height );
	g_MCamera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_Camera.FrameMove( fElapsedTime );
	g_MCamera.FrameMove( fElapsedTime );
	//SpinFirework.Update( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
								 double fTime, float fElapsedTime, void* pUserContext )
{
	float ClearColor[ 4 ] = { 0.176f, 0.196f, 0.667f, 0.0f };
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	XMMATRIX mView;
	XMMATRIX mProj;
	XMMATRIX mWorldViewProjection;
	//mView = g_Camera.GetViewMatrix();
	//mProj = g_Camera.GetProjMatrix();
	mView = g_MCamera.GetViewMatrix();
	mProj = g_MCamera.GetProjMatrix();
	//XMMATRIX rotate2 = XMMatrixRotationZ( -90 );
	//mView = XMMatrixMultiply( rotate2, mView );
	mWorldViewProjection = XMMatrixMultiply( mView, mProj );
	

	//g_SkyBox.RenderSkyBox( &mWorldViewProjection, pd3dImmediateContext );
	XMMATRIX rotate =  XMMatrixRotationY(fTime);
	g_Terrain.Render( &g_MCamera, pd3dImmediateContext,&rotate, fTime, &g_SkyBox );

	//DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR2, L"UI" );
	//UI.OnRender( fElapsedTime );
	//DXUT_EndPerfEvent();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{

	DialogResourceManager.OnD3D11ReleasingSwapChain();
	g_SkyBox.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	g_SkyBox.OnD3D11DestroyDevice();
	g_Terrain.OnD3D11DestroyDevice();
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						 bool* pbNoFurtherProcessing, void* pUserContext )
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = UI.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	// Pass all windows messages to camera so it can respond to user input
	g_MCamera.HandleMessages( hWnd, uMsg, wParam, lParam );
	//g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
					  bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
					  int xPos, int yPos, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	//switch( nControlID )
	//{
	//	case IDC_GLOWFACTOR_SLIDER:
	//		{
	//			WCHAR sz[100];
	//			float glowFactor= ( float )( UI.GetSlider( IDC_GLOWFACTOR_SLIDER )->GetValue() * 0.01f );
	//			swprintf_s( sz, 100, L"Glow Factor: %0.2f", glowFactor);
	//			PostEffect_Glow.m_CBperResize.glow_factor = glowFactor;
	//			UI.GetStatic( IDC_GLOWFACTOR_STATIC )->SetText( sz );
	//			break;
	//		}

}

//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// DXUT will create and use the best device (either D3D9 or D3D11) 
	// that is available on the system depending on which D3D callbacks are set below

	// Set general DXUT callbacks
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackKeyboard( OnKeyboard );
	DXUTSetCallbackMouse( OnMouse );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );


	// Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

	// Perform any application-level initialization here

	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen

	Initial();

	DXUTCreateWindow( L"Ocean Animation" );

	// Only require 10-level hardware
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1280, 720 );
	DXUTMainLoop(); // Enter into the DXUT ren  der loop

	// Perform any application-level cleanup here

	return DXUTGetExitCode();
}


