/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COUWBAT_PACKET_HELPER_H
#define COUWBAT_PACKET_HELPER_H

#include "couwbat-mac-header.h"
#include "couwbat-mpdu-delimiter.h"
#include "couwbat-map-subpacket.h"
#include "couwbat-meta-header.h"
#include "couwbat-pss-header.h"
#include "couwbat-tx-queue.h"
#include "couwbat-ul-burst-header.h"
#include "couwbat-1-byte-header.h"
#include "couwbat-packet-fcs.h"

namespace ns3 {

std::ostream& operator << (std::ostream& out, const std::vector<Ptr<Packet> > vec);

std::ostream& operator << (std::ostream& out, const std::vector<uint32_t> vec);

std::ostream& operator << (std::ostream& out, const std::vector<double> vec);

/**
 * \ingroup couwbat
 *
 * \brief Couwbat MAC packet helper functions
 *
 * This class offers the necessary helper functions to process with Couwbat MAC Packets.in the MAC implementation for BS and STA.
 *
 */
class CouwbatPacketHelper
{
public:
  /**
   * \brief Create a Couwbat Packet without a payload
   *
   * \param fc Couwbat frame type
   * \param source Source address
   * \param destination Destination address
   * \param seq Sequence number
   *
   * Should currently only be used for association and disassociation Packets
   */
  static Ptr<Packet> CreateEmpty (couwbat_frame_t fc,
      Mac48Address source, Mac48Address destination, uint8_t seq);

  /**
     * \brief Create a Couwbat Packet
     *
     * \param packet Payload packet
     * \param fc Couwbat frame type
     * \param source Source address
     * \param destination Destination address
     * \param seq Sequence number
     */
    static Ptr<Packet> CreateCouwbatControlPacket (Ptr<Packet> packet, couwbat_frame_t fc,
        Mac48Address source, Mac48Address destination, uint8_t seq);


  /**
   * \brief Create a Couwbat DL/UL Map Packet from a vector of Map subframes
   *
   * \param subframes A vector of all the Map subframes
   */
  static Ptr<Packet> CreateMap (Mac48Address source,
				const std::vector<Ptr<Packet> >& dlSubpackets,
				const std::vector<Ptr<Packet> >& ulSubpackets);

  /**
   * \brief Extract the Map subpackets from a Couwbat DL/UL Map Packet and add them to dlSubpackets and ulSubpackets
   *
   * \returns true if FCS matches, false if not or if packet is otherwise corrupt
   */
  static bool GetMapSubpackets (Ptr<const Packet> packet,
			       std::vector<Ptr<Packet> >& dlSubpackets,
			       std::vector<Ptr<Packet> >& ulSubpackets);

  /**
   * \brief Create a Couwbat DL Data Packet from a vector of payload packets
   */
  static Ptr<Packet> CreateDlDataPacket (Mac48Address source, Mac48Address destination,
					 uint8_t seq, CouwbatTxQueue &txQueue, uint32_t maxSizeBytes,
					 uint8_t ack, std::vector<Ptr<Packet> >& payloadHist);

  /**
   * \brief Create a Couwbat UL Data Packet from a vector of payload packets
   */
  static Ptr<Packet> CreateUlDataPacket (Mac48Address source, Mac48Address destination,
					 uint8_t seq, CouwbatTxQueue &txQueue, uint32_t maxSizeBytes,
					 uint8_t ack, std::vector<uint8_t> cqi, std::vector<Ptr<Packet> >& payloadHist);

  /**
   * \brief Get the payload packets from a Couwbat UL or DL Data Packet
   *
   * Destroys (leaves empty) the argument packet. So make sure you get the
   * info from the headers first before calling this.
   */
  static bool GetPayload (Ptr<Packet> packet, std::vector<Ptr<Packet> > &payloadTarget,
                          CouwbatUlBurstHeader *ulHeaderTarget = 0,
                          Couwbat1ByteHeader *dlHeaderTarget = 0);

private:
    /**
     * \brief Concatenate packets to the first packet
     *
     * Sequentially concatenates all packets to the end of the first packet
     * in the vector.
     */
    static Ptr<Packet> ConcatenatePackets (const std::vector<Ptr<Packet> >& packets);

    static Ptr<Packet> CreateBurst (Mac48Address destination, CouwbatTxQueue &txQueue,
                                    uint32_t maxSizeBytes, uint8_t &mpuCnt,
                                    std::vector<Ptr<Packet> >& payloadHist);
};

} // namespace ns3

#endif /* COUWBAT_PACKET_HELPER_H */
