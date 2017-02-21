/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "couwbat-packet-helper.h"
#include "ns3/log.h"
#ifdef __APPLE__
    #include <machine/endian.h>
#else
    #include <endian.h>
#endif
#include "couwbat-map-subpacket.h"
#include "couwbat-ul-burst-header.h"
#include "couwbat-mpdu-delimiter.h"
#include "ns3/packet.h"
#include <algorithm>
#include "couwbat-1-byte-header.h"

namespace ns3
{

std::ostream&
operator << (std::ostream& out, const std::vector<Ptr<Packet> > vec)
{
  out << "{";
  for (std::vector<Ptr<Packet> >::const_iterator i = vec.begin ();
      i != vec.end (); ++i)
    {
      out << (*i)->ToString ();
      if ((i + 1) != vec.end ())
        {
          out << ", ";
        }
    }
  out << "}";
  return out;
}

std::ostream&
operator << (std::ostream& out, const std::vector<uint32_t> vec)
{
  out << "{";
  for (std::vector<uint32_t>::const_iterator i = vec.begin ();
      i != vec.end (); ++i)
    {
      out << *i;
      if ((i + 1) != vec.end ())
        {
          out << ", ";
        }
    }
  out << "}";
  return out;
}

std::ostream&
operator << (std::ostream& out, const std::vector<double> vec)
{
  out << "{";
  for (std::vector<double>::const_iterator i = vec.begin ();
      i != vec.end (); ++i)
    {
      out << *i;
      if ((i + 1) != vec.end ())
        {
          out << ", ";
        }
    }
  out << "}";
  return out;
}

NS_LOG_COMPONENT_DEFINE("CouwbatPacketHelper");

Ptr<Packet>
CouwbatPacketHelper::CreateCouwbatControlPacket (Ptr<Packet> packet,
                                          couwbat_frame_t fc,
                                          Mac48Address source,
                                          Mac48Address destination,
                                          uint8_t seq)
{
  NS_LOG_FUNCTION(packet->GetSize () << fc << source << destination << seq);

  if (packet == 0)
    {
      packet = Create<Packet> ();
    }

  CouwbatMacHeader header;
  header.SetFrameType (fc);
  header.SetSource (source);
  header.SetDestination (destination);
  header.SetSequence (seq);
  packet->AddHeader (header);

  CouwbatFcsTrailer trailer;
  trailer.EnableFcs (true);
  trailer.CalcFcs (packet, 0);
  packet->AddTrailer (trailer);

  return packet;
}

Ptr<Packet>
CouwbatPacketHelper::CreateEmpty (couwbat_frame_t fc, Mac48Address source,
                                  Mac48Address destination, uint8_t seq)
{
  NS_LOG_FUNCTION(fc << source << destination << seq);

  Ptr<Packet> packet = Create<Packet> ();
  return CreateCouwbatControlPacket (packet, fc, source, destination, seq);
}

Ptr<Packet>
CouwbatPacketHelper::ConcatenatePackets (
    const std::vector<Ptr<Packet> >& packets)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Packet> ret = Create<Packet> ();

  std::vector<Ptr<Packet> >::const_iterator i = packets.begin ();
  while (i != packets.end ())
    {
      ret->AddAtEnd (*i);
      ++i;
    }

  return ret;
}

Ptr<Packet>
CouwbatPacketHelper::CreateMap (Mac48Address source,
        const std::vector<Ptr<Packet> >& dlMapSubpackets,
        const std::vector<Ptr<Packet> >& ulMapSubpackets)
{
  NS_LOG_FUNCTION(source << dlMapSubpackets.size () << ulMapSubpackets.size ());

  Ptr<Packet> dl = CouwbatPacketHelper::ConcatenatePackets (dlMapSubpackets);
  Couwbat1ByteHeader dl_h;
  dl_h.SetVal (dlMapSubpackets.size ());
  dl->AddHeader (dl_h);

  Ptr<Packet> ul = CouwbatPacketHelper::ConcatenatePackets (ulMapSubpackets);
  Couwbat1ByteHeader ul_h;
  ul_h.SetVal (ulMapSubpackets.size ());
  ul->AddHeader (ul_h);

  std::vector<Ptr<Packet> > payload;
  payload.push_back (dl);
  payload.push_back (ul);
  Ptr<Packet> packet = CouwbatPacketHelper::ConcatenatePackets (payload);

  couwbat_frame_t fc = COUWBAT_FC_CONTROL_MAP;
  Mac48Address BroadcastAddr = "ff:ff:ff:ff:ff:ff";
  uint8_t seq = 0;

  return CreateCouwbatControlPacket (packet, fc, source, BroadcastAddr, seq);
}

bool
CouwbatPacketHelper::GetMapSubpackets (Ptr<const Packet> packet, std::vector<Ptr<Packet> >& dlSubpackets,
               std::vector<Ptr<Packet> >& ulSubpackets)
{
  uint32_t packetSize = packet->GetSize ();
  Packet p (*packet);

  CouwbatMacHeader couwbatHeader;
  if ((packetSize -= couwbatHeader.GetHeaderSize ()) < 0) return false;
  p.RemoveHeader (couwbatHeader);

  CouwbatMapSubpacket dummy;
  const uint32_t subpacketSize = dummy.GetSerializedSize ();

  Couwbat1ByteHeader dlHeader;
  if ((packetSize -= dlHeader.GetHeaderSize ()) < 0) return false;
  p.RemoveHeader (dlHeader);

  uint32_t total_ie_dl = dlHeader.GetVal ();
  while (total_ie_dl != 0)
    {
      // check for corrupt packet, incorrect total_ie_dl
      if ((packetSize -= subpacketSize) < 0) return false;

      dlSubpackets.push_back (p.CreateFragment (0, subpacketSize));
      p.RemoveAtStart (subpacketSize);
      --total_ie_dl;
    }

  Couwbat1ByteHeader ulHeader;
  if ((packetSize -= ulHeader.GetHeaderSize ()) < 0) return false;
  p.RemoveHeader (ulHeader);

  uint32_t total_ie_ul = ulHeader.GetVal ();
  while (total_ie_ul != 0)
    {
      // check for corrupt packet, incorrect total_ie_ul
      if ((packetSize -= subpacketSize) < 0) return false;

      ulSubpackets.push_back (p.CreateFragment (0, subpacketSize));
      p.RemoveAtStart (subpacketSize);
      --total_ie_ul;
    }

  uint32_t fcsSize = couwbatHeader.GetHeaderSize ()
      + subpacketSize * (dlHeader.GetVal () + ulHeader.GetVal ())
      + dlHeader.GetHeaderSize () + ulHeader.GetHeaderSize ();

  CouwbatFcsHeader fcs;
  if ((packetSize -= fcs.GetHeaderSize ()) < 0) return false;
  p.RemoveHeader (fcs);
  bool fcsCorrect = fcs.CheckFcs (packet, fcsSize);
  if (!fcsCorrect) return false;

  return true;
}

Ptr<Packet>
CouwbatPacketHelper::CreateDlDataPacket (
    Mac48Address source, Mac48Address destination,
    uint8_t seq, CouwbatTxQueue &txQueue, uint32_t maxSizeBytes,
    uint8_t ack, std::vector<Ptr<Packet> >& payloadHist)
{
  CouwbatMacHeader header;
  header.SetFrameType (COUWBAT_FC_DATA_DL);
  header.SetSource (source);
  header.SetDestination (destination);
  header.SetSequence (seq);

  CouwbatFcsTrailer trailer;
  trailer.EnableFcs (true);

  Couwbat1ByteHeader ackHeader;
  ackHeader.SetVal (ack);

  Couwbat1ByteHeader nrMpus;

  uint32_t maxSizeBytesWithoutHeaders = maxSizeBytes - (header.GetSerializedSize () + ackHeader.GetSerializedSize () + nrMpus.GetSerializedSize () + trailer.GetSerializedSize ());

  uint8_t mpuCnt = 0;
  Ptr<Packet> burst = CouwbatPacketHelper::CreateBurst (destination, txQueue, maxSizeBytesWithoutHeaders, mpuCnt, payloadHist);
  NS_ASSERT (burst->GetSize () <= maxSizeBytesWithoutHeaders);

  nrMpus.SetVal (mpuCnt);
  burst->AddHeader (nrMpus);
  burst->AddHeader (ackHeader);
  burst->AddHeader (header);

  uint32_t fcsSize = header.GetHeaderSize () + ackHeader.GetHeaderSize () + nrMpus.GetHeaderSize ();
  trailer.CalcFcs (burst, fcsSize);
  burst->AddTrailer (trailer);

  NS_ASSERT (burst->GetSize () <= maxSizeBytes);

  return burst;
}

Ptr<Packet>
CouwbatPacketHelper::CreateUlDataPacket (
    Mac48Address source, Mac48Address destination,
    uint8_t seq, CouwbatTxQueue &txQueue, uint32_t maxSizeBytes,
    uint8_t ack, std::vector<uint8_t> cqi, std::vector<Ptr<Packet> >& payloadHist)
{
  CouwbatMacHeader header;
  header.SetFrameType (COUWBAT_FC_DATA_UL);
  header.SetSource (source);
  header.SetDestination (destination);
  header.SetSequence (seq);

  CouwbatUlBurstHeader ulHeader;
  ulHeader.m_ack = ack;
  for (uint32_t k = 0; k < Couwbat::MAX_SUBCHANS; ++k)
    {
      if (k < cqi.size ())
        {
          ulHeader.m_cqi[k] = cqi[k];
        }
      else
        {
          ulHeader.m_cqi[k] = 0;
        }
    }

  Couwbat1ByteHeader nrMpus;

  CouwbatFcsTrailer trailer;
  trailer.EnableFcs (true);

  uint32_t maxSizeBytesWithoutHeaders = maxSizeBytes - (header.GetSerializedSize () + ulHeader.GetSerializedSize ()
      + nrMpus.GetSerializedSize () + trailer.GetSerializedSize ());

  uint8_t mpuCnt = 0;
  Ptr<Packet> burst = CouwbatPacketHelper::CreateBurst (destination, txQueue, maxSizeBytesWithoutHeaders, mpuCnt, payloadHist);
  NS_ASSERT (burst->GetSize () <= maxSizeBytesWithoutHeaders);

  nrMpus.SetVal (mpuCnt);
  burst->AddHeader (nrMpus);
  burst->AddHeader (ulHeader);
  burst->AddHeader (header);

  uint32_t fcsSize = header.GetHeaderSize () + ulHeader.GetHeaderSize () + nrMpus.GetHeaderSize ();
  trailer.CalcFcs (burst, fcsSize);
  burst->AddTrailer (trailer);

  NS_ASSERT (burst->GetSize () <= maxSizeBytes);

  return burst;
}

Ptr<Packet>
CouwbatPacketHelper::CreateBurst (Mac48Address destination, CouwbatTxQueue &txQueue, uint32_t maxSizeBytes, uint8_t &mpuCnt,
                                  std::vector<Ptr<Packet> >& payloadHist)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<Packet> ret = Create<Packet> ();
  mpuCnt = 0;

  Ptr<Packet> currentPayload = txQueue.Peek (destination);
  while (currentPayload && mpuCnt < 254)
    {
      const uint32_t payloadSize = currentPayload->GetSize ();
      CouwbatMpduDelimiter delimiter (payloadSize);
      // std::cout << "payloadSize:" << payloadSize << "\n";
      static const uint32_t delimiterSize = delimiter.GetSerializedSize ();
      const uint32_t dataSize = payloadSize + delimiterSize;
      uint32_t padding = 0;
      if (dataSize % 4 != 0)
        {
          padding = 4 - (dataSize % 4);
        }
      const uint32_t totalSize = dataSize + padding;
      if (totalSize + delimiterSize > maxSizeBytes)
        {
          // doesn't fit, stop adding
          break;
        }

      bool retransmission = false;
      txQueue.Pop (destination, retransmission);
      Ptr<Packet> mpdu_header = Create<Packet> ();
      mpdu_header->AddHeader (delimiter);
      ret->AddAtEnd (mpdu_header);
      ret->AddAtEnd (currentPayload);
      if (!retransmission)
        {
          payloadHist.push_back (currentPayload);
        }
      if (padding > 0)
        {
          // Add real padding
          uint8_t *buf = new uint8_t[padding];
          Ptr<Packet> padPacket = Create<Packet> (buf, padding);
          delete[] buf;
          ret->AddAtEnd (padPacket);
        }

      maxSizeBytes -= totalSize;
      currentPayload = txQueue.Peek (destination);
      ++mpuCnt;
    }

  CouwbatMpduDelimiter lastDelimiter (0);
  Ptr<Packet> mpdu_header = Create<Packet> ();
  mpdu_header->AddHeader (lastDelimiter);
  ret->AddAtEnd (mpdu_header);
  ++mpuCnt;

  return ret;
}

bool
CouwbatPacketHelper::GetPayload (Ptr<Packet> packet, std::vector<Ptr<Packet> > &payloadTarget, CouwbatUlBurstHeader *ulHeaderTarget, Couwbat1ByteHeader *dlHeaderTarget)
{
  NS_LOG_FUNCTION (packet << ulHeaderTarget << dlHeaderTarget);

  uint32_t packetSize = packet->GetSize ();
  Packet p (*packet);
  p.RemoveAllByteTags ();
  p.RemoveAllPacketTags ();

  CouwbatMacHeader header;
  if ((packetSize -= header.GetHeaderSize ()) < 0) return false;
  p.RemoveHeader (header);
  uint32_t fcsSize = header.GetHeaderSize ();
  if (header.GetFrameType () == COUWBAT_FC_DATA_UL)
    {
      CouwbatUlBurstHeader ul_header;
      if ((packetSize -= ul_header.GetHeaderSize ()) < 0) return false;
      p.RemoveHeader (ul_header);
      fcsSize += ul_header.GetHeaderSize ();
      if (ulHeaderTarget)
        {
          *ulHeaderTarget = ul_header;
        }
    }

  if (header.GetFrameType () == COUWBAT_FC_DATA_DL)
    {
      Couwbat1ByteHeader dl_header;
      if ((packetSize -= dl_header.GetHeaderSize ()) < 0) return false;
      p.RemoveHeader (dl_header);
      fcsSize += dl_header.GetHeaderSize ();
      if (dlHeaderTarget)
        {
          *dlHeaderTarget = dl_header;
        }
    }

  Couwbat1ByteHeader nrMpus;
  if ((packetSize -= nrMpus.GetHeaderSize ()) < 0) return false;
  p.RemoveHeader (nrMpus);
  fcsSize += nrMpus.GetHeaderSize ();
  uint32_t payloadCount = nrMpus.GetVal ();

  NS_LOG_INFO (p.ToString ());

  NS_ASSERT (packetSize == p.GetSize ());

  while (payloadCount != 0 && p.GetSize () > 4)
    {
      CouwbatMpduDelimiter mpdu_del;
      if ((packetSize -= mpdu_del.GetHeaderSize ()) < 0) return false;
      p.RemoveHeader (mpdu_del);

      uint32_t mpdu_len = mpdu_del.GetMpduLen ();
      if (mpdu_len == 0)
        {
          break;
        }
      if (mpdu_del.IsValid ())
        {
          uint32_t padding = 0;
          if ((mpdu_len % 4) != 0 && (p.GetSize () - mpdu_len) > 4)
            {
              padding = 4 - (mpdu_len % 4);
            }

          if ((packetSize -= mpdu_len + padding) < 0) return false;
          payloadTarget.push_back (p.CreateFragment (0, mpdu_len));
          p.RemoveAtStart (mpdu_len + padding);

          --payloadCount;
        }
    }

  CouwbatFcsHeader fcs;
  if ((packetSize -= fcs.GetHeaderSize ()) < 0) return false;
  p.RemoveHeader (fcs);
  bool fcsCorrect = fcs.CheckFcs (packet, fcsSize);
  if (!fcsCorrect) return false;

  return true;
}

} // namespace ns3
