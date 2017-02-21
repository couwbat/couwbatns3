/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_PACKET_FCS_H
#define COUWBAT_PACKET_FCS_H

#include "ns3/header.h"
#include "ns3/trailer.h"
#include "ns3/packet.h"
#include <string>

namespace ns3 {


/**
 * \ingroup couwbat
 *
 * \brief Packet FCS trailer for Couwbat MAC frames that should be used
 * for packet CREATION and SERIALIZATION
 *
 * This class can be used to add the FCS at the end of an
 * Couwbat MAC frame.
 */
class CouwbatFcsTrailer : public Trailer
{
public:
  CouwbatFcsTrailer ();

  /**
   * \brief Enable or disable FCS checking and calculations
   * \param enable If true, enables FCS calculations.
   */
  void EnableFcs (bool enable);

  /**
   * \brief Updates the FCS Field to the correct FCS
   *
   * \param p Reference to a packet on which the FCS should be
   * calculated. The packet should not contain a CouwbatFcsTrailer.
   *
   * \param size The number of bytes from the start of the packet for which
   * to calculate FCS. If it is 0 or greater than the packet length in bytes,
   * FCS is calculated for the whole packet.
   */
  void CalcFcs (Ptr<const Packet> p, uint32_t size);

  /**
   * Calculate an FCS on the provided packet and check this value against
   * the FCS found when the trailer was deserialized (the one in the transmitted
   * packet).
   *
   * If FCS checking is disabled, this method will always
   * return true.
   *
   * \param p Reference to the packet on which the FCS should be
   * calculated. The packet should not contain a CouwbatFcsHeader or CouwbatFcsTrailer.
   *
   * \param size The number of bytes from the start of the packet for which
   * to calculate FCS. If it is 0 or greater than the packet length in bytes,
   * FCS is calculated for the whole packet.
   *
   * \return Returns true if the Packet FCS matches the FCS in the trailer,
   * false otherwise.
   */
  bool CheckFcs (Ptr<const Packet> p, uint32_t size);

  /**
   * Same as CheckFcs (Ptr<const Packet> p, uint32_t size) but takes an existing buffer
   */
  bool CheckFcs (uint8_t *buffer, uint32_t size);

  /**
   * \brief Sets the FCS to a new value
   * \param fcs New FCS value
   */
  void SetFcs (uint32_t fcs);

  /**
   * \return the FCS contained in this trailer
   */
  uint32_t GetFcs ();

  uint32_t GetTrailerSize () const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator end) const;
  virtual uint32_t Deserialize (Buffer::Iterator end);
  
private:

  /**
   * If true, FCS calculations are enabled.
   * If false, m_fcs is set to 0 and CheckFcs returns true.
   */
  bool m_calcFcs;
  uint32_t m_fcs; //!< Value of the FCS contained in the trailer
};


/**
 * \ingroup couwbat
 *
 * \brief Packet FCS header for Couwbat MAC frames that should be used
 * for packet DESERIALIZATION
 *
 * This class can be used to verify the FCS at the end of an
 * Couwbat MAC frame.
 */
class CouwbatFcsHeader : public Header
{
public:
  CouwbatFcsHeader ();

  /**
   * \brief Enable or disable FCS checking and calculations
   * \param enable If true, enables FCS calculations.
   */
  void EnableFcs (bool enable);

  /**
   * \brief Sets the FCS to a new value
   * \param fcs New FCS value
   */
  void SetFcs (uint32_t fcs);

  /**
   * \return the FCS contained in this trailer
   */
  uint32_t GetFcs ();

  /**
   * Calculate an FCS on the provided packet and check this value against
   * the FCS found when the trailer was deserialized (the one in the transmitted
   * packet).
   *
   * If FCS checking is disabled, this method will always
   * return true.
   *
   * \param p Reference to the packet on which the FCS should be
   * calculated. The packet should not contain a CouwbatFcsHeader or CouwbatFcsTrailer.
   *
   * \param size The number of bytes from the start of the packet for which
   * to calculate FCS. If it is 0 or greater than the packet length in bytes,
   * FCS is calculated for the whole packet.
   *
   * \return Returns true if the Packet FCS matches the FCS in the trailer,
   * false otherwise.
   */
  bool CheckFcs (Ptr<const Packet> p, uint32_t size);

  /**
   * Same as CheckFcs (Ptr<const Packet> p, uint32_t size) but takes an existing buffer
   */
  bool CheckFcs (uint8_t *buffer, uint32_t size);

  uint32_t GetHeaderSize () const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:

  /**
   * If true, FCS calculations are enabled.
   * If false, m_fcs is set to 0 and CheckFcs returns true.
   */
  bool m_calcFcs;
  uint32_t m_fcs; //!< Value of the FCS contained in the trailer
};

} // namespace ns3

#endif /* COUWBAT_PACKET_FCS_H */
