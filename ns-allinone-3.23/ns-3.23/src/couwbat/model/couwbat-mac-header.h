/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_MAC_HEADER_H
#define COUWBAT_MAC_HEADER_H

#include "ns3/header.h"
#include <string>
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup couwbat
 * 
 * Couwbat MAC Frame Type
 */
enum couwbat_frame_t
  {
    COUWBAT_FC_CONTROL_PSS = 0, //!< PSS type
    COUWBAT_FC_CONTROL_STA_ASSOC = 1, //!< STA association type
    COUWBAT_FC_CONTROL_STA_DISASSOC = 2, //!< STA disassociation type
    COUWBAT_FC_CONTROL_MAP = 3, //!< MAP type
    COUWBAT_FC_DATA_DL = 64, //!< Data downlink type
    COUWBAT_FC_DATA_UL = 65, //!< Data uplink type
    COUWBAT_FC_UNINITIALIZED
  };

std::ostream& operator<< (std::ostream& out, const couwbat_frame_t value);

/**
 * \ingroup couwbat
 *
 * \brief Generic packet header for all Couwbat MAC frames
 *
 * This class can be used to add a header to a Couwbat packet that
 * will specify the Couwbat frame type and subtype, source and destination
 * addresses, and sequence number.
 */
class CouwbatMacHeader : public Header
{
public:
  /**
   * \brief Construct a null Couwbat MAC header
   */
  CouwbatMacHeader ();

  /**
   * \param fc The Couwbat frame type of this packet
   */
  void SetFrameType (couwbat_frame_t fc);

  /**
   * \param source The source address of this packet
   */
  void SetSource (Mac48Address source);

  /**
   * \param destination The destination address of this packet
   */
  void SetDestination (Mac48Address destination);

  /**
   * \param seq The sequence number of this packet
   */
  void SetSequence (uint8_t seq);

  /**
   * \return The Couwbat frame type of this packet
   */
  couwbat_frame_t GetFrameType (void) const;

  /**
   * \return The value of a Couwbat frame type
   */
  static uint8_t GetFrameTypeValue (couwbat_frame_t fc);

  /**
   * \return The source address of this packet
   */
  Mac48Address GetSource (void) const;

  /**
   * \return The destination address of this packet
   */
  Mac48Address GetDestination (void) const;

  /**
   * \return The sequence number of this packet
   */
  uint8_t GetSequence (void) const;

  /**
   * \return The size of the header
   */
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

private:
  static const int MAC_ADDR_SIZE = 6; //!< size of src/dest addr header fields

  uint8_t m_fc;  //!< Frame control field
  Mac48Address m_source;        //!< Source address
  Mac48Address m_destination;   //!< Destination address
  uint8_t m_seq;  //!< Sequence number field
};

} // namespace ns3


#endif /* COUWBAT_MAC_HEADER_H */
