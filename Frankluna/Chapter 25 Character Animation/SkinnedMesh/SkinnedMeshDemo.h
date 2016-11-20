#pragma once

#include "../../../NBoxLib/Source/Macro/Macro.h"
#include "../../../NBoxLib/Source/Template/NSingleton.h"
#include "../../../NBoxLib/Source/Graphics/NGraphics.h"
#include "../../../NBoxLib/Source/Graphics/NDX11Buffer.h"
#include "../../../NBoxLib/Source/Resource/NResCache.h"
#include "../../../NBoxLib/Source/Resource/NResDir.h"
#include "../../../NBoxLib/Source/Util/NTime.h"
#include "../../../NBoxLib/Source/Actors/NActorManager.h"
#include "../../../NBoxLib/Source/Resource/NAssetGatherer.h"
#include "../../../NBoxLib/Source/Graphics/NTexture2DPool.h"
#include "NGlitterModelPool.h"
#include "SkinnedMeshNWin32.h"
#include "SkinnedMeshRenderManager.h"

#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "RenderStates.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include "TextureMgr.h"
#include "BasicModel.h"
#include "SkinnedModel.h"

#define SKIP_USELESS_LOADING

#define CAMERA_SHOWROOM
#define CAMERA_SHOWROOM_TARGET_X 0.f
#define CAMERA_SHOWROOM_TARGET_Y 3.0f
#define CAMERA_SHOWROOM_TARGET_Z 0.f
#define CAMERA_SHOWROOM_FREQ 0.1f
#define CAMERA_SHOWROOM_DIST 10.f
#define CAMERA_SHOWROOM_YAWDEG 20.f
#define CAMERA_SHOWROOM_PROGOFFSET 0.3f

enum COMPONENT_TYPE
{
	BASIC_GEOMETRY_MESH,
	COMPONENT_NO
};

struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

class SkinnedMeshApp : public NSingleton<SkinnedMeshApp> // : public D3DApp
{
public:
	SkinnedMeshApp();
	~SkinnedMeshApp();

	bool Init(HINSTANCE hInstance);
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

	inline NTexture2DPool* GetTexturePool() { return m_pTexturePool; }
	inline NGlitterModelPool* GetGlitterModelPool() { return m_pGlitterModelPool; }
	inline NTime* GetTime() { return m_pTime; }
	inline Camera* GetCurrentCamera() { return &mCam; }
	inline float GetWidth() { return (m_pWin32) ? m_pWin32->GetClientWidth() : 0.f; }
	inline float GetHeight() { return (m_pWin32) ? m_pWin32->GetClientHeight() : 0.f; }
	// =====================
	// From D3DApp Begin
	// =====================
public:
	int Run();

protected:
	void CalculateFrameStats();

	// =====================
	// From D3DApp End
	// =====================

private:
	void DrawSceneToSsaoNormalDepthMap();
	void DrawSceneToShadowMap();
	void DrawScreenQuad(ID3D11ShaderResourceView* srv);
	void BuildShadowTransform();
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();
	void BuildScreenQuadGeometryBuffers();

private:
#ifdef CAMERA_SHOWROOM
	float m_fCameraShowroomProg;
#endif

	// ===================
	// NBoxLib begin
	// ===================

	SkinnedMeshNWin32* m_pWin32;
	NGraphicsDevice* m_pGraphicsDevice;
	SkinnedMeshRenderManager* m_pRenderManager;
	NResCache* m_pResCache;
	NTime* m_pTime;
	NActorManager* m_pActorManager;
	NAssetGatherer* m_pAssetGatherer;
	NTexture2DPool* m_pTexturePool;
	NGlitterModelPool* m_pGlitterModelPool;

	// ===================
	// NBoxLib end
	// ===================

	TextureMgr mTexMgr;

	Sky* mSky;

	SkinnedModel* mCharacterModel;
	SkinnedModelInstance mCharacterInstance1;
	SkinnedModelInstance mCharacterInstance2;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

	ID3D11Buffer* mSkySphereVB;
	ID3D11Buffer* mSkySphereIB;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	ID3D11ShaderResourceView* mStoneTexSRV;
	ID3D11ShaderResourceView* mBrickTexSRV;

	ID3D11ShaderResourceView* mStoneNormalTexSRV;
	ID3D11ShaderResourceView* mBrickNormalTexSRV;

	BoundingSphere mSceneBounds;

	static const int SMapSize = 2048;
	ShadowMap* mSmap;
	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	Ssao* mSsao;

	float mLightRotationAngle;
	XMFLOAT3 mOriginalLightDir[3];
	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mSkullWorld;

	int mBoxVertexOffset;
	int mGridVertexOffset;
	int mSphereVertexOffset;
	int mCylinderVertexOffset;

	UINT mBoxIndexOffset;
	UINT mGridIndexOffset;
	UINT mSphereIndexOffset;
	UINT mCylinderIndexOffset;

	UINT mBoxIndexCount;
	UINT mGridIndexCount;
	UINT mSphereIndexCount;
	UINT mCylinderIndexCount;

	UINT mSkullIndexCount;

	Camera mCam;

	POINT mLastMousePos;
};
