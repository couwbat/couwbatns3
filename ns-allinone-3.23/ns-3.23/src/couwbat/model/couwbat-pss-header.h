/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_PSS_HEADER_H
#define COUWBAT_PSS_HEADER_H

#include "ns3/header.h"
#include "ns3/couwbat.h"
#include <bitset>

namespace ns3 {

/**
 * \ingroup couwbat
 *
 * \brief Packet header for Couwbat PSS frames. Contains information about
 * backup channel, MAP length, and wideband allocation.
 */
class CouwbatPssHeader : public Header
{
public:
  union BackupChannel_t {
    struct {
      uint16_t chSwitchActive : 1;
      uint16_t newChNumber : 6;
      uint16_t chSwitchCounter : 9;
    } fields;
    uint16_t value;
  };
  union PssMaintain_t {
      struct {
        uint16_t pssFragments : 6;
        uint16_t mapLength : 10;
      } fields;
      uint16_t value;
    };

  CouwbatPssHeader ();

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

  union BackupChannel_t m_backupChannel;
  union PssMaintain_t m_pssMaintain;
  //uint16_t m_pssFragments;
  std::bitset<Couwbat::MAX_SUBCHANS> m_allocation;
};

} // namespace ns3


#endif /* COUWBAT_PSS_HEADER_H */
