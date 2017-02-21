/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iomanip>
#include <iostream>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "couwbat-pss-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CouwbatPssHeader");

NS_OBJECT_ENSURE_REGISTERED (CouwbatPssHeader);

CouwbatPssHeader::CouwbatPssHeader ()
    //: m_pssFragments (1)
{
  NS_LOG_FUNCTION (this);
}

uint32_t 
CouwbatPssHeader::GetHeaderSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId 
CouwbatPssHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatPssHeader")
    .SetParent<Header> ()
    .AddConstructor<CouwbatPssHeader> ()
  ;
  return tid;
}

TypeId 
CouwbatPssHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
CouwbatPssHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

  os << "m_chSwitchActive=" << m_backupChannel.fields.chSwitchActive << ", "
      << "m_chSwitchCounter=" << m_backupChannel.fields.chSwitchCounter << ", "
      << "m_newChNumber=" << m_backupChannel.fields.newChNumber << ", "
      << "m_pssFragments=" << m_pssMaintain.fields.pssFragments << ", "
      << "m_mapLength=" << m_pssMaintain.fields.mapLength << ", "
      << "m_allocation=" << m_allocation.to_string ();
}

uint32_t 
CouwbatPssHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static const uint32_t ret = sizeof (m_backupChannel) + sizeof (m_pssMaintain) + sizeof (m_allocation);
  return ret;
}

void
CouwbatPssHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_backupChannel.value);
  i.WriteHtonU16 (m_pssMaintain.value);
  //i.WriteHtonU16 (m_pssFragments);
  i.WriteHtonU64 (m_allocation.to_ulong ());
}

uint32_t
CouwbatPssHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  m_backupChannel.value = i.ReadNtohU16 ();
  m_pssMaintain.value = i.ReadNtohU16 ();
  //m_pssFragments = i.ReadNtohU16 ();
  m_allocation = std::bitset<64> (i.ReadNtohU64 ());

  return GetSerializedSize ();
}

} // namespace ns3
