/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iomanip>
#include <iostream>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "couwbat-meta-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CouwbatMetaHeader");

NS_OBJECT_ENSURE_REGISTERED (CouwbatMetaHeader);

CouwbatMetaHeader::CouwbatMetaHeader ()
  : m_flags (0),
    m_frequency_band (0),
    m_ofdm_sym_sframe_count (0),
    m_ofdm_sym_offset (0),
    m_ofdm_sym_len (0),
    m_reserved(0)
{
  NS_LOG_FUNCTION (this);
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      m_MCS[k] = 0;
      m_CQI[k] = 0;
    }
}


uint32_t
CouwbatMetaHeader::GetHeaderSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId
CouwbatMetaHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatMetaHeader")
    .SetParent<Header> ()
    .AddConstructor<CouwbatMetaHeader> ()
  ;
  return tid;
}

TypeId
CouwbatMetaHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CouwbatMetaHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

  os << "m_flags=" << m_flags
     << ", m_frequency_band=" << m_frequency_band
     << ", m_ofdm_sym_sframe_count=" << m_ofdm_sym_sframe_count
     << ", m_ofdm_sym_offset=" << m_ofdm_sym_offset
     << ", m_ofdm_sym_len=" << m_ofdm_sym_len
     << ", m_allocatedSubChannels=" << m_allocatedSubChannels.to_string ()
     << ", MCS={" << (int)m_MCS[0];

  for (uint32_t k = 1; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      os << ","<< (int)m_MCS[k];
    }

   os << "}, CQI={" << (int)m_CQI[0];

  for (uint32_t k = 1; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      os << ","<< (int)m_CQI[k];
    }

   os << "}, m_reserved=" << m_reserved;
}

uint32_t
CouwbatMetaHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static uint32_t ret = sizeof (m_flags) + sizeof (m_frequency_band)
      + sizeof (m_ofdm_sym_sframe_count) + sizeof (m_ofdm_sym_offset)
      + sizeof (m_ofdm_sym_len) + sizeof (m_allocatedSubChannels)
      + sizeof (m_MCS) + sizeof (m_CQI) + sizeof (m_reserved);

  return ret;
}

void
CouwbatMetaHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  i.WriteHtonU32 (m_flags);
  i.WriteHtonU16 (m_frequency_band);
  i.WriteHtonU32 (m_ofdm_sym_sframe_count);
  i.WriteHtonU16 (m_ofdm_sym_offset);
  i.WriteHtonU16 (m_ofdm_sym_len);
  i.WriteHtonU64 (m_allocatedSubChannels.to_ulong ());
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      i.WriteU8 (m_MCS[k]);
    }
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      i.WriteU8 (m_CQI[k]);
    }
  i.WriteHtonU16 (m_reserved);
}

uint32_t
CouwbatMetaHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  m_flags = i.ReadNtohU32 ();
  m_frequency_band = i.ReadNtohU16 ();
  m_ofdm_sym_sframe_count = i.ReadNtohU32 ();
  m_ofdm_sym_offset = i.ReadNtohU16 ();
  m_ofdm_sym_len = i.ReadNtohU16 ();
  m_allocatedSubChannels = std::bitset<Couwbat::MAX_SUBCHANS> (i.ReadNtohU64 ());
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      m_MCS[k] = i.ReadU8 ();
    }
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      m_CQI[k] = i.ReadU8 ();
    }
  m_reserved = i.ReadNtohU16 ();

  return GetSerializedSize ();
}

} // namespace ns3
