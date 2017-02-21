/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_MPDU_DELIMITER_H
#define COUWBAT_MPDU_DELIMITER_H

#include "ns3/header.h"
#include "ns3/packet.h"
#include <string>

namespace ns3 {

/**
 * \ingroup couwbat
 *
 * \brief MPDU delimiter implementation for data frames
 *
 * This class can be used to create MPDU delimiters for
 * A-MPDU subframes. Format and implementation is similar to 802.11
 */
class CouwbatMpduDelimiter : public Header
{
public:
  /**
   * \brief Construct an invalid delimiter
   *
   *    Only use when the header will subsequently be deserialized
   *    or manually set the length.
   */
  CouwbatMpduDelimiter ();

  /**
   * \brief Construct a Couwbat MPDU delimiter
   *
   * \param mpdu_len The length of the following MPDU.
   *    The value must fit in 12 bits or it will be
   *    truncated and horrible things will happen.
   */
  CouwbatMpduDelimiter (uint16_t mpdu_len);

  /**
   * \brief Check if this MPDU delimiter is valid
   *
   * \return True if valid
   *
   * An MPDU delimiter is valid if the delimiter signature
   * matches and the CRC for the first 2 bytes checks out.
   */
  bool IsValid (void) const;

  /**
   * \brief Set the MPDU length
   */
  void SetMpduLen (uint16_t mpdu_len);

  /**
   * \brief Get the MPDU length
   */
  uint32_t GetMpduLen (void) const;

  /**
   *\return Returns the size of the header
   */
  uint32_t GetHeaderSize (void) const;

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
  /**
   * \return The CRC value for the CURRENT m_res_mpdu_len
   */
  uint8_t CalcCrc (void) const;

  /**
   * The value of the first two bytes. First 4 bits are reserved
   * and remaining 12 bits are the MPDU length.
   */
  uint16_t m_res_mpdu_len;
  uint8_t m_crc; //!< The value of the CRC
  uint8_t m_sig; //!< The value of the delimiter signature

  /**
   * The value of the 4 reserved bytes. If it doesn't fit in the 4 least
   * significant bits, it is truncated.
   */
  static const uint16_t COUWBAT_MPDU_DELIMITER_RES_VAL = 0;
  /**
   * The value of the delimiter signature.
   */
  static const uint8_t COUWBAT_MPDU_DELIMITER_SIGNATURE = 0x43;
};

} // namespace ns3

#endif /* COUWBAT_MPDU_DELIMITER_H */
