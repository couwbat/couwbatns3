#include "couwbat-mac.h"
#include "couwbat.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("CouwbatMac");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (CouwbatMac);

TypeId
CouwbatMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatMac")
    .SetParent<Object> ()

    .SetGroupName ("Couwbat")
  ;
  return tid;
}

CouwbatMac::CouwbatMac ()
: m_netlinkMode (false)
{
  NS_LOG_FUNCTION (this);
}

CouwbatMac::~CouwbatMac ()
{
  NS_LOG_FUNCTION (this);
}

void
CouwbatMac::SetAddress (Mac48Address ad)
{
  NS_LOG_FUNCTION (this << ad);
  m_address = ad;
}

Mac48Address 
CouwbatMac::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_address;
}

void
CouwbatMac::SetDevice (Ptr<Object> device)
{
  NS_LOG_FUNCTION (this << device);
  m_device = device;
}

Ptr<Object>
CouwbatMac::GetDevice (void)
{
  NS_LOG_FUNCTION (this);
  return m_device;
}

void
CouwbatMac::SetPhy (Ptr<CouwbatPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_phy = phy;
}

Ptr<CouwbatPhy>
CouwbatMac::GetPhy (void)
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void
CouwbatMac::SetNetlinkMode (bool value)
{
  NS_LOG_FUNCTION (this);
  m_netlinkMode = value;
}

void
CouwbatMac::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (this);
  if (!m_netlinkMode)
    {
      m_device->GetObject<CouwbatNetDevice> ()->ForwardUp (packet, from, to);
    }

  if (m_netlinkMode && !m_netlinkForwardUpCallback.IsNull ())
    {
      m_netlinkForwardUpCallback (packet, from, to);
    }
}

void
CouwbatMac::SetNetlinkForwardUpCallback (Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb)
{
  NS_LOG_FUNCTION (this);
  m_netlinkForwardUpCallback = cb;
}

double
CouwbatMac::BitsPerSymbol (enum CouwbatMCS mcs)
{
  double bits_per_symb = 0;
  switch (mcs)
  {
    case COUWBAT_MCS_BPSK_1_2:
      bits_per_symb = 0.5;
      break;
    case COUWBAT_MCS_QPSK_1_2:
      bits_per_symb = 1;
      break;
    case COUWBAT_MCS_QPSK_3_4:
      bits_per_symb = 1.5;
      break;
    case COUWBAT_MCS_QPSK:
      bits_per_symb = 2;
      break;
    case COUWBAT_MCS_16QAM_1_2:
      bits_per_symb = 2;
      break;
    case COUWBAT_MCS_16QAM_3_4:
      bits_per_symb = 3;
      break;
    case COUWBAT_MCS_16QAM:
      bits_per_symb = 4;
      break;
    case COUWBAT_MCS_64QAM_2_3:
      bits_per_symb = 4;
      break;
    case COUWBAT_MCS_64QAM_3_4:
      bits_per_symb = 4.5;
      break;
    case COUWBAT_MCS_64QAM_5_6:
      bits_per_symb = 5;
      break;
    case COUWBAT_MCS_64QAM:
      bits_per_symb = 6;
      break;
    case COUWBAT_MCS_256QAM_3_4:
      bits_per_symb = 6;
      break;
    case COUWBAT_MCS_256QAM_5_6:
      bits_per_symb = 5*8/6;
      break;

    default:
      bits_per_symb = 1;
      break;
  }
  return bits_per_symb;
}

double
CouwbatMac::TransmittableBytesWithSymbols (uint32_t num_symbols, uint32_t num_subchannels, const std::vector<enum CouwbatMCS> &mcs)
{

  NS_LOG_FUNCTION (num_symbols << num_subchannels);

  NS_ASSERT (mcs.size () == num_subchannels);
  NS_ASSERT (num_subchannels > 0);

  double preamble_symb = Couwbat::GetSymbolspreamble();

  double total_bits_per_symb = 0;
  for (uint32_t i = 0; i < mcs.size (); ++i)
    {
      NS_ASSERT (mcs[i] != COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE);
      total_bits_per_symb += (CouwbatPhy::BitsPerSymbol (mcs[i]) * Couwbat::GetNumberOfDataSubcarriersPerSubchannel());
    }
  double total_bytes_per_symb = total_bits_per_symb / 8;

  double transmittable_bytes = (num_symbols - preamble_symb) * total_bytes_per_symb;

  NS_LOG_FUNCTION (transmittable_bytes);
  return transmittable_bytes;
}

double
CouwbatMac::NecessarySymbolsForBytes (uint32_t size_bytes, uint32_t num_subchannels, const std::vector<enum CouwbatMCS> &mcs, double &padding_bytes)
{
  NS_ASSERT (mcs.size () == num_subchannels);
  NS_ASSERT (num_subchannels > 0);

  double bits_per_symb = 0;
  for (uint32_t i = 0; i < mcs.size(); ++i)
    {
      bits_per_symb += (CouwbatPhy::BitsPerSymbol (mcs[i]) * Couwbat::GetNumberOfDataSubcarriersPerSubchannel());
    }

  // size is in bytes
  double size_bits = size_bytes * 8.0;

  double preamble_symb = Couwbat::GetSymbolspreamble();

  // number of OFDM symbols depends on the MCS, number of subcarriers per subchannel
  // and total number of used subchannels
  double num_symbols = preamble_symb + (size_bits / bits_per_symb);

  if (num_symbols != floor (num_symbols))
    {
      padding_bytes = (bits_per_symb - fmod (size_bits, bits_per_symb)) / 8.0;
      NS_ASSERT (padding_bytes != 0);
    }
  else
    {
      padding_bytes = 0;
    }

  NS_LOG_FUNCTION (num_symbols);
  return num_symbols;
}

} // namespace ns3
