/**
 *  @file   PandoraSDK/src/Managers/GeometryManager.cc
 * 
 *  @brief  Implementation of the geometry manager class.
 * 
 *  $Log: $
 */

#include "Managers/GeometryManager.h"

#include "Objects/DetectorGap.h"
#include "Objects/SubDetector.h"

#include "Pandora/ObjectFactory.h"

namespace pandora
{

GeometryManager::GeometryManager(const Pandora *const pPandora) :
    m_hitTypeToGranularityMap(this->GetDefaultHitTypeToGranularityMap()),
    m_pPandora(pPandora)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

GeometryManager::~GeometryManager()
{
    (void) this->EraseAllContent();
}

//------------------------------------------------------------------------------------------------------------------------------------------

const SubDetector &GeometryManager::GetSubDetector(const std::string &subDetectorName) const
{
    SubDetectorMap::const_iterator iter = m_subDetectorMap.find(subDetectorName);

    if (m_subDetectorMap.end() == iter)
        throw StatusCodeException(STATUS_CODE_NOT_FOUND);

    return *(iter->second);
}

//------------------------------------------------------------------------------------------------------------------------------------------

const SubDetector &GeometryManager::GetSubDetector(const SubDetectorType subDetectorType) const
{
    SubDetectorTypeMap::const_iterator iter = m_subDetectorTypeMap.find(subDetectorType);

    if (m_subDetectorTypeMap.end() == iter)
        throw StatusCodeException(STATUS_CODE_NOT_FOUND);

    if (m_subDetectorTypeMap.count(subDetectorType) != 1)
        throw StatusCodeException(STATUS_CODE_OUT_OF_RANGE);

    return *(iter->second);
}

//------------------------------------------------------------------------------------------------------------------------------------------

Granularity GeometryManager::GetHitTypeGranularity(const HitType hitType) const
{
    HitTypeToGranularityMap::const_iterator iter = m_hitTypeToGranularityMap.find(hitType);

    if (m_hitTypeToGranularityMap.end() != iter)
        return iter->second;

    std::cout << "GeometryManager: specified hitType must be registered with a specific granularity. See PandoraApi.h " << std::endl;
    throw StatusCodeException(STATUS_CODE_NOT_FOUND);
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode GeometryManager::CreateSubDetector(const PandoraApi::Geometry::SubDetector::Parameters &inputParameters,
    const ObjectFactory<PandoraApi::Geometry::SubDetector::Parameters, SubDetector> &factory)
{
    const SubDetector *pSubDetector = NULL;

    try
    {
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, factory.Create(inputParameters, pSubDetector));

        if (!m_subDetectorMap.insert(SubDetectorMap::value_type(pSubDetector->GetSubDetectorName(), pSubDetector)).second)
            throw StatusCodeException(STATUS_CODE_FAILURE);

        m_subDetectorTypeMap.insert(SubDetectorTypeMap::value_type(pSubDetector->GetSubDetectorType(), pSubDetector));
    }
    catch (StatusCodeException &statusCodeException)
    {
        std::cout << "Failed to create sub detector: " << statusCodeException.ToString() << std::endl;
        delete pSubDetector;
        pSubDetector = NULL;
        return statusCodeException.GetStatusCode();
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

template <typename PARAMETERS, typename OBJECT>
StatusCode GeometryManager::CreateGap(const PARAMETERS &parameters, const ObjectFactory<PARAMETERS, OBJECT> &factory)
{
    const OBJECT *pDetectorGap = NULL;

    try
    {
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, factory.Create(parameters, pDetectorGap));

        if (NULL == pDetectorGap)
            return STATUS_CODE_FAILURE;

        m_detectorGapList.insert(pDetectorGap);
        return STATUS_CODE_SUCCESS;
    }
    catch (StatusCodeException &statusCodeException)
    {
        std::cout << "Failed to create gap: " << statusCodeException.ToString() << std::endl;
        delete pDetectorGap;
        pDetectorGap = NULL;
        return statusCodeException.GetStatusCode();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode GeometryManager::EraseAllContent()
{
    for (SubDetectorMap::const_iterator iter = m_subDetectorMap.begin(), iterEnd = m_subDetectorMap.end(); iter != iterEnd; ++iter)
        delete iter->second;

    for (DetectorGapList::const_iterator iter = m_detectorGapList.begin(), iterEnd = m_detectorGapList.end(); iter != iterEnd; ++iter)
        delete *iter;

    m_subDetectorMap.clear();
    m_subDetectorTypeMap.clear();
    m_detectorGapList.clear();
    m_hitTypeToGranularityMap.clear();

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

GeometryManager::HitTypeToGranularityMap GeometryManager::GetDefaultHitTypeToGranularityMap() const
{
    HitTypeToGranularityMap hitTypeToGranularityMap;

    if (!hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(TRACKER, VERY_FINE)).second ||
        !hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(ECAL, FINE)).second ||
        !hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(HCAL, COARSE)).second ||
        !hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(MUON, VERY_COARSE)).second ||
        !hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(TPC_VIEW_U, VERY_FINE)).second ||
        !hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(TPC_VIEW_V, VERY_FINE)).second ||
        !hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(TPC_VIEW_W, VERY_FINE)).second ||
        !hitTypeToGranularityMap.insert(HitTypeToGranularityMap::value_type(TPC_3D, VERY_FINE)).second)
    {
        throw StatusCodeException(STATUS_CODE_FAILURE);
    }

    return hitTypeToGranularityMap;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode GeometryManager::SetHitTypeGranularity(const HitType hitType, const Granularity granularity)
{
    HitTypeToGranularityMap::iterator iter = m_hitTypeToGranularityMap.find(hitType);

    if (m_hitTypeToGranularityMap.end() != iter)
    {
        iter->second = granularity;
    }
    else
    {
        m_hitTypeToGranularityMap[hitType] = granularity;
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

template StatusCode GeometryManager::CreateGap(const PandoraApi::Geometry::LineGap::Parameters &, const ObjectFactory<PandoraApi::Geometry::LineGap::Parameters, LineGap> &);
template StatusCode GeometryManager::CreateGap(const PandoraApi::Geometry::BoxGap::Parameters &, const ObjectFactory<PandoraApi::Geometry::BoxGap::Parameters, BoxGap> &);
template StatusCode GeometryManager::CreateGap(const PandoraApi::Geometry::ConcentricGap::Parameters &, const ObjectFactory<PandoraApi::Geometry::ConcentricGap::Parameters, ConcentricGap> &);

} // namespace pandora
