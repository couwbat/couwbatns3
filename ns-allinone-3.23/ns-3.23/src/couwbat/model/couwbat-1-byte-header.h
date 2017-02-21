/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_1_BYTE_HEADER_H
#define COUWBAT_1_BYTE_HEADER_H

#include "ns3/header.h"
#include <string>
#include "ns3/mac48-address.h"

namespace ns3 {

/**
 * \ingroup couwbat
 *
 * \brief Header for an extra data field of size 1 byte. 
 * This class is currently used for indicating the number of Couwbat UL/DL MAP subpackets that follow in the MAP frame format.
 */
class Couwbat1ByteHeader : public Header
{
public:
  Couwbat1ByteHeader ();

  void SetVal (uint8_t v); //!< Set the contained value

  uint8_t GetVal (void) const; //!< Get the contained value

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
  uint8_t val; //!< Storage for contained value
};

} // namespace ns3


#endif /* COUWBAT_1_BYTE_HEADER_H */
