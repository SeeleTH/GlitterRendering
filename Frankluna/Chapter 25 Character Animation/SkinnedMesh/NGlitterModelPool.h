#pragma once
#include "../../../NBoxLib/Source/Macro/Macro.h"
#include "../../../NBoxLib/Source/Resource/NAssetProxy.h"
#include "../../../NBoxLib/Source/Resource/NAssetCache.h"
#include "../../../NBoxLib/Source/Resource/NAssetLoader.h"
#include "../../../NBoxLib/Source/Resource/NAssetLoadResult.h"
#include "../../../NBoxLib/Source/Resource/NAssetPool.h"
#include "../../../NBoxLib/Source/Graphics/NGraphics.h"
#include "../../../NBoxLib/Source/Thread/NCriticalSection.h"

class NAssetGatherer;
class NTexture2DProxy;

namespace GlitterModel
{
	struct PosNormalTexTan
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
		DirectX::XMFLOAT4 TangentU;

		PosNormalTexTan()
		{
			Pos.x = Pos.y = Pos.z = 0.f;
			Normal.x = Normal.z = 0.f;
			Normal.y = 1.f;
			Tex.x = Tex.y = 0.f;
			TangentU.x = 1;
			TangentU.y = TangentU.z = 0.f;
		}
	};

	struct Material
	{
		DirectX::XMFLOAT3 m_f3Albedo;
		float m_fRoughness;
		float m_fSpecular;
		float m_fMetallic;
		float m_fCavity;
		float m_fMatMask;
		float m_fFlakesDensity;
		float padding[3];
		NTexture2DProxy* m_sDiffuseMap;
		NTexture2DProxy* m_sNormalMap;
		NTexture2DProxy* m_sRSMCMap;
		NTexture2DProxy* m_sMatMaskMap;
		NTexture2DProxy* m_sFlakesMap;

		Material() 
			: m_f3Albedo()
			, m_fRoughness()
			, m_fSpecular()
			, m_fMetallic()
			, m_fCavity()
			, m_fMatMask()
			, m_fFlakesDensity(0.f)
			, m_sDiffuseMap(NULL)
			, m_sNormalMap(NULL)
			, m_sRSMCMap(NULL)
			, m_sMatMaskMap(NULL)
			, m_sFlakesMap(NULL)
		{
		}
	};

	struct GroupMat
	{
		Material* m_pMaterial;
		NIndexBuffer* m_pIndexBuffer;
		UINT32 m_uIndexCount;
		~GroupMat()
		{
			N_DELETE(m_pIndexBuffer);
		}
	};

	struct Group
	{
		std::map<std::string, GroupMat*> m_vMaterials;
		~Group()
		{
			for (auto it = m_vMaterials.begin(); it != m_vMaterials.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vMaterials.clear();
		}
	};

	struct Object
	{
		std::map<std::string, Group*> m_vGroups;

		~Object()
		{
			for (auto it = m_vGroups.begin(); it != m_vGroups.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vGroups.clear();
		}
	};

	struct World
	{
		std::map<std::string, Material*> m_vMaterials;
		std::map<std::string, Object*> m_vObject;
		NVertexBuffer* m_pVertexBuffer;

		~World()
		{
			for (auto it = m_vMaterials.begin(); it != m_vMaterials.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vMaterials.clear();
			for (auto it = m_vObject.begin(); it != m_vObject.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vObject.clear();
			N_DELETE(m_pVertexBuffer);
		}
	};
};

class NGlitterModelProxy : public INAssetProxy
{
public:

	NGlitterModelProxy(std::string key, UINT32 type = 0) : m_sKey(key), m_u32Type(type), m_pWorld(NULL){}
	virtual ~NGlitterModelProxy() override { N_DELETE(m_pWorld); }
	virtual std::string GetKey() override { return m_sKey; }

	inline void SetWorld(GlitterModel::World* world){ m_pWorld = world; }
	inline GlitterModel::World* GetWorld() { return m_pWorld; }
protected:
	std::string m_sKey;
	UINT32 m_u32Type;

	GlitterModel::World* m_pWorld;
};

class NGlitterModelLoadResult : public INAssetLoadResult
{
public:
	NGlitterModelLoadResult(GlitterModel::World* world) : m_pGlitterModelWorld(world) {}
	~NGlitterModelLoadResult() { }
	inline GlitterModel::World* GetGlitterModelData() { return m_pGlitterModelWorld; }
protected:
	GlitterModel::World* m_pGlitterModelWorld;
};

class NTexture2DPool;

class NGlitterModelLoader : public INAssetLoader
{
public:
	struct triInd
	{
		UINT32 p[3];
	};

	struct Material
	{
		DirectX::XMFLOAT3 m_f3Ka;
		DirectX::XMFLOAT3 m_f3Kd;
		DirectX::XMFLOAT3 m_f3Ks;
		DirectX::XMFLOAT3 m_f3Ke;
		DirectX::XMFLOAT3 m_f3Ns;
		DirectX::XMFLOAT3 m_f3Ni;
		float m_fD;
		UINT32 m_u32Illum;
		std::string m_sMapKa;
		std::string m_sMapKd;
		std::string m_sMapKs;
		std::string m_sMapNs;
		std::string m_sMapD;
		std::string m_sMapBump;
		std::string m_sBump;
		std::string m_sDisp;
		std::string m_sDecal;
		std::string m_sDir;

		// Specialized Parameter
		std::map<std::string, float> m_vCustomFloats;
		std::map<std::string, DirectX::XMFLOAT3> m_vCustomVecs;
		std::map<std::string, std::string> m_vCustomStrings;

		Material() :m_f3Ka(), m_f3Kd(), m_f3Ks(), m_f3Ns(), m_f3Ni(), m_f3Ke(), m_fD(0.f), m_u32Illum(0u), m_sDir("")
		{
		}
	};

	struct GroupIndexes
	{
		std::vector<triInd> m_vVertIndexes;
		std::vector<triInd> m_vNormIndexes;
		std::vector<triInd> m_vTanIndexes;
		std::vector<triInd> m_vTexCIndexes;
	};

	struct Group
	{
		std::map<std::string, GroupIndexes*> m_vMaterials;

		~Group()
		{
			for (auto it = m_vMaterials.begin(); it != m_vMaterials.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vMaterials.clear();
		}
	};

	struct Object
	{
		std::map<std::string, Group*> m_vGroups;

		~Object()
		{
			for (auto it = m_vGroups.begin(); it != m_vGroups.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vGroups.clear();
		}
	};

	struct World
	{
		std::vector<DirectX::XMFLOAT4> m_vVertices;
		std::vector<DirectX::XMFLOAT3> m_vNormals;
		std::vector<DirectX::XMFLOAT4> m_vTangents;
		std::vector<DirectX::XMFLOAT3> m_vTexCoords;
		std::map<std::string, Material*> m_vMaterials;
		std::map<std::string, Object*> m_vObject;

		~World()
		{
			for (auto it = m_vMaterials.begin(); it != m_vMaterials.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vMaterials.clear();
			for (auto it = m_vObject.begin(); it != m_vObject.end(); it++)
			{
				N_DELETE(it->second);
			}
			m_vObject.clear();
		}
	};

	NGlitterModelLoader(NGraphicsDevice* dev, NTexture2DPool* texturePool);
	virtual ~NGlitterModelLoader();
	virtual INAssetLoadResult* LoadProxy_GatherThread(INAssetStreamer* streamer, INAssetProxy* proxy) override;
	virtual void ClearResult(INAssetLoadResult* result) override;

protected:
	GlitterModel::World* createWorldFromRawData(World* rawWorld);
	void consumeOneLine(INAssetStreamer* streamer, const std::string &line, const std::string &dir);
	void loadMaterialLib(INAssetStreamer* streamer, const std::string &file);
	void consumeMatOneLine(const std::string &line, const std::string &dir);
	void generateTangents(World* world);
	void generateMatIndexes(World* world);

	void setObject(const std::string &name);
	void setGroup(const std::string &name);
	void setGroupMat(const std::string &name);

	NGraphicsDevice* m_pDevice;
	NTexture2DPool* m_pTexturePool;

	World* m_pCurWorld;
	Object* m_pCurObject;
	Group* m_pCurGroup;
	GroupIndexes* m_pCurGroupMat;
	std::string m_sCurMTL;
	Material* m_pCurMat;
};

class NGlitterModelPool : public INAssetPool
{
public:
	enum MODEL_TYPE
	{
		MODEL_TYPE_ONE = 0
	};
	NGlitterModelPool(NGraphicsDevice* dev, NAssetGatherer* gatherer, NTexture2DPool* texturePool);
	virtual ~NGlitterModelPool();

	virtual void AddLoadedProxy(INAssetProxy* proxy, INAssetLoadResult* result) override;
	NGlitterModelProxy* GetModel(std::string key, UINT32 type = 0, BOOL async = true, GlitterModel::World* asset = NULL);

protected:
	NAssetGatherer* m_pGatherer;
	NTexture2DPool* m_pTexturePool;

	NGlitterModelLoader* m_pModelLoader;
	NFileAssetCache<std::string, NGlitterModelProxy*>* m_pCache;

	NCriticalSection m_csPoolLock;
};