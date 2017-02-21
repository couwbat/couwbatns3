#include "spectrum-map.h"
#include "couwbat.h"
#include "ns3/log.h"
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("SpectrumMap");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED (SpectrumMap);

TypeId
SpectrumMap::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::SpectrumMap")
    .SetParent<Object> ()
    .AddConstructor<SpectrumMap> ()
    .SetGroupName("Couwbat")
  ;
  return tid;
}

SpectrumMap::SpectrumMap (void)
{
  NS_LOG_FUNCTION (this);
  m_map.resize (Couwbat::GetNumberOfSubcarriers (),false);
}

SpectrumMap::~SpectrumMap ()
{
  NS_LOG_FUNCTION (this);
}

void
SpectrumMap::SetSpectrum (uint32_t start_subcarrier, uint32_t nr_subcarriers)
{
  NS_LOG_FUNCTION (start_subcarrier << nr_subcarriers);
  uint32_t i;
  for (i=0; i < nr_subcarriers; i++)
    {
      m_map.at(start_subcarrier +i) = true;
    }
}

bool
SpectrumMap::at (uint32_t pos)
{
  return m_map.at (pos);
}

void
SpectrumMap::OR (Ptr<SpectrumMap> otherMap)
{
  NS_LOG_FUNCTION (this);
  //m_map |= otherMap->m_map;
  uint32_t i;
  for (i=0; i<m_map.size (); i++)
    {
      m_map.at(i) = m_map.at(i) || otherMap->m_map.at(i);
    }
}

void
SpectrumMap::XOR (Ptr<SpectrumMap> otherMap)
{
  NS_LOG_FUNCTION (this);
  //m_map ^= otherMap->m_map;
  uint32_t i;
  for (i=0; i<m_map.size (); i++)
    {
      m_map.at(i) = m_map.at(i) ^ otherMap->m_map.at(i);
    }
}

ATTRIBUTE_HELPER_CPP (SpectrumMap);

std::ostream &
operator << (std::ostream &os, const SpectrumMap &map)
{
  uint32_t i;
  for (i=0; i<map.m_map.size (); i++)
    {
      os << map.m_map.at(i);
    }
  return os;
}

std::ostream &
operator << (std::ostream &os, const Ptr<SpectrumMap> &map)
{
  uint32_t i;
  for (i=0; i<map->m_map.size (); i++)
    {
      os << map->m_map.at(i);
    }
  return os;
}

std::istream &
operator >> (std::istream &is, SpectrumMap &map)
{
  // TODO implement stream to SpectrumMap, example code above from rectangle
/*
  is >> rectangle.xMin >> c1 >> rectangle.xMax >> c2 >> rectangle.yMin >> c3 >> rectangle.yMax;
  if (c1 != '|' ||
      c2 != '|' ||
      c3 != '|')
    {
      is.setstate (std::ios_base::failbit);
    }
*/
  return is;
}

} // namespace ns3

