#ifndef SIMPLE_COUWBAT_PHY_H
#define SIMPLE_COUWBAT_PHY_H

#include "couwbat-phy.h"
#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "couwbat-mode.h"
//#include "wifi-phy-standard.h"
#include "couwbat-intf-helper.h"

namespace ns3
{

class SimpleCouwbatChannel;
class CouwbatPhyStateHelper;

/**
 * \brief Couwbat PHY layer model
 * \ingroup couwbat
 *
 * This PHY implements a simple model of couwbat.
 * The model implemented here is based on the model
 * described in the Couwbat model library.
 *
 * This class is expected to be used in tandem with the
 * ns3::SimpleCouwbatChannel class.
 * By default no properties are set, so it is the callers responsibility
 * to set them before using the channel.
 * 
 * Original code base taken from corresponding class in ns3 Wi-Fi module.
 *
 */
class SimpleCouwbatPhy : public CouwbatPhy
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  SimpleCouwbatPhy (); //!< Default constructor
  virtual ~SimpleCouwbatPhy (); //!< Destructor

  /**
   * Set the device this PHY is associated with.
   *
   * \param device The device this PHY is associated with.
   */
  void SetDevice (Ptr<Object> device);

  /**
   * Return the device this PHY is associated with.
   *
   * \return The device this PHY is associated with.
   */
  Ptr<Object> GetDevice (void) const;

  /**
   * Set the SimpleCouwbatChannel this SimpleCouwbatPhy is to be connected to.
   *
   * \param channel The SimpleCouwbatChannel this SimpleCouwbatPhy is to be connected to.
   */
  void SetChannel (Ptr<SimpleCouwbatChannel> channel);

  /**
   * Get the SimpleCouwbatChannel this SimpleCouwbatPhy is connected to.
   *
   * \return The SimpleCouwbatChannel this SimpleCouwbatPhy is connected to.
   */
  virtual Ptr<CouwbatChannel> GetChannel (void) const;

  /**
   * Starting receiving the packet (i.e. the first bit of the preamble has arrived).
   *
   * \param packet the arriving packet
   * \param rxPowerDbm the receive power in dBm for each available subchannel
   * \param txVector the TXVECTOR of the arriving packet
   */
  void StartReceivePacket (Ptr<Packet> packet,
                           std::vector<double> rxPowerDbm,
                           CouwbatTxVector txVector);

  /**
   * Sets the RX loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver.
   *
   * \param noiseFigureDb noise figure in dB
   */
  void SetRxNoiseFigure (double noiseFigureDb);
  /**
   * Sets the minimum available transmission power level (dBm).
   *
   * \param start the minimum transmission power level (dBm)
   */
  void SetTxPowerStart (double start);
  /**
   * Sets the maximum available transmission power level (dBm).
   *
   * \param end the maximum transmission power level (dBm)
   */
  void SetTxPowerEnd (double end);
  /**
   * Sets the number of transmission power levels available between the
   * minimum level and the maximum level.  Transmission power levels are
   * equally separated (in dBm) with the minimum and the maximum included.
   *
   * \param n the number of available levels
   */
  void SetNTxPower (uint32_t n);
  /**
   * Sets the transmission gain (dB).
   *
   * \param gain the transmission gain in dB
   */
  void SetTxGain (double gain);
  /**
   * Sets the reception gain (dB).
   *
   * \param gain the reception gain in dB
   */
  void SetRxGain (double gain);
  /**
   * Sets the energy detection threshold (dBm).
   * The energy of a received signal should be higher than
   * this threshold (dbm) to allow the PHY layer to detect the signal.
   *
   * \param threshold the energy detction threshold in dBm
   */
  void SetEdThreshold (double threshold);
  /**
   * Sets the CCA threshold (dBm).  The energy of a received signal
   * should be higher than this threshold to allow the PHY
   * layer to declare CCA BUSY state.
   *
   * \param threshold the CCA threshold in dBm
   */
  void SetCcaMode1Threshold (double threshold);
  /**
   * Sets the error rate model.
   *
   * \param rate the error rate model
   */
  void SetErrorRateModel (const std::vector<Ptr<CouwbatErrorRateModel> > &rate);
  /**
   * Sets the mobility model.
   *
   * \param mobility The mobility model this PHY is associated with.
   */
  void SetMobility (Ptr<Object> mobility);
  /**
   * Return the RX noise figure (dBm).
   *
   * \return the RX noise figure in dBm
   */
  double GetRxNoiseFigure (void) const;
  /**
   * Return the transmission gain (dB).
   *
   * \return the transmission gain in dB
   */
  double GetTxGain (void) const;
  /**
   * Return the reception gain (dB).
   *
   * \return the reception gain in dB
   */
  double GetRxGain (void) const;
  /**
   * Return the energy detection threshold (dBm).
   *
   * \return the energy detection threshold in dBm
   */
  double GetEdThreshold (void) const;
  /**
   * Return the CCA threshold (dBm).
   *
   * \return the CCA threshold in dBm
   */
  double GetCcaMode1Threshold (void) const;
  /**
   * Return the error rate model this PHY is using.
   *
   * \return the error rate model this PHY is using
   */
  std::vector<Ptr<CouwbatErrorRateModel> > GetErrorRateModel (void) const;
  /**
   * Return the mobility model this PHY is associated with.
   *
   * \return The mobility model this PHY is associated with.
   */
  Ptr<Object> GetMobility (void);

  /**
   * Return the minimum available transmission power level (dBm).
   * \return the minimum available transmission power level (dBm)
   */
  virtual double GetTxPowerStart (void) const;
  /**
   * Return the maximum available transmission power level (dBm).
   * \return the maximum available transmission power level (dBm)
   */
  virtual double GetTxPowerEnd (void) const;
  /**
   * Return the number of available transmission power levels.
   *
   * \return the number of available transmission power levels
   */
  virtual uint32_t GetNTxPower (void) const;
  virtual void SetReceiveOkCallback (CouwbatPhy::RxOkCallback callback);
  virtual void SetReceiveErrorCallback (CouwbatPhy::RxErrorCallback callback);
  virtual void SendPacket (Ptr<Packet> packet);
  void SendPacketMh (Ptr<Packet> packet, CouwbatMode txMode, CouwbatTxVector txVector, bool metaheaderExists);
  virtual void RegisterListener (CouwbatPhyListener *listener);
  virtual bool IsStateIdle (void);
  virtual bool IsStateBusy (void);
  virtual bool IsStateRx (void);
  virtual bool IsStateTx (void);
  virtual bool IsStateSwitching (void);
  virtual Time GetStateDuration (void);
  virtual Time GetDelayUntilIdle (void);
  virtual Time GetLastRxStartTime (void) const;
//  virtual uint32_t GetNModes (void) const;
  virtual CouwbatMode GetMode (uint32_t mode) const;
  virtual double CalculateSnr (CouwbatMode txMode, double ber) const;

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

  /**
   * \param freq the operating frequency on this node (2.4 GHz or 5GHz).
   */
  virtual void SetFrequency (uint32_t freq);
  /**
   * \return the operating frequency on this node
   */
  virtual uint32_t GetFrequency (void) const;
  virtual uint32_t GetBssMembershipSelector (uint32_t selector) const;
//  virtual CouwbatModeList GetMembershipSelectorModes(uint32_t selector);
  /**
   * \return the number of MCS supported by this phy
   */
  virtual uint8_t GetNMcs (void) const;
  virtual uint8_t GetMcs (uint8_t mcs) const;

  //virtual uint32_t WifiModeToMcs (WifiMode mode);
  //virtual WifiMode McsToWifiMode (uint8_t mcs);

  //virtual void ConfigureStandard ();

  inline uint32_t GetSfCnt ()
  {
    return m_sfCnt;
  };

private:
  //YansCouwbatPhy (const YansCouwbatPhy &o);
  virtual void DoDispose (void);

  /**
   * Return the energy detection threshold.
   *
   * \return the energy detection threshold.
   */
  double GetEdThresholdW (void) const;
  /**
   * Convert from dBm to Watts.
   *
   * \param dbm the power in dBm
   * \return the equivalent Watts for the given dBm
   */
  double DbmToW (double dbm) const;
  /**
   * Convert from dB to ratio.
   *
   * \param db
   * \return ratio
   */
  double DbToRatio (double db) const;
  /**
   * Convert from Watts to dBm.
   *
   * \param w the power in Watts
   * \return the equivalent dBm for the given Watts
   */
  double WToDbm (double w) const;
  /**
   * Convert from ratio to dB.
   *
   * \param ratio
   * \return dB
   */
  double RatioToDb (double ratio) const;
  /**
   * Get the power of the given power level in dBm.
   * In YansCouwbatPhy implementation, the power levels are equally spaced (in dBm).
   *
   * \param power the power level
   * \return the transmission power in dBm at the given power level
   */
  double GetPowerDbm (uint8_t power) const;
  /**
   * The last bit of the packet has arrived.
   *
   * \param packet the packet that the last bit has arrived
   * \param event the corresponding event of the first time the packet arrives
   */
  void EndReceive (Ptr<Packet> packet, std::vector<Ptr<CouwbatInterferenceHelper::Event> > event);

  void SfTrigger (void);

protected:
  virtual void DoInitialize (void);

private:
  double   m_edThresholdW;        //!< Energy detection threshold in watts
  double   m_ccaMode1ThresholdW;  //!< Clear channel assessment (CCA) threshold in watts
  double   m_txGainDb;            //!< Transmission gain (dB)
  double   m_rxGainDb;            //!< Reception gain (dB)
  double   m_txPowerBaseDbm;      //!< Minimum transmission power (dBm)
  double   m_txPowerEndDbm;       //!< Maximum transmission power (dBm)
  uint32_t m_nTxPower;            //!< Number of available transmission power levels

  Ptr<SimpleCouwbatChannel> m_channel;        //!< YansCouwbatChannel that this YansCouwbatPhy is connected to
  //uint16_t             m_channelNumber;  //!< Operating channel number
  Ptr<Object>          m_device;         //!< Pointer to the device
  Ptr<Object>          m_mobility;       //!< Pointer to the mobility model

  CouwbatPhy::RxOkCallback m_rxOkCallback;
  CouwbatPhy::RxErrorCallback m_rxErrorCallback;


//  /**
//   * This vector holds the set of transmission modes that this
//   * CouwbatPhy(-derived class) can support. In conversation we call this
//   * the DeviceRateSet (not a term you'll find in the standard), and
//   * it is a superset of standard-defined parameters such as the
//   * OperationalRateSet, and the BSSBasicRateSet (which, themselves,
//   * have a superset/subset relationship).
//   *
//   * Mandatory rates relevant to this CouwbatPhy can be found by
//   * iterating over this vector looking for WifiMode objects for which
//   * WifiMode::IsMandatory() is true.
//   *
//   * A quick note is appropriate here (well, here is as good a place
//   * as any I can find)...
//   *
//   * In the standard there is no text that explicitly precludes
//   * production of a device that does not support some rates that are
//   * mandatory (according to the standard) for PHYs that the device
//   * happens to fully or partially support.
//   *
//   * This approach is taken by some devices which choose to only support,
//   * for example, 6 and 9 Mbps ERP-OFDM rates for cost and power
//   * consumption reasons (i.e., these devices don't need to be designed
//   * for and waste current on the increased linearity requirement of
//   * higher-order constellations when 6 and 9 Mbps more than meet their
//   * data requirements). The wording of the standard allows such devices
//   * to have an OperationalRateSet which includes 6 and 9 Mbps ERP-OFDM
//   * rates, despite 12 and 24 Mbps being "mandatory" rates for the
//   * ERP-OFDM PHY.
//   *
//   * Now this doesn't actually have any impact on code, yet. It is,
//   * however, something that we need to keep in mind for the
//   * future. Basically, the key point is that we can't be making
//   * assumptions like "the Operational Rate Set will contain all the
//   * mandatory rates".
//   */
//  CouwbatModeList m_deviceRateSet;

  std::vector<uint32_t> m_bssMembershipSelectorSet;
  std::vector<uint8_t> m_deviceMcsSet;
  EventId m_endRxEvent;
  uint32_t m_sfCnt;

  Ptr<UniformRandomVariable> m_random;  //!< Provides uniform random variables.
  double m_channelStartingFrequency;    //!< Standard-dependent center frequency of 0-th channel in MHz
  Ptr<CouwbatPhyStateHelper> m_state;      //!< Pointer to CouwbatPhyStateHelper
  std::vector<Ptr<CouwbatInterferenceHelper> > m_interference;    //!< Pointer to interference helper for each subchannel
  Time m_channelSwitchDelay;            //!< Time required to switch between channel

};

} // namespace ns3

#endif /* SIMPLE_COUWBAT_PHY_H */
