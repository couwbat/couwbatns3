/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iomanip>
#include <iostream>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "couwbat-ul-burst-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CouwbatUlBurstHeader");

NS_OBJECT_ENSURE_REGISTERED (CouwbatUlBurstHeader);

CouwbatUlBurstHeader::CouwbatUlBurstHeader ()
    : m_ack (0)
{
  NS_LOG_FUNCTION (this);
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      m_cqi[k] = 0;
    }
}

uint32_t 
CouwbatUlBurstHeader::GetHeaderSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId 
CouwbatUlBurstHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatUlBurstHeader")
    .SetParent<Header> ()
    .AddConstructor<CouwbatUlBurstHeader> ()
  ;
  return tid;
}

TypeId 
CouwbatUlBurstHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
CouwbatUlBurstHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

  os << "ack=" << int(m_ack)
     << ", cqi={" << int(m_cqi[0]);

  for (uint32_t k = 1; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      os << ","<< (int)m_cqi[k];
    }

  os << "}";
}

uint32_t 
CouwbatUlBurstHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static uint32_t ret = sizeof (m_ack) + sizeof (m_cqi);
  return ret;
}

void
CouwbatUlBurstHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  i.WriteU8 (m_ack);
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      i.WriteU8 (m_cqi[k]);
    }
}

uint32_t
CouwbatUlBurstHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  m_ack = i.ReadU8 ();
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      m_cqi[k] = i.ReadU8 ();
    }

  return GetSerializedSize ();
}

} // namespace ns3
