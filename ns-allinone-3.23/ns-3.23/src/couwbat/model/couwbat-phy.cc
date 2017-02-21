#include "couwbat-phy.h"
#include "ns3/log.h"
#include <math.h>
#include "couwbat.h"

NS_LOG_COMPONENT_DEFINE ("CouwbatPhy");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (CouwbatPhy);

TypeId
CouwbatPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatPhy")
    .SetParent<Object> ()

    .AddTraceSource ("PhyTxBegin",
                     "Trace source indicating a packet has begun transmitting over the channel medium",
                     MakeTraceSourceAccessor (&CouwbatPhy::m_phyTxBeginTrace))
    .AddTraceSource ("PhyTxEnd",
                     "Trace source indicating a packet has been completely transmitted over the channel. NOTE: the only official CouwbatPhy implementation available to this date (YansCouwbatPhy) never fires this trace source.",
                     MakeTraceSourceAccessor (&CouwbatPhy::m_phyTxEndTrace))
    .AddTraceSource ("PhyTxDrop",
                     "Trace source indicating a packet has been dropped by the device during transmission",
                     MakeTraceSourceAccessor (&CouwbatPhy::m_phyTxDropTrace))
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet has begun being received from the channel medium by the device",
                     MakeTraceSourceAccessor (&CouwbatPhy::m_phyRxBeginTrace))
    .AddTraceSource ("PhyRxEnd",
                     "Trace source indicating a packet has been completely received from the channel medium by the device",
                     MakeTraceSourceAccessor (&CouwbatPhy::m_phyRxEndTrace))
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&CouwbatPhy::m_phyRxDropTrace))

    .SetGroupName ("Couwbat")
  ;
  return tid;
}

CouwbatPhy::CouwbatPhy ()
{
  NS_LOG_FUNCTION (this);
}

CouwbatPhy::~CouwbatPhy ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t
CouwbatPhy::GetPlcpHeaderDurationMicroSeconds (CouwbatMode payloadMode)
{
  switch (payloadMode.GetModulationClass ())
    {
    case COUWBAT_MOD_CLASS_OFDM:
      {
        return 4;
      }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return 0;
    }
}

uint32_t
CouwbatPhy::GetPlcpPreambleDurationMicroSeconds (CouwbatMode payloadMode)
{
  switch (payloadMode.GetModulationClass ())
    {
    case COUWBAT_MOD_CLASS_OFDM:
      {
		return 8;
      }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return 0;
    }
}

double
CouwbatPhy::BitsPerSymbol (enum CouwbatMCS mcs)
{
  double bits_per_symb = 0;
  switch (mcs)
  {
    case COUWBAT_MCS_BPSK_1_2: // TODO could be removed
      bits_per_symb = 0.5;
      break;
    case COUWBAT_MCS_QPSK_1_2:
      bits_per_symb = 1;
      break;
    case COUWBAT_MCS_QPSK_3_4:
      bits_per_symb = 1.5;
      break;
    case COUWBAT_MCS_16QAM_1_2:
      bits_per_symb = 2;
      break;
    case COUWBAT_MCS_16QAM_3_4:
      bits_per_symb = 3;
      break;
    case COUWBAT_MCS_64QAM_1_2:
      bits_per_symb = 3;
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
    case COUWBAT_MCS_256QAM_3_4:
      bits_per_symb = 6;
      break;
    case COUWBAT_MCS_256QAM_5_6:
      bits_per_symb = 5*8/6;
      break;

    default:
      bits_per_symb = 1; // COUWBAT_MCS_QPSK_1_2
      break;
  }
  return bits_per_symb;
}

double
CouwbatPhy::GetPlcpSymbols (CouwbatTxVector txvector)
{
  CouwbatMode payloadMode = txvector.GetMode();
  double plcpMicroSeconds = GetPlcpPreambleDurationMicroSeconds (payloadMode);
	    //+ GetPlcpHeaderDurationMicroSeconds (payloadMode);
  double plcpSymbols = plcpMicroSeconds / Couwbat::GetSymbolDuration ();
  return plcpSymbols;
}

uint32_t
CouwbatPhy::SymbolsToBytes (uint32_t num_symbols, CouwbatTxVector txvector)
{
  CouwbatMode payloadMode = txvector.GetMode();

  NS_LOG_FUNCTION (num_symbols << payloadMode);

  switch (payloadMode.GetModulationClass ())
    {
    case COUWBAT_MOD_CLASS_OFDM:
      {
	double plcpSymbols = GetPlcpSymbols (txvector);
	double total_bits_per_symb = 0;

	for (uint32_t i = 0; i < payloadMode.GetSubchannels().size(); ++i)
	  {
	    total_bits_per_symb += (CouwbatPhy::BitsPerSymbol (payloadMode.GetMCS ()[i])
        * Couwbat::GetNumberOfDataSubcarriersPerSubchannel());
	  }

	double total_bytes_per_symb = total_bits_per_symb / 8;

	uint32_t transmittable_bytes = floor((num_symbols - plcpSymbols) * total_bytes_per_symb);

	NS_LOG_FUNCTION (transmittable_bytes);
	return transmittable_bytes;
      }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return 0;
    }
}

double
CouwbatPhy::GetPayloadDurationMicroSeconds (uint32_t size, CouwbatTxVector txvector)
{
  CouwbatMode payloadMode=txvector.GetMode();

  NS_LOG_FUNCTION (size << payloadMode);

  switch (payloadMode.GetModulationClass ())
    {
    case COUWBAT_MOD_CLASS_OFDM:
      {
        double bits_per_symb = 0;
        for (uint32_t i = 0; i < payloadMode.GetSubchannels().size(); ++i)
          {
            bits_per_symb += (CouwbatPhy::BitsPerSymbol (payloadMode.GetMCS ()[i]) * Couwbat::GetNumberOfDataSubcarriersPerSubchannel());
          }

        // size is in bytes
        uint32_t size_bits = size * 8;

        // number of OFDM symbols depends on the MCS, number of subcarriers per subchannel
        // and total number of used subchannels
        uint32_t num_symbols = ceil(size_bits / bits_per_symb);

        uint32_t payload_duration_mus = Couwbat::GetSymbolDuration() * num_symbols;

        NS_LOG_FUNCTION (payload_duration_mus);

        return payload_duration_mus;
      }
    default:
      NS_FATAL_ERROR ("unsupported modulation class");
      return 0;
    }
}

Time
CouwbatPhy::CalculateTxDuration (uint32_t size, CouwbatTxVector txvector)
{
  CouwbatMode payloadMode=txvector.GetMode();
  double duration = GetPlcpPreambleDurationMicroSeconds (payloadMode)
    //+ GetPlcpHeaderDurationMicroSeconds (payloadMode)
    + GetPayloadDurationMicroSeconds (size, txvector);
  return MicroSeconds (duration);
}



void
CouwbatPhy::NotifyTxBegin (Ptr<const Packet> packet)
{
  m_phyTxBeginTrace (packet);
}

void
CouwbatPhy::NotifyTxEnd (Ptr<const Packet> packet)
{
  m_phyTxEndTrace (packet);
}

void
CouwbatPhy::NotifyTxDrop (Ptr<const Packet> packet)
{
  m_phyTxDropTrace (packet);
}

void
CouwbatPhy::NotifyRxBegin (Ptr<const Packet> packet)
{
  m_phyRxBeginTrace (packet);
}

void
CouwbatPhy::NotifyRxEnd (Ptr<const Packet> packet)
{
  m_phyRxEndTrace (packet);
}

void
CouwbatPhy::NotifyRxDrop (Ptr<const Packet> packet)
{
  m_phyRxDropTrace (packet);
}


std::ostream& operator<< (std::ostream& os, enum CouwbatPhy::State state)
{
  switch (state)
    {
    case CouwbatPhy::IDLE:
      return (os << "IDLE");
    case CouwbatPhy::TX:
      return (os << "TX");
    case CouwbatPhy::RX:
      return (os << "RX");
    case CouwbatPhy::SWITCHING:
      return (os << "SWITCHING");
    default:
      NS_FATAL_ERROR ("Invalid CouwbatPhy state");
      return (os << "INVALID");
    }
}
} // namespace ns3
