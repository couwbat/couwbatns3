/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "couwbat-packet-fcs.h"
#include "ns3/crc32.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CouwbatFcs");

/**
 * CouwbatFcsTrailer implementation
 */

NS_OBJECT_ENSURE_REGISTERED (CouwbatFcsTrailer);

CouwbatFcsTrailer::CouwbatFcsTrailer ()
  : m_calcFcs (true),
    m_fcs (0)
{
  NS_LOG_FUNCTION (this);
}

void
CouwbatFcsTrailer::EnableFcs (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_calcFcs = enable;
}

void
CouwbatFcsTrailer::CalcFcs (Ptr<const Packet> p, uint32_t size)
{
  NS_LOG_FUNCTION (this << p);
  if (!m_calcFcs)
    {
      return;
    }

  uint8_t *buffer;
  uint32_t packSz = p->GetSize ();
  if (size == 0 || size > packSz)
    {
      size = packSz;
    }
  buffer = new uint8_t[size];
  p->CopyData (buffer, size);
  m_fcs = CRC32Calculate (buffer, size);
  delete[] buffer;
}

bool
CouwbatFcsTrailer::CheckFcs (Ptr<const Packet> p, uint32_t size)
{
  NS_LOG_FUNCTION (this << p << size);
  if (!m_calcFcs)
    {
      return true;
    }

  uint8_t *buffer;
  uint32_t crc;
  uint32_t packSz = p->GetSize ();
  if (size == 0 || size > packSz)
    {
      size = packSz;
    }
  buffer = new uint8_t[size];
  p->CopyData (buffer, size);
  crc = CRC32Calculate (buffer, size);
  delete[] buffer;
  return (m_fcs == crc);
}

bool
CouwbatFcsTrailer::CheckFcs (uint8_t *buffer, uint32_t size)
{
  NS_LOG_FUNCTION (this << buffer << size);
  if (!m_calcFcs)
    {
      return true;
    }

  uint32_t crc;

  crc = CRC32Calculate (buffer, size);
  return (m_fcs == crc);
}

void
CouwbatFcsTrailer::SetFcs (uint32_t fcs)
{
  NS_LOG_FUNCTION (this << fcs);
  m_fcs = fcs;
}

uint32_t
CouwbatFcsTrailer::GetFcs (void)
{
  NS_LOG_FUNCTION (this);
  return m_fcs;
}

uint32_t
CouwbatFcsTrailer::GetTrailerSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId 
CouwbatFcsTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatFcsTrailer")
    .SetParent<Trailer> ()
    .AddConstructor<CouwbatFcsTrailer> ()
  ;
  return tid;
}

TypeId 
CouwbatFcsTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
CouwbatFcsTrailer::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "fcs=" << m_fcs;
}

uint32_t 
CouwbatFcsTrailer::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static uint32_t ret = sizeof (m_fcs);
  return ret;
}

void
CouwbatFcsTrailer::Serialize (Buffer::Iterator end) const
{
  NS_LOG_FUNCTION (this << &end);
  Buffer::Iterator i = end;
  i.Prev (GetSerializedSize ());

  i.WriteU32 (m_fcs);
}

uint32_t
CouwbatFcsTrailer::Deserialize (Buffer::Iterator end)
{
  NS_LOG_FUNCTION (this << &end);
  Buffer::Iterator i = end;
  uint32_t size = GetSerializedSize ();
  i.Prev (size);

  m_fcs = i.ReadU32 ();

  return size;
}





/**
 * CouwbatFcsHeader implementation
 */

NS_OBJECT_ENSURE_REGISTERED (CouwbatFcsHeader);

CouwbatFcsHeader::CouwbatFcsHeader ()
  : m_calcFcs (true),
    m_fcs (0)
{
  NS_LOG_FUNCTION (this);
}

void
CouwbatFcsHeader::EnableFcs (bool enable)
{
  NS_LOG_FUNCTION (this << enable);
  m_calcFcs = enable;
}

bool
CouwbatFcsHeader::CheckFcs (Ptr<const Packet> p, uint32_t size)
{
  NS_LOG_FUNCTION (this << p << size);
  if (!m_calcFcs)
    {
      return true;
    }

  uint8_t *buffer;
  uint32_t crc;
  uint32_t packSz = p->GetSize ();
  if (size == 0 || size > packSz)
    {
      size = packSz;
    }
  buffer = new uint8_t[size];
  p->CopyData (buffer, size);
  crc = CRC32Calculate (buffer, size);
  delete[] buffer;
  return (m_fcs == crc);
}

bool
CouwbatFcsHeader::CheckFcs (uint8_t *buffer, uint32_t size)
{
  NS_LOG_FUNCTION (this << buffer << size);
  if (!m_calcFcs)
    {
      return true;
    }

  uint32_t crc;

  crc = CRC32Calculate (buffer, size);
  return (m_fcs == crc);
}

void
CouwbatFcsHeader::SetFcs (uint32_t fcs)
{
  NS_LOG_FUNCTION (this << fcs);
  m_fcs = fcs;
}

uint32_t
CouwbatFcsHeader::GetFcs (void)
{
  NS_LOG_FUNCTION (this);
  return m_fcs;
}

uint32_t
CouwbatFcsHeader::GetHeaderSize (void) const
{
  NS_LOG_FUNCTION (this);
  return GetSerializedSize ();
}

TypeId
CouwbatFcsHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatFcsHeader")
    .SetParent<Trailer> ()
    .AddConstructor<CouwbatFcsHeader> ()
  ;
  return tid;
}

TypeId
CouwbatFcsHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CouwbatFcsHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "fcs=" << m_fcs;
}

uint32_t
CouwbatFcsHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  static uint32_t ret = sizeof (m_fcs);
  return ret;
}

void
CouwbatFcsHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  i.WriteU32 (m_fcs);
}

uint32_t
CouwbatFcsHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  uint32_t size = GetSerializedSize ();

  m_fcs = i.ReadU32 ();

  return size;
}

} // namespace ns3
