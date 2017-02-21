/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_MAP_SUBPACKET_H
#define COUWBAT_MAP_SUBPACKET_H

#include "ns3/header.h"
#include "ns3/couwbat.h"
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup couwbat
 *
 * \brief Packet header for Couwbat MAP subframes
 */
class CouwbatMapSubpacket : public Header
{
public:
  CouwbatMapSubpacket ();

  uint32_t GetHeaderSize () const;

  void SetAmc (const uint8_t mcs[]); //!< Helper for setting AMC bitarray

  void GetMcs (uint8_t mcs[]) const; //!< Helper for reading AMC bitarray

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

  static const int MAC_ADDR_SIZE = 6; //!< size of dest addr header field

  Mac48Address m_ie_id;   //!< Destination address
  uint8_t m_amc[24]; //!< bitarray of AMC
  uint16_t m_ofdm_offset; //!< OFDM offset
  uint16_t m_ofdm_count; //!< OFDM count
};

} // namespace ns3


#endif /* COUWBAT_MAP_SUBPACKET_H */
