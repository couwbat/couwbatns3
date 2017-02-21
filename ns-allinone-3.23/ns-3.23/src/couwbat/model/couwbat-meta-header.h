/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_META_HEADER_H
#define COUWBAT_META_HEADER_H

#include "ns3/header.h"
#include <string>
#include "ns3/mac48-address.h"
#include "ns3/couwbat.h"
#include <bitset>

namespace ns3 {

/**
 * \ingroup couwbat
 * 
 * Flags that indicate the type of the metaheader
 */
enum meta_header_flags {
//  CW_CMD_WIFI_EXTRA_UNSPEC    = 0x00, /**< reserved */
    CW_CMD_WIFI_EXTRA_ZERO_SF_START = 0x01, /**< PSS phase started indicator */
    CW_CMD_WIFI_EXTRA_ZERO_RX   = 0x02, /**< Set receive subchannels */
    CW_CMD_WIFI_EXTRA_TX        = 0x03, /**< Packet transmission */
    CW_CMD_WIFI_EXTRA_TX_ERR    = 0x04, /**< Transmission failed */
    CW_CMD_WIFI_EXTRA_RX        = 0x05, /**< Packet reception */
    CW_CMD_WIFI_EXTRA_RX_ERR    = 0x06, /**< Failed CRC check */
    CW_CMD_WIFI_EXTRA_NO_SEQ    = 0x07,  /**< Sequence aborted, no sequence info */
    /* add new receptions status here */
//  __CW_CMD_WIFI_EXTRA_AFTER_LAST,
//  __CW_CMD_WIFI_EXTRA_MAX = __CW_CMD_WIFI_EXTRA_AFTER_LAST -1
};

/**
 * \ingroup couwbat
 *
 * \brief Couwbat meta packet header for Couwbat MAC frame
 * exchanged between MAC and PHY
 */
class CouwbatMetaHeader : public Header
{
public:
  CouwbatMetaHeader ();

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

  uint32_t m_flags;	/**< see enum meta_header_flags */
  uint16_t m_frequency_band;	/**< sub-band (0 to 1 GHz, 1 to 2Ghz, or 2 to 3 GHz */
  uint32_t m_ofdm_sym_sframe_count;	/**< absolute superframe counter */
  uint16_t m_ofdm_sym_offset;	/**< relative start (time) of MPDU */
  uint16_t m_ofdm_sym_len;	/**< duration (number of ODFM symbols) of MPDU */
  std::bitset<Couwbat::MAX_SUBCHANS> m_allocatedSubChannels;	/**< bitarray of used logical subchannels */
  uint8_t m_MCS[Couwbat::MAX_SUBCHANS];	/**< MCS/ Bitrate for each subchannel */
  uint8_t m_CQI[Couwbat::MAX_SUBCHANS];	/**< channel quality indicator per subchannel */
  uint16_t m_reserved;	/**< (reserved) */
};

} // namespace ns3


#endif /* COUWBAT_META_HEADER_H */
