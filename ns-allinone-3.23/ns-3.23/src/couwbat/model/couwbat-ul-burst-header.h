/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_UL_BURST_HEADER_H
#define COUWBAT_UL_BURST_HEADER_H

#include "ns3/header.h"
#include "ns3/couwbat.h"

namespace ns3 {

/**
 * \ingroup couwbat
 *
 * \brief Packet header for Couwbat UL bursts
 */
class CouwbatUlBurstHeader : public Header
{
public:
  CouwbatUlBurstHeader ();

  uint32_t GetHeaderSize () const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  uint8_t m_ack;
  uint8_t m_cqi[Couwbat::MAX_SUBCHANS];
};

} // namespace ns3


#endif /* COUWBAT_UL_BURST_HEADER_H */
