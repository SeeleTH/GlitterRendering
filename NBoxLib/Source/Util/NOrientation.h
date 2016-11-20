#pragma once

#include <DirectXMath.h>
//#include <DirectXPackedVector.h>

#include "../Macro/Macro.h"


class NOrientation
{
public:
    NOrientation();

    inline DirectX::XMFLOAT3& Pos()
    {
        m_u32Dirty |= DF_POS;
        return m_vPos;
    }

    inline const DirectX::XMFLOAT3& Pos() const
    {
        return m_vPos;
    }

    inline DirectX::XMFLOAT3 & Scale()
    {
        m_u32Dirty |= DF_SCALE;
        return m_vScale;
    }

    inline const DirectX::XMFLOAT3 & Scale() const
    {
        return m_vScale;
    }

    inline BOOL isDirty() const
    {
        return m_u32Dirty != 0;
    }

    void SetRot(const DirectX::XMFLOAT4X4 &mMatrix);
    void SetRot(const DirectX::FXMVECTOR vForward, const DirectX::FXMVECTOR vUp);

    void AddRot(const DirectX::XMFLOAT4X4 &mMatrix);
    void RotateAxis(const DirectX::FXMVECTOR vAxis, float fRadians);

    void UpdateMatrix(DirectX::XMFLOAT4X4 &mMatrix) const;

protected:
    void CreateMatrix(DirectX::XMFLOAT4X4 &mMatrix) const;
    void RotAndNormalize() const;

    enum DIRTY_FLAGS {
        DF_POS          = (1 << 0),
        DF_SCALE        = (1 << 1),
        DF_ROT          = (1 << 2),
        DF_NORMALIZE    = (1 << 3),
    };

    DirectX::XMFLOAT3 m_vPos;
    DirectX::XMFLOAT3 m_vScale;

    mutable DirectX::XMFLOAT3 m_vRight;
    mutable DirectX::XMFLOAT3 m_vUp;
    mutable DirectX::XMFLOAT3 m_vForward;

    mutable UINT32   m_u32Dirty;
};