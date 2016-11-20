#include "NOrientation.h"

NOrientation::NOrientation()
    : m_vPos(0.f,0.f,0.f)
    , m_vScale(1.f, 1.f, 1.f)
    , m_vRight(1.f, 0.f, 0.f)
    , m_vUp(0.f, 1.f, 0.f)
    , m_vForward(0.f, 0.f, 1.f)
    , m_u32Dirty(0xFFFFFFFF)
{

}

void NOrientation::SetRot(const DirectX::XMFLOAT4X4 &mMatrix)
{
    m_vRight.x = mMatrix(0, 0);
    m_vRight.y = mMatrix(0, 1);
    m_vRight.z = mMatrix(0, 2);

    m_vUp.x = mMatrix(1, 0);
    m_vUp.y = mMatrix(1, 1);
    m_vUp.z = mMatrix(1, 2);

    m_vForward.x = mMatrix(2, 0);
    m_vForward.y = mMatrix(2, 1);
    m_vForward.z = mMatrix(2, 2);

    m_u32Dirty |= DF_ROT | DF_NORMALIZE;
}

void NOrientation::SetRot(const DirectX::FXMVECTOR vForward, const DirectX::FXMVECTOR vUp)
{
    DirectX::FXMVECTOR vRight = DirectX::XMVector3Cross(vUp, vForward);
    DirectX::FXMVECTOR vUp2 = DirectX::XMVector3Cross(vForward, vRight);

    DirectX::XMStoreFloat3(&m_vForward, vForward);
    DirectX::XMStoreFloat3(&m_vUp, vUp2);
    DirectX::XMStoreFloat3(&m_vRight, vRight);

    m_u32Dirty |= DF_ROT | DF_NORMALIZE;
}

void NOrientation::AddRot(const DirectX::XMFLOAT4X4 &mMatrix)
{
    DirectX::XMVECTOR col0 = DirectX::XMVectorSet(mMatrix(0, 0), mMatrix(1, 0), mMatrix(2, 0), 0.0f);
    DirectX::XMVECTOR col1 = DirectX::XMVectorSet(mMatrix(0, 1), mMatrix(1, 1), mMatrix(2, 1), 0.0f);
    DirectX::XMVECTOR col2 = DirectX::XMVectorSet(mMatrix(0, 2), mMatrix(1, 2), mMatrix(2, 2), 0.0f);
    DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&m_vRight);
    DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&m_vUp);
    DirectX::XMVECTOR F = DirectX::XMLoadFloat3(&m_vForward);

    m_vRight.x = DirectX::XMVectorGetX(DirectX::XMVector3Dot(R, col0));
    m_vRight.y = DirectX::XMVectorGetX(DirectX::XMVector3Dot(R, col1));
    m_vRight.z = DirectX::XMVectorGetX(DirectX::XMVector3Dot(R, col2));

    m_vUp.x = DirectX::XMVectorGetX(DirectX::XMVector3Dot(U, col0));
    m_vUp.y = DirectX::XMVectorGetX(DirectX::XMVector3Dot(U, col1));
    m_vUp.z = DirectX::XMVectorGetX(DirectX::XMVector3Dot(U, col2));

    m_vForward.x = DirectX::XMVectorGetX(DirectX::XMVector3Dot(F, col0));
    m_vForward.y = DirectX::XMVectorGetX(DirectX::XMVector3Dot(F, col1));
    m_vForward.z = DirectX::XMVectorGetX(DirectX::XMVector3Dot(F, col2));

    m_u32Dirty |= DF_ROT | DF_NORMALIZE;
}

void NOrientation::RotateAxis(const DirectX::FXMVECTOR vAxis, float fRadians)
{

    m_u32Dirty |= DF_ROT | DF_NORMALIZE;
}

void NOrientation::UpdateMatrix(DirectX::XMFLOAT4X4 &mMatrix) const
{
    if (isDirty()){
        RotAndNormalize();
        CreateMatrix(mMatrix);
        m_u32Dirty = 0UL;
    }
}

void NOrientation::CreateMatrix(DirectX::XMFLOAT4X4 &mMatrix) const
{
    mMatrix(0, 0) = m_vRight.x * m_vScale.x;
    mMatrix(0, 1) = m_vRight.y * m_vScale.x;
    mMatrix(0, 2) = m_vRight.z * m_vScale.x;
    mMatrix(0, 3) = 0.f;

    mMatrix(1, 0) = m_vUp.x * m_vScale.y;
    mMatrix(1, 1) = m_vUp.y * m_vScale.y;
    mMatrix(1, 2) = m_vUp.z * m_vScale.y;
    mMatrix(1, 3) = 0.f;

    mMatrix(2, 0) = m_vForward.x * m_vScale.z;
    mMatrix(2, 1) = m_vForward.y * m_vScale.z;
    mMatrix(2, 2) = m_vForward.z * m_vScale.z;
    mMatrix(2, 3) = 0.f;

    mMatrix(3, 0) = m_vPos.x;
    mMatrix(3, 1) = m_vPos.y;
    mMatrix(3, 2) = m_vPos.z;
    mMatrix(3, 3) = 1.f;
}

void NOrientation::RotAndNormalize() const
{
    if ((m_u32Dirty & DF_NORMALIZE) == 0)
        return;

    DirectX::XMVECTOR F, U, R;
    F = DirectX::XMLoadFloat3(&m_vForward);
    U = DirectX::XMLoadFloat3(&m_vUp);
    R = DirectX::XMLoadFloat3(&m_vRight);

    F = DirectX::XMVector3Normalize(F);
    U = DirectX::XMVector3Normalize(U);
    R = DirectX::XMVector3Normalize(R);

    DirectX::XMStoreFloat3(&m_vForward, F);
    DirectX::XMStoreFloat3(&m_vUp, U);
    DirectX::XMStoreFloat3(&m_vRight, R);

    m_u32Dirty = m_u32Dirty & ~DF_NORMALIZE;
}