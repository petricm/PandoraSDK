/**
 *    @file PandoraSDK/src/Objects/CaloHit.cc
 * 
 *    @brief Implementation of the calo hit class.
 * 
 *  $Log: $
 */

#include "Objects/CaloHit.h"
#include "Objects/MCParticle.h"

#include <cmath>

namespace pandora
{

CaloHit::CaloHit(const PandoraApi::CaloHit::Parameters &parameters) :
    m_positionVector(parameters.m_positionVector.Get()),
    m_expectedDirection(parameters.m_expectedDirection.Get().GetUnitVector()),
    m_cellNormalVector(parameters.m_cellNormalVector.Get().GetUnitVector()),
    m_cellGeometry(parameters.m_cellGeometry.Get()),
    m_cellSize0(parameters.m_cellSize0.Get()),
    m_cellSize1(parameters.m_cellSize1.Get()),
    m_cellThickness(parameters.m_cellThickness.Get()),
    m_nCellRadiationLengths(parameters.m_nCellRadiationLengths.Get()),
    m_nCellInteractionLengths(parameters.m_nCellInteractionLengths.Get()),
    m_time(parameters.m_time.Get()),
    m_inputEnergy(parameters.m_inputEnergy.Get()),
    m_mipEquivalentEnergy(parameters.m_mipEquivalentEnergy.Get()),
    m_electromagneticEnergy(parameters.m_electromagneticEnergy.Get()),
    m_hadronicEnergy(parameters.m_hadronicEnergy.Get()),
    m_isDigital(parameters.m_isDigital.Get()),
    m_hitType(parameters.m_hitType.Get()),
    m_hitRegion(parameters.m_hitRegion.Get()),
    m_layer(parameters.m_layer.Get()),
    m_isInOuterSamplingLayer(parameters.m_isInOuterSamplingLayer.Get()),
    m_cellLengthScale(0.f),
    m_isPossibleMip(false),
    m_isIsolated(false),
    m_isAvailable(true),
    m_weight(1.f),
    m_pParentAddress(parameters.m_pParentAddress.Get())
{
    m_cellLengthScale = this->CalculateCellLengthScale();
}

//------------------------------------------------------------------------------------------------------------------------------------------

CaloHit::CaloHit(const PandoraContentApi::CaloHitFragment::Parameters &parameters) :
    m_positionVector(parameters.m_pOriginalCaloHit->m_positionVector),
    m_expectedDirection(parameters.m_pOriginalCaloHit->m_expectedDirection),
    m_cellNormalVector(parameters.m_pOriginalCaloHit->m_cellNormalVector),
    m_cellGeometry(parameters.m_pOriginalCaloHit->m_cellGeometry),
    m_cellSize0(parameters.m_pOriginalCaloHit->m_cellSize0),
    m_cellSize1(parameters.m_pOriginalCaloHit->m_cellSize1),
    m_cellThickness(parameters.m_pOriginalCaloHit->m_cellThickness),
    m_nCellRadiationLengths(parameters.m_pOriginalCaloHit->m_nCellRadiationLengths),
    m_nCellInteractionLengths(parameters.m_pOriginalCaloHit->m_nCellInteractionLengths),
    m_time(parameters.m_pOriginalCaloHit->m_time),
    m_inputEnergy(parameters.m_weight.Get() * parameters.m_pOriginalCaloHit->m_inputEnergy),
    m_mipEquivalentEnergy(parameters.m_weight.Get() * parameters.m_pOriginalCaloHit->m_mipEquivalentEnergy),
    m_electromagneticEnergy(parameters.m_weight.Get() * parameters.m_pOriginalCaloHit->m_electromagneticEnergy),
    m_hadronicEnergy(parameters.m_weight.Get() * parameters.m_pOriginalCaloHit->m_hadronicEnergy),
    m_isDigital(parameters.m_pOriginalCaloHit->m_isDigital),
    m_hitType(parameters.m_pOriginalCaloHit->m_hitType),
    m_hitRegion(parameters.m_pOriginalCaloHit->m_hitRegion),
    m_layer(parameters.m_pOriginalCaloHit->m_layer),
    m_pseudoLayer(parameters.m_pOriginalCaloHit->m_pseudoLayer),
    m_isInOuterSamplingLayer(parameters.m_pOriginalCaloHit->m_isInOuterSamplingLayer),
    m_cellLengthScale(parameters.m_pOriginalCaloHit->m_cellLengthScale),
    m_isPossibleMip(parameters.m_pOriginalCaloHit->m_isPossibleMip),
    m_isIsolated(parameters.m_pOriginalCaloHit->m_isIsolated),
    m_isAvailable(parameters.m_pOriginalCaloHit->m_isAvailable),
    m_weight(parameters.m_weight.Get() * parameters.m_pOriginalCaloHit->m_weight),
    m_mcParticleWeightMap(parameters.m_pOriginalCaloHit->m_mcParticleWeightMap),
    m_pParentAddress(parameters.m_pOriginalCaloHit->m_pParentAddress)
{
    for (MCParticleWeightMap::iterator iter = m_mcParticleWeightMap.begin(), iterEnd = m_mcParticleWeightMap.end(); iter != iterEnd; ++iter)
        iter->second = iter->second * parameters.m_weight.Get();
}

//------------------------------------------------------------------------------------------------------------------------------------------

CaloHit::~CaloHit()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode CaloHit::SetPseudoLayer(const unsigned int pseudoLayer)
{
    if (!(m_pseudoLayer = pseudoLayer))
        return STATUS_CODE_NOT_INITIALIZED;

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode CaloHit::AlterMetadata(const PandoraContentApi::CaloHit::Metadata &metadata)
{
    if (metadata.m_isPossibleMip.IsInitialized())
        m_isPossibleMip = metadata.m_isPossibleMip.Get();

    if (metadata.m_isIsolated.IsInitialized())
        m_isIsolated = metadata.m_isIsolated.Get();

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CaloHit::SetMCParticleWeightMap(const MCParticleWeightMap &mcParticleWeightMap)
{
    m_mcParticleWeightMap = mcParticleWeightMap;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CaloHit::RemoveMCParticles()
{
    m_mcParticleWeightMap.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------

float CaloHit::CalculateCellLengthScale() const
{
    if (RECTANGULAR == this->GetCellGeometry())
    {
        return std::sqrt(this->GetCellSize0() * this->GetCellSize1());
    }
    else if (POINTING == this->GetCellGeometry())
    {
        float radius(0.f), phi(0.f), theta(0.f);
        this->GetPositionVector().GetSphericalCoordinates(radius, phi, theta);
        const float centralEta(-1.f * std::log(std::tan(theta / 2.f)));

        const float etaMin(centralEta - this->GetCellSize0() / 2.f), etaMax(centralEta + this->GetCellSize0() / 2.f);
        const float thetaMin(2.f * std::atan(std::exp(-1.f * etaMin))), thetaMax(2.f * std::atan(std::exp(-1.f * etaMax)));

        return std::sqrt(std::fabs(radius * this->GetCellSize1() * radius * (thetaMax - thetaMin)));
    }
    else
    {
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CaloHit::GetCellCorners(CartesianPointList &cartesianPointList) const
{
    if (RECTANGULAR == this->GetCellGeometry())
    {
        this->GetRectangularCellCorners(cartesianPointList);
    }
    else if (POINTING == this->GetCellGeometry())
    {
        this->GetPointingCellCorners(cartesianPointList);
    }
    else
    {
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CaloHit::GetRectangularCellCorners(CartesianPointList &cartesianPointList) const
{
    const CartesianVector &position(this->GetPositionVector());

    CartesianVector normal(this->GetCellNormalVector());
    CartesianVector dirU((BARREL == this->GetHitRegion()) ? CartesianVector(0.f, 0.f, 1.f) : CartesianVector(0.f, 1.f, 0.f) );
    CartesianVector dirV(normal.GetCrossProduct(dirU));

    dirU *= (this->GetCellSize0() / 2.f);
    dirV *= (this->GetCellSize1() / 2.f);
    normal *= (this->GetCellThickness() / 2.f);

    cartesianPointList.push_back(CartesianVector(position - dirU - dirV - normal));
    cartesianPointList.push_back(CartesianVector(position + dirU - dirV - normal));
    cartesianPointList.push_back(CartesianVector(position + dirU + dirV - normal));
    cartesianPointList.push_back(CartesianVector(position - dirU + dirV - normal));

    cartesianPointList.push_back(CartesianVector(position - dirU - dirV + normal));
    cartesianPointList.push_back(CartesianVector(position + dirU - dirV + normal));
    cartesianPointList.push_back(CartesianVector(position + dirU + dirV + normal));
    cartesianPointList.push_back(CartesianVector(position - dirU + dirV + normal));
}

//------------------------------------------------------------------------------------------------------------------------------------------

void CaloHit::GetPointingCellCorners(CartesianPointList &cartesianPointList) const
{
    float radius(0.f), phi(0.f), theta(0.f);
    this->GetPositionVector().GetSphericalCoordinates(radius, phi, theta);
    const float centralEta(-1.f * std::log(std::tan(theta / 2.f)));

    const float rMin(radius - this->GetCellThickness() / 2.f), rMax(radius + this->GetCellThickness() / 2.f);
    const float phiMin(phi - this->GetCellSize1() / 2.f), phiMax(phi + this->GetCellSize1() / 2.f);
    const float etaMin(centralEta - this->GetCellSize0() / 2.f), etaMax(centralEta + this->GetCellSize0() / 2.f);
    const float thetaMin(2.f * std::atan(std::exp(-1.f * etaMin))), thetaMax(2.f * std::atan(std::exp(-1.f * etaMax)));

    const float sinTheta(std::sin(theta)), cosTheta(std::cos(theta));
    const float sinThetaMin(std::sin(thetaMin)), cosThetaMin(std::cos(thetaMin)), sinPhiMin(std::sin(phiMin)), cosPhiMin(std::cos(phiMin));
    const float sinThetaMax(std::sin(thetaMax)), cosThetaMax(std::cos(thetaMax)), sinPhiMax(std::sin(phiMax)), cosPhiMax(std::cos(phiMax));

    float thetaMinRScale(1.f), thetaMaxRScale(1.f);

    if (BARREL == this->GetHitRegion())
    {
        if (std::fabs(sinThetaMin) > std::numeric_limits<float>::epsilon())
            thetaMinRScale = std::fabs(sinTheta / sinThetaMin);

        if (std::fabs(sinThetaMax) > std::numeric_limits<float>::epsilon())
            thetaMaxRScale = std::fabs(sinTheta / sinThetaMax);
    }
    else
    {
        if (std::fabs(cosThetaMin) > std::numeric_limits<float>::epsilon())
            thetaMinRScale = std::fabs(cosTheta / cosThetaMin);

        if (std::fabs(cosThetaMax) > std::numeric_limits<float>::epsilon())
            thetaMaxRScale = std::fabs(cosTheta / cosThetaMax);
    }

    const float rMinAtThetaMin(thetaMinRScale * rMin), rMinAtThetaMax(thetaMaxRScale * rMin);
    const float rMaxAtThetaMin(thetaMinRScale * rMax), rMaxAtThetaMax(thetaMaxRScale * rMax);

    cartesianPointList.push_back(CartesianVector(rMinAtThetaMin * sinThetaMin * cosPhiMin, rMinAtThetaMin * sinThetaMin * sinPhiMin, rMinAtThetaMin * cosThetaMin));
    cartesianPointList.push_back(CartesianVector(rMinAtThetaMax * sinThetaMax * cosPhiMin, rMinAtThetaMax * sinThetaMax * sinPhiMin, rMinAtThetaMax * cosThetaMax));
    cartesianPointList.push_back(CartesianVector(rMinAtThetaMax * sinThetaMax * cosPhiMax, rMinAtThetaMax * sinThetaMax * sinPhiMax, rMinAtThetaMax * cosThetaMax));
    cartesianPointList.push_back(CartesianVector(rMinAtThetaMin * sinThetaMin * cosPhiMax, rMinAtThetaMin * sinThetaMin * sinPhiMax, rMinAtThetaMin * cosThetaMin));

    cartesianPointList.push_back(CartesianVector(rMaxAtThetaMin * sinThetaMin * cosPhiMin, rMaxAtThetaMin * sinThetaMin * sinPhiMin, rMaxAtThetaMin * cosThetaMin));
    cartesianPointList.push_back(CartesianVector(rMaxAtThetaMax * sinThetaMax * cosPhiMin, rMaxAtThetaMax * sinThetaMax * sinPhiMin, rMaxAtThetaMax * cosThetaMax));
    cartesianPointList.push_back(CartesianVector(rMaxAtThetaMax * sinThetaMax * cosPhiMax, rMaxAtThetaMax * sinThetaMax * sinPhiMax, rMaxAtThetaMax * cosThetaMax));
    cartesianPointList.push_back(CartesianVector(rMaxAtThetaMin * sinThetaMin * cosPhiMax, rMaxAtThetaMin * sinThetaMin * sinPhiMax, rMaxAtThetaMin * cosThetaMin));
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

std::ostream &operator<<(std::ostream &stream, const CaloHit &caloHit)
{
    stream  << " CaloHit: " << std::endl
            << " position " << caloHit.GetPositionVector()
            << " energy   " << caloHit.GetInputEnergy() << std::endl;

    return stream;
}

} // namespace pandora
