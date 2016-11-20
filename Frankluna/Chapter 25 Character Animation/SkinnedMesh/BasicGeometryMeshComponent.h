#include "../../../NBoxLib/Source/Actors/NActor.h"
#include "../../../NBoxLib/Source/Resource/NResCache.h"
#include "../../../NBoxLib/Source/Graphics/NGraphics.h"
//#include "SkinnedMeshDemo.h"

class BasicGeometryMeshComponent : public NActorComponent//, public NRenderableObject
{
public:
	enum GEOMETRY_TYPE {
		GEO_TYPE_INVALID,
		GEO_TYPE_BOX,
		GEO_TYPE_SPHERE,
		GEO_TYPE_GEOSPHERE,
		GEO_TYPE_CYLINDER,
		GEO_TYPE_GRID
	};

	struct GEOMETRY_DATA {
		GEOMETRY_TYPE m_eType;
		float m_fWidth;
		float m_fHeight;
		float m_fDepth;
		float m_radius;
		UINT32 m_u32SliceCount;
		UINT32 m_u32StackCount;
		UINT32 m_u32NumSubDivisions;
		float m_fBottomRadius;
		float m_fTopRadius;
		UINT32 m_u32M;
		UINT32 m_u32N;
	};

	BasicGeometryMeshComponent();
	virtual ~BasicGeometryMeshComponent(void);

	virtual bool VOnDataInit(NActorElement* pData);
	virtual void VOnInit(void);
	virtual void VOnUpdate(float deltaMs);
	virtual void VOnDestroy(void);

	static const char *g_sName;
	inline virtual const char* VGetName() const { return g_sName; }

protected:
	std::shared_ptr<NResHandle> m_pColorTex;
	std::shared_ptr<NResHandle> m_pNormalTex;
	NVertexBuffer* m_pVB;
	NIndexBuffer* m_pIB;

	GEOMETRY_DATA m_sGeoData;
};