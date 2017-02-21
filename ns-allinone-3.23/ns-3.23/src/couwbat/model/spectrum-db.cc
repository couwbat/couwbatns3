#include "spectrum-db.h"
#include "spectrum-map.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SpectrumDb");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (SpectrumDb);

TypeId
SpectrumDb::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::SpectrumDb")
    .SetParent<Object> ()
    .AddConstructor<SpectrumDb> ()
    .SetGroupName("Couwbat")
  ;
  return tid;
}

SpectrumDb::SpectrumDb (void)
{
  NS_LOG_FUNCTION (this);
  
  // create unoccupied spectrum map
  m_specMap = CreateObject<SpectrumMap> ();
}

SpectrumDb::~SpectrumDb ()
{
  NS_LOG_FUNCTION (this);
}

void
SpectrumDb::OccupySpectrum (Ptr<SpectrumMap> map)
{
  NS_LOG_FUNCTION (this << map);
  // TODO check that the spectrum to occupy is free
  m_specMap->OR(map);
  NS_LOG_INFO ("Occupied Spectrum:    " << m_specMap);
}

void
SpectrumDb::LeaveSpectrum (Ptr<SpectrumMap> map)
{
  NS_LOG_FUNCTION (this << map);
  m_specMap->XOR(map);
  NS_LOG_INFO ("Occupied Spectrum:     " << m_specMap);
}

Ptr<SpectrumMap>
SpectrumDb::GetOccupiedSpectrum (void) const
{
  NS_LOG_FUNCTION (this);
  return CopyObject (m_specMap);
}

} // namespace ns3
