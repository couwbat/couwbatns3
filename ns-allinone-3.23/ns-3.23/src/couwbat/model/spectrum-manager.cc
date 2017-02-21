#include "spectrum-manager.h"
#include "spectrum-db.h"
#include "spectrum-map.h"
#include "couwbat.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SpectrumManager");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (SpectrumManager);

TypeId
SpectrumManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SpectrumManager")
    .SetParent<Object> ()
    .SetGroupName("Couwbat")
    .AddConstructor<SpectrumManager> ()
  ;
  return tid;
}

SpectrumManager::SpectrumManager ()
{
  NS_LOG_FUNCTION (this);
}

SpectrumManager::~SpectrumManager ()
{
  NS_LOG_FUNCTION (this);
}

void
SpectrumManager::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
}

void
SpectrumManager::SetSpectrumDb (Ptr<Object> specDb)
{
  NS_LOG_FUNCTION (this << specDb);
  m_specDb = specDb;
}

bool
SpectrumManager::IsCcFree (uint32_t ccId)
{
  NS_LOG_FUNCTION (this << ccId);

  // get the spectrum map from the database
  Ptr<SpectrumMap> specMap;
  Ptr<SpectrumDb> specDb = m_specDb->GetObject<SpectrumDb> ();
  if (specDb)
    {
      specMap = specDb->GetOccupiedSpectrum ();
    }

  // calculate first subcarrier of the requested CC
  //uint32_t ccFirstScId = (Couwbat::GetNumberOfGuardSubcarriersPerSubchannel () / 2) + (ccId * Couwbat::GetNumberOfSubcarriersPerSubchannel ());
  const uint32_t ccFirstScId = ccId * Couwbat::GetNumberOfSubcarriersPerSubchannel ();

  NS_LOG_INFO ("checking CC=" << ccId << " SC("<<ccFirstScId<<" to "<<ccFirstScId+Couwbat::GetNumberOfSubcarriersPerSubchannel()-1<<")" << " against database map=" << specMap);
  // for each data subcarrier check if set in spectrum map
  // TODO do we only need to check data subcarrier or also guard subcarriers?
  for (uint32_t i=ccFirstScId;
      i < (ccFirstScId + Couwbat::GetNumberOfSubcarriersPerSubchannel ()); ++i)
    {
      if (specMap->at (i))
        {
          return false;
        }
    }
  return true;
}

} // namespace ns3
