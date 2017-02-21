/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iomanip>
#include <iostream>

#include "couwbat-1-byte-header.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CouwbatTotalIeHeader");

NS_OBJECT_ENSURE_REGISTERED (Couwbat1ByteHeader);

Couwbat1ByteHeader::Couwbat1ByteHeader ()
  : val (0)
{
  NS_LOG_FUNCTION (this);
}

void
Couwbat1ByteHeader::SetVal (uint8_t v)
{
  NS_LOG_FUNCTION (this << uint32_t (v));
  val = v;
}

uint8_t
Couwbat1ByteHeader::GetVal (void) const
{
  NS_LOG_FUNCTION (this);
  return val;
}

uint32_t
Couwbat1ByteHeader::GetHeaderSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId 
Couwbat1ByteHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Couwbat1ByteHeader")
    .SetParent<Header> ()
    .AddConstructor<Couwbat1ByteHeader> ()
  ;
  return tid;
}

TypeId 
Couwbat1ByteHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
Couwbat1ByteHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);

  os << "val=" << int (val);
}

uint32_t 
Couwbat1ByteHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static uint32_t ret = sizeof (val);
  return ret;
}

void
Couwbat1ByteHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  i.WriteU8 (val);
}

uint32_t
Couwbat1ByteHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  val = i.ReadU8 ();

  return GetSerializedSize ();
}

} // namespace ns3
