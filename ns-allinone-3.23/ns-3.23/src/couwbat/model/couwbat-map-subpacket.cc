/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iomanip>
#include <iostream>
#include <string.h>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "couwbat-map-subpacket.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CouwbatMapSubpacket");

NS_OBJECT_ENSURE_REGISTERED (CouwbatMapSubpacket);

CouwbatMapSubpacket::CouwbatMapSubpacket ()
  : m_ofdm_offset (0),
    m_ofdm_count (0)
{
  NS_LOG_FUNCTION (this);
}

void
CouwbatMapSubpacket::SetAmc (const uint8_t mcs[])
{
  std::memset(m_amc, 0, 24);
  for (uint32_t i = 0; i < Couwbat::MAX_SUBCHANS; ++i)
    {

      int first = (mcs[i] & 4) != 0;
      int second = (mcs[i] & 2) != 0;
      int third = (mcs[i] & 1) != 0;

      m_amc[(3 * i) / 8] |= (first << (7 - (3 * i) % 8));
      m_amc[(3 * i + 1) / 8] |= (second << (7 - (3 * i + 1) % 8));
      m_amc[(3 * i + 2) / 8] |= (third << (7 - (3 * i + 2) % 8));
    }
}

void
CouwbatMapSubpacket::GetMcs (uint8_t mcs[]) const
{
  for (uint32_t i = 0; i < Couwbat::MAX_SUBCHANS; ++i)
    {
      // Get bits
      int first = (m_amc[(3 * i) / 8] & (1 << (7 - (3 * i) % 8))) != 0;
      int second = (m_amc[(3 * i + 1) / 8] & (1 << (7 - (3 * i + 1) % 8))) != 0;
      int third = (m_amc[(3 * i + 2) / 8] & (1 << (7 - (3 * i + 2) % 8))) != 0;

      // Set AMC
      mcs[i] = 4 * first + 2 * second + 1 * third;
    }
}

uint32_t 
CouwbatMapSubpacket::GetHeaderSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId 
CouwbatMapSubpacket::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatMapSubpacket")
    .SetParent<Header> ()
    .AddConstructor<CouwbatMapSubpacket> ()
  ;
  return tid;
}

TypeId 
CouwbatMapSubpacket::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
CouwbatMapSubpacket::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

  os << "ie_id=" << m_ie_id
     << ", amc[]=" << "some_amc"
     << ", ofdm_offset=" << m_ofdm_offset
     << ", ofdm_count=" << m_ofdm_count;
}

uint32_t 
CouwbatMapSubpacket::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static uint32_t ret = MAC_ADDR_SIZE + sizeof (m_amc)
      + sizeof(m_ofdm_offset) + sizeof (m_ofdm_count);

  return ret;
}

void
CouwbatMapSubpacket::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  WriteTo (i, m_ie_id);
  for (int j = 0; j < 24; ++j)
    {
      i.WriteU8 (m_amc[j]);
    }
  i.WriteHtonU16 (m_ofdm_offset);
  i.WriteHtonU16 (m_ofdm_count);
}

uint32_t
CouwbatMapSubpacket::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  ReadFrom (i, m_ie_id);
  for (int j = 0; j < 24; ++j)
    {
      m_amc[j] = i.ReadU8 ();
    }
  m_ofdm_offset = i.ReadNtohU16 ();
  m_ofdm_count = i.ReadNtohU16 ();

  return GetSerializedSize ();
}

} // namespace ns3
