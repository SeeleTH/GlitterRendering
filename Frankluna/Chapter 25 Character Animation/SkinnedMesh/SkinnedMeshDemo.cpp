//***************************************************************************************
// SkinnedMeshDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates skinned mesh character animation.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//      Press '1' for wireframe
//
//***************************************************************************************
#include "SkinnedMeshDemo.h"
#include "BasicGeometryMeshComponent.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NTextureNResLoader.h"
#include "../../../NBoxLib/Source/Core/ResLoaders/NShaderNResLoader.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	if (!SkinnedMeshApp::Instance().Init(hInstance))
		return 0;
	
	return SkinnedMeshApp::Instance().Run();
}
 

SkinnedMeshApp::SkinnedMeshApp()
: mSky(0), mCharacterModel(0),  
  mShapesVB(0), mShapesIB(0), mSkullVB(0), mSkullIB(0), mScreenQuadVB(0), mScreenQuadIB(0),
  mStoneTexSRV(0), mBrickTexSRV(0),
  mStoneNormalTexSRV(0), mBrickNormalTexSRV(0), 
  mSkullIndexCount(0), mSmap(0), mSsao(0),
  mLightRotationAngle(0.0f)

  // NBox
  , m_pWin32(NULL)
  , m_pGraphicsDevice(NULL)
  , m_pRenderManager(NULL)
  , m_pResCache(NULL)
  , m_pTime(NULL)
  , m_pActorManager(NULL)
  , m_pAssetGatherer(NULL)
  , m_pTexturePool(NULL)
  , m_pGlitterModelPool(NULL)

#ifdef CAMERA_SHOWROOM
  , m_fCameraShowroomProg(CAMERA_SHOWROOM_PROGOFFSET)
#endif
{

	//mMainWndCaption = L"Skinned Mesh Demo";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	// Estimate the scene bounding sphere manually since we know how the scene was constructed.
	// The grid is the "widest object" with a width of 20 and depth of 30.0f, and centered at
	// the world space origin.  In general, you need to loop over every world space vertex
	// position and compute the bounding sphere.
	mSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mSkullWorld, XMMatrixMultiply(skullScale, skullOffset));

	for(int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&mCylWorld[i*2+0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mCylWorld[i*2+1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		XMStoreFloat4x4(&mSphereWorld[i*2+0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&mSphereWorld[i*2+1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}

	mDirLights[0].Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(1.0f, 0.9f, 0.9f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	// Shadow acne gets worse as we increase the slope of the polygon (from the
	// perspective of the light).
	//mDirLights[0].Direction = XMFLOAT3(5.0f/sqrtf(50.0f), -5.0f/sqrtf(50.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(125.0f), -5.0f/sqrtf(125.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(116.0f), -4.0f/sqrtf(116.0f), 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(10.0f/sqrtf(109.0f), -3.0f/sqrtf(109.0f), 0.0f);

	mDirLights[1].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[2].Ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);

	mOriginalLightDir[0] = mDirLights[0].Direction;
	mOriginalLightDir[1] = mDirLights[1].Direction;
	mOriginalLightDir[2] = mDirLights[2].Direction;

	mGridMat.Ambient  = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mGridMat.Diffuse  = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mGridMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mGridMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mCylinderMat.Ambient  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mCylinderMat.Diffuse  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	mCylinderMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSphereMat.Ambient  = XMFLOAT4(0.3f, 0.4f, 0.5f, 1.0f);
	mSphereMat.Diffuse  = XMFLOAT4(0.2f, 0.3f, 0.4f, 1.0f);
	mSphereMat.Specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);
	mSphereMat.Reflect  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

	mBoxMat.Ambient  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
	mBoxMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mSkullMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 16.0f);
	mSkullMat.Reflect  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
}

SkinnedMeshApp::~SkinnedMeshApp()
{
	m_pRenderManager->VDestroy();
	N_DELETE(m_pRenderManager);

	//if (m_pAssetGatherer)
	//	m_pAssetGatherer->Stop();
	N_DELETE(m_pAssetGatherer);
	N_DELETE(m_pGlitterModelPool);
	N_DELETE(m_pTexturePool);

    N_DELETE(m_pActorManager);
    N_DELETE(m_pTime);
	N_DELETE(m_pResCache);
    N_DELETE(m_pGraphicsDevice);
    N_DELETE(m_pWin32);

	N_DELETE(mCharacterModel);

	N_DELETE(mSky);
	N_DELETE(mSmap);
	N_DELETE(mSsao);

	N_RELEASE(mShapesVB);
	N_RELEASE(mShapesIB);
	N_RELEASE(mSkullVB);
	N_RELEASE(mSkullIB);
	N_RELEASE(mScreenQuadVB);
	N_RELEASE(mScreenQuadIB);

	RenderStates::DestroyAll();
	InputLayouts::DestroyAll(); 
	Effects::DestroyAll();
}


int SkinnedMeshApp::Run()
{
    if (!m_pWin32)
        return -1;

    BOOL isQuit = false;


    while (!isQuit)
    {
        m_pTime->Update(m_pWin32->GetActivated());

        isQuit = !m_pWin32->Run();

        if (m_pWin32->GetActivated())
        {

            CalculateFrameStats();
            UpdateScene(m_pTime->GetDeltaFloat());
			m_pRenderManager->VRender();
            //DrawScene();
        }
        else
        {
            Sleep(100);
        }
    }

    return (m_pWin32)?m_pWin32->GetQuitParam():-1;
}

void SkinnedMeshApp::CalculateFrameStats()
{
    // Code computes the average frames per second, and also the 
    // average time it takes to render one frame.  These stats 
    // are appended to the window caption bar.

    static int frameCnt = 0;
    static UINT64 microTimeElapsed = 0;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_pTime->GetCurMicros() - microTimeElapsed) >= 1000000)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_pWin32->GetMainWindowCaption() << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_pWin32->GetHWND(), outs.str().c_str());

        // Reset for next average.
        frameCnt = 0;
        microTimeElapsed += 1000000;
    }
}

bool SkinnedMeshApp::Init(HINSTANCE hInstance)
{
	// NBoxLib
	m_pWin32 = new SkinnedMeshNWin32(hInstance);
	m_pGraphicsDevice = new NGraphicsDevice();

    N_ASSERT(m_pWin32);
    N_ASSERT(m_pGraphicsDevice);

    if (!m_pWin32 || !m_pWin32->VInit())
        return false;

    m_pGraphicsDevice->PreInit();
    if (!m_pGraphicsDevice->Init(m_pWin32->GetHWND(), m_pWin32->GetClientWidth(), m_pWin32->GetClientHeight()))
        return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(m_pGraphicsDevice->GetD3D11Device());
	InputLayouts::InitAll(m_pGraphicsDevice->GetD3D11Device());
	RenderStates::InitAll(m_pGraphicsDevice->GetD3D11Device());

	mCam.SetLens(0.25f*MathHelper::Pi, m_pWin32->GetAspectRatio(), 1.0f, 1000.0f);
#ifndef SKIP_USELESS_LOADING
	mTexMgr.Init(m_pGraphicsDevice->GetD3D11Device());

	mSky  = new Sky(m_pGraphicsDevice->GetD3D11Device(), L"Textures/desertcube1024.dds", 5000.0f);
	mSmap = new ShadowMap(m_pGraphicsDevice->GetD3D11Device(), SMapSize, SMapSize);

    mSsao = new Ssao(m_pGraphicsDevice->GetD3D11Device(), m_pGraphicsDevice->GetD3D11DeviceContext(), m_pWin32->GetClientWidth(), m_pWin32->GetClientHeight(), mCam.GetFovY(), mCam.GetFarZ());

	mStoneTexSRV = mTexMgr.CreateTexture(L"Textures/floor.dds");
	mBrickTexSRV = mTexMgr.CreateTexture(L"Textures/bricks.dds");
	mStoneNormalTexSRV = mTexMgr.CreateTexture(L"Textures/floor_nmap.dds");
	mBrickNormalTexSRV = mTexMgr.CreateTexture(L"Textures/bricks_nmap.dds");

	BuildShapeGeometryBuffers();
	BuildSkullGeometryBuffers();
	BuildScreenQuadGeometryBuffers();

	mCharacterModel = new SkinnedModel(m_pGraphicsDevice->GetD3D11Device(), mTexMgr, "Models\\soldier.m3d", L"Textures\\");
	mCharacterInstance1.Model = mCharacterModel;
	mCharacterInstance2.Model = mCharacterModel;
	mCharacterInstance1.TimePos = 0.0f;
	mCharacterInstance2.TimePos = 0.0f;
	mCharacterInstance1.ClipName = "Take1";
	mCharacterInstance2.ClipName = "Take1";
	mCharacterInstance1.FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());
	mCharacterInstance2.FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());

	// Reflect to change coordinate system from the RHS the data was exported out as.
	XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
	XMMATRIX modelRot   = XMMatrixRotationY(MathHelper::Pi);
	XMMATRIX modelOffset = XMMatrixTranslation(-2.0f, 0.0f, -7.0f);
	XMStoreFloat4x4(&mCharacterInstance1.World, modelScale*modelRot*modelOffset);

	modelOffset = XMMatrixTranslation(2.0f, 0.0f, -7.0f);
	XMStoreFloat4x4(&mCharacterInstance2.World, modelScale*modelRot*modelOffset);
#endif
    // Time
    if (!m_pTime)
        m_pTime = new NTime();

    // ResCache
    if (!m_pResCache)
        m_pResCache = new NResCache(2048, new NDirResFile(L""));
    if (!m_pResCache->Init())
        return false;
	m_pResCache->RegisterLoader(std::shared_ptr<INResLoader>(new NDDSTextureNResLoader(m_pGraphicsDevice)));
	m_pResCache->RegisterLoader(std::shared_ptr<INResLoader>(new NVertexShaderNResLoader(m_pGraphicsDevice)));
	m_pResCache->RegisterLoader(std::shared_ptr<INResLoader>(new NPixelShaderNResLoader(m_pGraphicsDevice)));
	m_pResCache->RegisterLoader(std::shared_ptr<INResLoader>(new NGeometryShaderNResLoader(m_pGraphicsDevice)));
	m_pResCache->RegisterLoader(std::shared_ptr<INResLoader>(new NComputeShaderNResLoader(m_pGraphicsDevice)));

    // ActorManager
    m_pActorManager = new NActorManager();
    m_pActorManager->VInit();
	m_pActorManager->RegisterComponent<BasicGeometryMeshComponent>(BasicGeometryMeshComponent::g_sName);

	// AssetGatherer
	m_pAssetGatherer = new NAssetGatherer();
	m_pAssetGatherer->Init();

	m_pTexturePool = new NTexture2DPool(m_pAssetGatherer);
	m_pTexturePool->Init(m_pGraphicsDevice);

	m_pGlitterModelPool = new NGlitterModelPool(m_pGraphicsDevice, m_pAssetGatherer, m_pTexturePool);

	// RenderManager
	m_pRenderManager = new SkinnedMeshRenderManager(m_pGraphicsDevice, m_pResCache, &mCam);
	m_pRenderManager->VInit();

    NRes testActor("Actors\\testactor.xml");
    std::shared_ptr<NResHandle> testActorHandle = m_pResCache->GetHandle(&testActor);
	DirectX::XMFLOAT4X4 identity;
	DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
	m_pActorManager->VCreateActor(testActorHandle->GetBuffer(), testActorHandle->GetSize(), { 0.f, 0.f, 0.f }, identity);
	
	return true;
}

void SkinnedMeshApp::OnResize()
{
    m_pGraphicsDevice->OnResize(m_pWin32->GetClientWidth(), m_pWin32->GetClientHeight());

    mCam.SetLens(0.25f*MathHelper::Pi, m_pWin32->GetAspectRatio(), 1.0f, 1000.0f);

	if( mSsao )
	{
		mSsao->OnSize(m_pWin32->GetClientWidth(), m_pWin32->GetClientHeight(), mCam.GetFovY(), mCam.GetFarZ());
	}
}

void SkinnedMeshApp::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
#ifdef CAMERA_SHOWROOM
	{
		float temp;
		m_fCameraShowroomProg += (dt * CAMERA_SHOWROOM_FREQ);
		m_fCameraShowroomProg = modf(m_fCameraShowroomProg, &temp);
		float camHDist = CAMERA_SHOWROOM_DIST * cos(CAMERA_SHOWROOM_YAWDEG * XM_PI / 180.f);
		float camProgRad = m_fCameraShowroomProg * XM_PI * 2.f;
		XMFLOAT3 camTar;
		XMFLOAT3 camPos;
		XMFLOAT3 camUp;
		camTar.x = CAMERA_SHOWROOM_TARGET_X;
		camTar.y = CAMERA_SHOWROOM_TARGET_Y;
		camTar.z = CAMERA_SHOWROOM_TARGET_Z;
		camPos.y = camTar.y + CAMERA_SHOWROOM_DIST * sin(CAMERA_SHOWROOM_YAWDEG * XM_PI / 180.f);
		camPos.x = camTar.x + camHDist * sin(camProgRad);
		camPos.z = camTar.z + camHDist * cos(camProgRad);
		camUp.x = 0.f;
		camUp.y = 1.0f;
		camUp.z = 0.f;
		mCam.LookAt(camPos, camTar, camUp);
	}
#else
	if( GetAsyncKeyState('W') & 0x8000 )
		mCam.Walk(10.0f*dt);

	if( GetAsyncKeyState('S') & 0x8000 )
		mCam.Walk(-10.0f*dt);

	if( GetAsyncKeyState('A') & 0x8000 )
		mCam.Strafe(-10.0f*dt);

	if( GetAsyncKeyState('D') & 0x8000 )
		mCam.Strafe(10.0f*dt);
#endif

    const SkinnedMeshNWin32::TempMouseBehaviorInfo* mouseInfo = m_pWin32->GetMouseInfo();
    if (mouseInfo->m_bIsDowned)
    {
        if (mouseInfo->m_fDY > 0.000001f || mouseInfo->m_fDY < -0.000001f)
            mCam.Pitch(mouseInfo->m_fDY);
        if (mouseInfo->m_fDX > 0.000001f || mouseInfo->m_fDX < -0.000001f)
            mCam.RotateY(mouseInfo->m_fDX);
    }

    if (m_pActorManager)
        m_pActorManager->VUpdate(dt);

	if(m_pAssetGatherer)
		m_pAssetGatherer->OnUpdate();

	//
	// Animate the character.
	// 
#ifndef SKIP_USELESS_LOADING
	mCharacterInstance1.Update(dt);
	mCharacterInstance2.Update(dt);

	//
	// Animate the lights (and hence shadows).
	//

	BuildShadowTransform();
#endif
	mCam.UpdateViewMatrix();
}

void SkinnedMeshApp::DrawScene()
{
	//
	// Render the scene to the shadow map.
	//
	mSmap->BindDsvAndSetNullRenderTarget(m_pGraphicsDevice->GetD3D11DeviceContext());

	DrawSceneToShadowMap();

	m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(0);

	//
	// Render the view space normals and depths.  This render target has the
	// same dimensions as the back buffer, so we can use the screen viewport.
	// This render pass is needed to compute the ambient occlusion.
	// Notice that we use the main depth/stencil buffer in this pass.  
	//
    m_pGraphicsDevice->Clear(N_B_DEPTH_BUFFER | N_B_STENCIL_BUFFER);
	m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetViewports(1, (D3D11_VIEWPORT*)m_pGraphicsDevice->GetScreenViewport());
    mSsao->SetNormalDepthRenderTarget(m_pGraphicsDevice->GetDepthStencilView());
	
	DrawSceneToSsaoNormalDepthMap();

	//
	// Now compute the ambient occlusion.
	//

	mSsao->ComputeSsao(mCam);
	mSsao->BlurAmbientMap(2);

	//
	// Restore the back and depth buffer and viewport to the OM stage.
	//
	ID3D11RenderTargetView* renderTargets[1] = {m_pGraphicsDevice->GetRenderTargetView()};
    m_pGraphicsDevice->GetD3D11DeviceContext()->OMSetRenderTargets(1, renderTargets, m_pGraphicsDevice->GetDepthStencilView());
	m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetViewports(1, (D3D11_VIEWPORT*)m_pGraphicsDevice->GetScreenViewport());

    m_pGraphicsDevice->GetD3D11DeviceContext()->ClearRenderTargetView(m_pGraphicsDevice->GetRenderTargetView(), reinterpret_cast<const float*>(&Colors::Silver));

	// We already laid down scene depth to the depth buffer in the Normal/Depth map pass,
	// so we can set the depth comparison test to “EQUALS.E This prevents any overdraw
	// in this rendering pass, as only the nearest visible pixels will pass this depth
	// comparison test.

	m_pGraphicsDevice->GetD3D11DeviceContext()->OMSetDepthStencilState(RenderStates::EqualsDSS, 0);
 
	XMMATRIX view     = mCam.View();
	XMMATRIX proj     = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());
	Effects::BasicFX->SetCubeMap(mSky->CubeMapSRV());
	Effects::BasicFX->SetShadowMap(mSmap->DepthMapSRV());
	Effects::BasicFX->SetSsaoMap(mSsao->AmbientSRV());

	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());
	Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());
	Effects::NormalMapFX->SetShadowMap(mSmap->DepthMapSRV());
	Effects::NormalMapFX->SetSsaoMap(mSsao->AmbientSRV());

	// Figure out which technique to use for different geometry.
	ID3DX11EffectTechnique* activeTech        = Effects::NormalMapFX->Light3TexTech;
	ID3DX11EffectTechnique* activeSphereTech  = Effects::BasicFX->Light3ReflectTech;
	ID3DX11EffectTechnique* activeSkullTech   = Effects::BasicFX->Light3ReflectTech;
	ID3DX11EffectTechnique* activeSkinnedTech = Effects::NormalMapFX->Light3TexSkinnedTech;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX shadowTransform = XMLoadFloat4x4(&mShadowTransform);

	//
	// Draw the grid, cylinders, and box without any cubemap reflection.
	// 
	
	UINT stride = sizeof(Vertex::PosNormalTexTan);
    UINT offset = 0;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::PosNormalTexTan);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);
     
	if( GetAsyncKeyState('1') & 0x8000 )
		m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(RenderStates::WireframeRS);
	
    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the grid.
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));
		Effects::NormalMapFX->SetMaterial(mGridMat);
		Effects::NormalMapFX->SetDiffuseMap(mStoneTexSRV);
		Effects::NormalMapFX->SetNormalMap(mStoneNormalTexSRV);

		activeTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
		Effects::NormalMapFX->SetMaterial(mBoxMat);
		Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
		Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);

		activeTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw the cylinders.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));
			Effects::NormalMapFX->SetMaterial(mCylinderMat);
			Effects::NormalMapFX->SetDiffuseMap(mBrickTexSRV);
			Effects::NormalMapFX->SetNormalMap(mBrickNormalTexSRV);
 
			activeTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}
    }

	//
	// Draw the spheres with cubemap reflection.
	//
 
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	activeSphereTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the spheres.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::BasicFX->SetShadowTransform(world*shadowTransform);
			Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
			Effects::BasicFX->SetMaterial(mSphereMat);
 
			activeSphereTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
	}

	stride = sizeof(Vertex::Basic32);
    offset = 0;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::Basic32);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	activeSkullTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the skull.
		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::BasicFX->SetMaterial(mSkullMat);
		Effects::BasicFX->SetShadowTransform(world*shadowTransform);
	 
		activeSkullTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	//
	// Draw the animated characters.
	//

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

	activeSkinnedTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Instance 1

		world = XMLoadFloat4x4(&mCharacterInstance1.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
		Effects::NormalMapFX->SetBoneTransforms(
			&mCharacterInstance1.FinalTransforms[0], 
			mCharacterInstance1.FinalTransforms.size());

		for(UINT subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
		{
			Effects::NormalMapFX->SetMaterial(mCharacterInstance1.Model->Mat[subset]);
			Effects::NormalMapFX->SetDiffuseMap(mCharacterInstance1.Model->DiffuseMapSRV[subset]);
			Effects::NormalMapFX->SetNormalMap(mCharacterInstance1.Model->NormalMapSRV[subset]);

			activeSkinnedTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			mCharacterInstance1.Model->ModelMesh.Draw(m_pGraphicsDevice->GetD3D11DeviceContext(), subset);
		}

		// Instance 2

		world = XMLoadFloat4x4(&mCharacterInstance2.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world*shadowTransform);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
		
		Effects::NormalMapFX->SetBoneTransforms(
			&mCharacterInstance2.FinalTransforms[0], 
			mCharacterInstance2.FinalTransforms.size());

		for(UINT subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
		{
			Effects::NormalMapFX->SetMaterial(mCharacterInstance2.Model->Mat[subset]);
			Effects::NormalMapFX->SetDiffuseMap(mCharacterInstance2.Model->DiffuseMapSRV[subset]);
			Effects::NormalMapFX->SetNormalMap(mCharacterInstance2.Model->NormalMapSRV[subset]);

			activeSkinnedTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			mCharacterInstance2.Model->ModelMesh.Draw(m_pGraphicsDevice->GetD3D11DeviceContext(), subset);
		}
	}
	
	// Turn off wireframe.
	m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(0);

	// Restore from RenderStates::EqualsDSS
	m_pGraphicsDevice->GetD3D11DeviceContext()->OMSetDepthStencilState(0, 0);

	// Debug view SSAO map.
	DrawScreenQuad(mSsao->AmbientSRV());

	mSky->Draw(m_pGraphicsDevice->GetD3D11DeviceContext(), mCam);

	// restore default states, as the SkyFX changes them in the effect file.
	m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(0);
	m_pGraphicsDevice->GetD3D11DeviceContext()->OMSetDepthStencilState(0, 0);

	// Unbind shadow map and AmbientMap as a shader input because we are going to render
	// to it next frame.  These textures can be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	m_pGraphicsDevice->GetD3D11DeviceContext()->PSSetShaderResources(0, 16, nullSRV);

    m_pGraphicsDevice->Present();
}


void SkinnedMeshApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

    SetCapture(m_pWin32->GetHWND());
}

void SkinnedMeshApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SkinnedMeshApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void SkinnedMeshApp::DrawSceneToSsaoNormalDepthMap()
{
	XMMATRIX view     = mCam.View();
	XMMATRIX proj     = mCam.Proj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	ID3DX11EffectTechnique* tech = Effects::SsaoNormalDepthFX->NormalDepthTech;
	ID3DX11EffectTechnique* animatedTech = Effects::SsaoNormalDepthFX->NormalDepthSkinnedTech;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldView;
	XMMATRIX worldInvTransposeView;
	XMMATRIX worldViewProj;

	//
	// Draw the grid, cylinders, spheres and box.

	UINT stride = sizeof(Vertex::PosNormalTexTan);
    UINT offset = 0;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::PosNormalTexTan);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);

	if( GetAsyncKeyState('1') & 0x8000 )
		m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(RenderStates::WireframeRS);
     
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the grid.
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView     = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));

		tech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView     = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));

		tech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw the cylinders.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldView     = world*view;
			worldInvTransposeView = worldInvTranspose*view;
			worldViewProj = world*view*proj;

			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// Draw the spheres.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldView     = world*view;
			worldInvTransposeView = worldInvTranspose*view;
			worldViewProj = world*view*proj;

			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());

			tech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
    }
 
	//
	// Draw the skull.
	//

	stride = sizeof(Vertex::Basic32);
    offset = 0;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::Basic32);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView     = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());

		tech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	//
	// Draw the animated characters.
	//

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

	animatedTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Instance 1

		world = XMLoadFloat4x4(&mCharacterInstance1.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView     = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());
		Effects::SsaoNormalDepthFX->SetBoneTransforms(
			&mCharacterInstance1.FinalTransforms[0], 
			mCharacterInstance1.FinalTransforms.size());

		animatedTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());

		for(UINT subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
		{
			mCharacterInstance1.Model->ModelMesh.Draw(m_pGraphicsDevice->GetD3D11DeviceContext(), subset);
		}

		// Instance 2

		world = XMLoadFloat4x4(&mCharacterInstance2.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView     = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());
		Effects::SsaoNormalDepthFX->SetBoneTransforms(
			&mCharacterInstance2.FinalTransforms[0], 
			mCharacterInstance2.FinalTransforms.size());

		animatedTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());

		for(UINT subset = 0; subset < mCharacterInstance2.Model->SubsetCount; ++subset)
		{
			mCharacterInstance2.Model->ModelMesh.Draw(m_pGraphicsDevice->GetD3D11DeviceContext(), subset);
		}
	}

	m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(0);
}

void SkinnedMeshApp::DrawSceneToShadowMap()
{
	XMMATRIX view     = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj     = XMLoadFloat4x4(&mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	Effects::BuildShadowMapFX->SetEyePosW(mCam.GetPosition());
	Effects::BuildShadowMapFX->SetViewProj(viewProj);

	// These properties could be set per object if needed.
	Effects::BuildShadowMapFX->SetHeightScale(0.07f);
	Effects::BuildShadowMapFX->SetMaxTessDistance(1.0f);
	Effects::BuildShadowMapFX->SetMinTessDistance(25.0f);
	Effects::BuildShadowMapFX->SetMinTessFactor(1.0f);
	Effects::BuildShadowMapFX->SetMaxTessFactor(5.0f);

	ID3DX11EffectTechnique* smapTech = Effects::BuildShadowMapFX->BuildShadowMapTech;
	ID3DX11EffectTechnique* animatedSmapTech = Effects::BuildShadowMapFX->BuildShadowMapSkinnedTech;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	//
	// Draw the grid, cylinders, spheres, and box.
	// 

	UINT stride = sizeof(Vertex::PosNormalTexTan);
    UINT offset = 0;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::PosNormalTexTan);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mShapesVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mShapesIB, DXGI_FORMAT_R32_UINT, 0);
     
	if( GetAsyncKeyState('1') & 0x8000 )
		m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(RenderStates::WireframeRS);
	
    D3DX11_TECHNIQUE_DESC techDesc;
    smapTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the grid.
		world = XMLoadFloat4x4(&mGridWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(8.0f, 10.0f, 1.0f));

		smapTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Draw the box.
		world = XMLoadFloat4x4(&mBoxWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));

		smapTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Draw the cylinders.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCylWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));

			smapTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
		}

		// Draw the spheres.
		for(int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mSphereWorld[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());

			smapTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
			m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
		}
    }

	//
	// Draw the skull.
	//
	stride = sizeof(Vertex::Basic32);
    offset = 0;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::Basic32);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mSkullVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		world = XMLoadFloat4x4(&mSkullWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());

		smapTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(mSkullIndexCount, 0, 0);
	}

	//
	// Draw the animated characters.
	//

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

	animatedSmapTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Instance 1

		world = XMLoadFloat4x4(&mCharacterInstance1.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());
		Effects::BuildShadowMapFX->SetBoneTransforms(
			&mCharacterInstance1.FinalTransforms[0], 
			mCharacterInstance1.FinalTransforms.size());


		animatedSmapTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());

		for(UINT subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
		{
			mCharacterInstance1.Model->ModelMesh.Draw(m_pGraphicsDevice->GetD3D11DeviceContext(), subset);
		}

		// Instance 2

		world = XMLoadFloat4x4(&mCharacterInstance2.World);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());
		Effects::BuildShadowMapFX->SetBoneTransforms(
			&mCharacterInstance2.FinalTransforms[0], 
			mCharacterInstance2.FinalTransforms.size());

		animatedSmapTech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());

		for(UINT subset = 0; subset < mCharacterInstance1.Model->SubsetCount; ++subset)
		{
			mCharacterInstance2.Model->ModelMesh.Draw(m_pGraphicsDevice->GetD3D11DeviceContext(), subset);
		}
	}

	m_pGraphicsDevice->GetD3D11DeviceContext()->RSSetState(0);
}

void SkinnedMeshApp::DrawScreenQuad(ID3D11ShaderResourceView* srv)
{
	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetInputLayout(InputLayouts::Basic32);
    m_pGraphicsDevice->GetD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	m_pGraphicsDevice->GetD3D11DeviceContext()->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);
 
	// Scale and shift quad to lower-right corner.
	XMMATRIX world(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f);

	ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewRedTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		Effects::DebugTexFX->SetWorldViewProj(world);
		Effects::DebugTexFX->SetTexture(srv);

		tech->GetPassByIndex(p)->Apply(0, m_pGraphicsDevice->GetD3D11DeviceContext());
		m_pGraphicsDevice->GetD3D11DeviceContext()->DrawIndexed(6, 0, 0);
    }
}

void SkinnedMeshApp::BuildShadowTransform()
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
	XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = V*P*T;

	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S);
}

void SkinnedMeshApp::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 50, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset      = 0;
	mGridVertexOffset     = box.Vertices.size();
	mSphereVertexOffset   = mGridVertexOffset + grid.Vertices.size();
	mCylinderVertexOffset = mSphereVertexOffset + sphere.Vertices.size();

	// Cache the index count of each object.
	mBoxIndexCount      = box.Indices.size();
	mGridIndexCount     = grid.Indices.size();
	mSphereIndexCount   = sphere.Indices.size();
	mCylinderIndexCount = cylinder.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset      = 0;
	mGridIndexOffset     = mBoxIndexCount;
	mSphereIndexOffset   = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;
	
	UINT totalVertexCount = 
		box.Vertices.size() + 
		grid.Vertices.size() + 
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	UINT totalIndexCount = 
		mBoxIndexCount + 
		mGridIndexCount + 
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::PosNormalTexTan> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = box.Vertices[i].Position;
		vertices[k].Normal   = box.Vertices[i].Normal;
		vertices[k].Tex      = box.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			box.Vertices[i].TangentU.x,
			box.Vertices[i].TangentU.y,
			box.Vertices[i].TangentU.z,
			1.0f);
	}

	for(size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = grid.Vertices[i].Position;
		vertices[k].Normal   = grid.Vertices[i].Normal;
		vertices[k].Tex      = grid.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			grid.Vertices[i].TangentU.x,
			grid.Vertices[i].TangentU.y,
			grid.Vertices[i].TangentU.z,
			1.0f);
	}

	for(size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = sphere.Vertices[i].Position;
		vertices[k].Normal   = sphere.Vertices[i].Normal;
		vertices[k].Tex      = sphere.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			sphere.Vertices[i].TangentU.x,
			sphere.Vertices[i].TangentU.y,
			sphere.Vertices[i].TangentU.z,
			1.0f);
	}

	for(size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos      = cylinder.Vertices[i].Position;
		vertices[k].Normal   = cylinder.Vertices[i].Normal;
		vertices[k].Tex      = cylinder.Vertices[i].TexC;
		vertices[k].TangentU = XMFLOAT4(
			cylinder.Vertices[i].TangentU.x,
			cylinder.Vertices[i].TangentU.y,
			cylinder.Vertices[i].TangentU.z,
			1.0f);
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormalTexTan) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(m_pGraphicsDevice->GetD3D11Device()->CreateBuffer(&vbd, &vinitData, &mShapesVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(m_pGraphicsDevice->GetD3D11Device()->CreateBuffer(&ibd, &iinitData, &mShapesIB));
}
 
void SkinnedMeshApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");
	
	if(!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;
	
	std::vector<Vertex::Basic32> vertices(vcount);
	for(UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3*tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for(UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
	}

	fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(m_pGraphicsDevice->GetD3D11Device()->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
    HR(m_pGraphicsDevice->GetD3D11Device()->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}

void SkinnedMeshApp::BuildScreenQuadGeometryBuffers()
{
	GeometryGenerator::MeshData quad;

	GeometryGenerator geoGen;
	geoGen.CreateFullscreenQuad(quad);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(quad.Vertices.size());

	for(UINT i = 0; i < quad.Vertices.size(); ++i)
	{
		vertices[i].Pos    = quad.Vertices[i].Position;
		vertices[i].Normal = quad.Vertices[i].Normal;
		vertices[i].Tex    = quad.Vertices[i].TexC;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * quad.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(m_pGraphicsDevice->GetD3D11Device()->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &quad.Indices[0];
    HR(m_pGraphicsDevice->GetD3D11Device()->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}
