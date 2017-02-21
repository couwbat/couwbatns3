/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iomanip>
#include <iostream>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "couwbat-mac-header.h"
#include "ns3/address-utils.h"

namespace ns3 {

std::ostream&
operator<< (std::ostream& out, const couwbat_frame_t value)
{
  return out << (uint32_t) value;
}

NS_LOG_COMPONENT_DEFINE ("CouwbatMacHeader");

NS_OBJECT_ENSURE_REGISTERED (CouwbatMacHeader);

CouwbatMacHeader::CouwbatMacHeader ()
  : m_fc (COUWBAT_FC_UNINITIALIZED),
    m_seq (0)
{
  NS_LOG_FUNCTION (this);
}

void
CouwbatMacHeader::SetFrameType (couwbat_frame_t fc)
{
  NS_LOG_FUNCTION (this);
  m_fc = uint8_t (fc);
}

couwbat_frame_t
CouwbatMacHeader::GetFrameType (void) const
{
  NS_LOG_FUNCTION (this);
  return (couwbat_frame_t) m_fc;
}

void
CouwbatMacHeader::SetSource (Mac48Address source)
{
  NS_LOG_FUNCTION (this << source);
  m_source = source;
}

Mac48Address
CouwbatMacHeader::GetSource (void) const
{
  NS_LOG_FUNCTION (this);
  return m_source;
}

void 
CouwbatMacHeader::SetDestination (Mac48Address dst)
{
  NS_LOG_FUNCTION (this << dst);
  m_destination = dst;
}

Mac48Address
CouwbatMacHeader::GetDestination (void) const
{
  NS_LOG_FUNCTION (this);
  return m_destination;
}

void
CouwbatMacHeader::SetSequence (uint8_t seq)
{
  NS_LOG_FUNCTION (this);
  m_seq = seq;
}

uint8_t
CouwbatMacHeader::GetSequence (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

uint32_t 
CouwbatMacHeader::GetHeaderSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId 
CouwbatMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatMacHeader")
    .SetParent<Header> ()
    .AddConstructor<CouwbatMacHeader> ()
  ;
  return tid;
}

TypeId 
CouwbatMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
CouwbatMacHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

  os << "fc=" << uint32_t (m_fc)
     << ", source=" << m_source
     << ", destination=" << m_destination
     << ", seq=" << uint32_t (m_seq);
}

uint32_t 
CouwbatMacHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static const uint32_t ret = sizeof (m_fc) + 2 * MAC_ADDR_SIZE + sizeof (m_seq);
  return ret;
}

void
CouwbatMacHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  i.WriteU8 (m_fc);
  WriteTo (i, m_source);
  WriteTo (i, m_destination);
  i.WriteU8 (m_seq);
}

uint32_t
CouwbatMacHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  m_fc = i.ReadU8 ();
  ReadFrom (i, m_source);
  ReadFrom (i, m_destination);
  m_seq = i.ReadU8 ();

  return GetSerializedSize ();
}

} // namespace ns3
