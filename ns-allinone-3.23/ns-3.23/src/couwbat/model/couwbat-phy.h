#ifndef COUWBAT_PHY_H
#define COUWBAT_PHY_H

#include "ns3/core-module.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "couwbat-tx-vector.h"
#include "couwbat-mode.h"

namespace ns3
{

class CouwbatChannel;
class NetDevice;

/**
 * \brief Receive notifications about phy events.
 * 
 * Original code base taken from corresponding class in ns3 Wi-Fi module.
 */
class CouwbatPhyListener
{
public:
  virtual ~CouwbatPhyListener (void) = 0;

  /**
   * \param duration the expected duration of the packet reception.
   *
   * We have received the first bit of a packet. We decided
   * that we could synchronize on this packet. It does not mean
   * we will be able to successfully receive completely the
   * whole packet. It means that we will report a BUSY status until
   * one of the following happens:
   *   - NotifyRxEndOk
   *   - NotifyRxEndError
   *   - NotifyTxStart
   */
  virtual void NotifyRxStart (Time duration) = 0;
  
  /**
   * We have received the last bit of a packet for which
   * NotifyRxStart was invoked first and, the packet has
   * been successfully received.
   */
  virtual void NotifyRxEndOk (void) = 0;
  
  /**
   * We have received the last bit of a packet for which
   * NotifyRxStart was invoked first and, the packet has
   * _not_ been successfully received.
   */
  virtual void NotifyRxEndError (void) = 0;
  
  /**
   * \param duration the expected transmission duration.
   *
   * We are about to send the first bit of the packet.
   * We do not send any event to notify the end of
   * transmission. Listeners should assume that the
   * channel implicitely reverts to the idle state
   * unless they have received a cca busy report.
   */
  virtual void NotifyTxStart (Time duration) = 0;

  /**
   * \param duration the expected busy duration.
   *
   * This method does not really report a real state
   * change as opposed to the other methods in this class.
   * It merely reports that, unless the medium is reported
   * busy through NotifyTxStart or NotifyRxStart/End,
   * it will be busy as defined by the currently selected
   * CCA mode.
   *
   * Typical client code which wants to have a clear picture
   * of the CCA state will need to keep track of the time at
   * which the last NotifyCcaBusyStart method is called and
   * what duration it reported.
   */
  virtual void NotifyMaybeCcaBusyStart (Time duration) = 0;
  
  /**
   * \param duration the expected channel switching duration.
   *
   * We do not send any event to notify the end of
   * channel switching. Listeners should assume that the
   * channel implicitely reverts to the idle or busy states.
   */
  virtual void NotifySwitchingStart (Time duration) = 0;
};

/**
 * Similar to WiFi
 * \brief Couwbat PHY layer model.
 * \ingroup couwbat
 */
class CouwbatPhy : public Object
{
public:

  /**
   * The state of the PHY layer.
   */
  enum State
  {
	/**
	 * The PHY layer is IDLE.
	 */
	IDLE,
	/**
	 * The PHY layer is sending a packet.
	 */
	TX,
	/**
	 * The PHY layer is receiving a packet.
	 */
	RX,
	/**
	 * The PHY layer is switching to other channel. TODO do we need that?
	 */
	SWITCHING
  };

  /**
   * arg1: packet received successfully
   * arg2: snr of packet
   * arg3: mode of packet
   * arg4: type of preamble used for packet.
   */
  typedef Callback<void,Ptr<Packet> > RxOkCallback;
  
  /**
   * arg1: packet received unsuccessfully
   * arg2: snr of packet
   */
  typedef Callback<void,Ptr<const Packet> > RxErrorCallback;

  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  CouwbatPhy (); //!< Default constructor
  virtual ~CouwbatPhy ();//!< Destructor

  /**
   * Return the minimum available transmission power level (dBm).
   *
   * \return the minimum available transmission power level in dBm
   */
  virtual double GetTxPowerStart (void) const = 0;
  
  /**
   * Return the maximum available transmission power level (dBm).
   *
   * \return the maximum available transmission power level in dBm
   */
  virtual double GetTxPowerEnd (void) const = 0;
  
  /**
   * \return the number of tx power levels available for this PHY.
   */
  virtual uint32_t GetNTxPower (void) const = 0;

  /**
   * \param callback the callback to invoke
   *        upon successful packet reception.
   */
  virtual void SetReceiveOkCallback (RxOkCallback callback) = 0;
  
  /**
   * \param callback the callback to invoke
   *        upon erroneous packet reception.
   */
  virtual void SetReceiveErrorCallback (RxErrorCallback callback) = 0;

  /**
   * \param packet the packet to send
   * \param mode the transmission mode to use to send this packet
   * \param txvector the txvector that has tx parameters as txPowerLevel a power level to use to send this packet. The real
   *        transmission power is calculated as txPowerMin + txPowerLevel * (txPowerMax - txPowerMin) / nTxLevels
   */
  virtual void SendPacket (Ptr<Packet> packet) = 0;

  /**
   * \param listener the new listener
   *
   * Add the input listener to the list of objects to be notified of
   * PHY-level events.
   */
  virtual void RegisterListener (CouwbatPhyListener *listener) = 0;

  /**
   * \return true of the current state of the PHY layer is CouwbatPhy::IDLE, false otherwise.
   */
  virtual bool IsStateIdle (void) = 0;
  
  /**
   * \return true of the current state of the PHY layer is not CouwbatPhy::IDLE, false otherwise.
   */
  virtual bool IsStateBusy (void) = 0;
  
  /**
   * \return true of the current state of the PHY layer is CouwbatPhy::RX, false otherwise.
   */
  virtual bool IsStateRx (void) = 0;
  
  /**
   * \return true of the current state of the PHY layer is CouwbatPhy::TX, false otherwise.
   */
  virtual bool IsStateTx (void) = 0;
  
  /**
   * \return true of the current state of the PHY layer is CouwbatPhy::SWITCHING, false otherwise.
   */
  virtual bool IsStateSwitching (void) = 0;
  
  /**
   * \return the amount of time since the current state has started.
   */
  virtual Time GetStateDuration (void) = 0;
  
  /**
   * \return the predicted delay until this PHY can become CouwbatPhy::IDLE.
   *
   * The PHY will never become CouwbatPhy::IDLE _before_ the delay returned by
   * this method but it could become really idle later.
   */
  virtual Time GetDelayUntilIdle (void) = 0;

  /**
   * Return the start time of the last received packet.
   *
   * \return the start time of the last received packet
   */
  virtual Time GetLastRxStartTime (void) const = 0;

  static double BitsPerSymbol (enum CouwbatMCS);

  static double GetPlcpSymbols (CouwbatTxVector txvector);

  static uint32_t SymbolsToBytes (uint32_t num_symbols, CouwbatTxVector txvector);

  /**
   * \param size the number of bytes in the packet to send
   * \param txvector the transmission parameters used for this packet
   * \return the total amount of time this PHY will stay busy for
   *          the transmission of these bytes.
   */
  static Time CalculateTxDuration (uint32_t size, CouwbatTxVector txvector);

  /**
   * \param payloadMode the CouwbatMode use for the transmission of the payload
   * \param preamble the type of preamble
   *
   * \return the duration of the PLCP header in microseconds
   */
  static uint32_t GetPlcpHeaderDurationMicroSeconds (CouwbatMode payloadMode);

  /**
   * \param payloadMode the CouwbatMode use for the transmission of the payload
   * \param preamble the type of preamble
   *
   * \return the duration of the PLCP preamble in microseconds
   */
  static uint32_t GetPlcpPreambleDurationMicroSeconds (CouwbatMode payloadMode);

  /**
   * \param size the number of bytes in the packet to send
   * \param txvector the transmission parameters used for this packet
   *
   * \return the duration of the payload in microseconds
   */
  static double GetPayloadDurationMicroSeconds (uint32_t size, CouwbatTxVector txvector);

  /**
   * The CouwbatPhy::GetNModes() and CouwbatPhy::GetMode() methods are used
   * (e.g., by a WifiRemoteStationManager) to determine the set of
   * transmission/reception modes that this CouwbatPhy(-derived class)
   * can support - a set of CouwbatMode objects which we call the
   * DeviceRateSet, and which is stored as CouwbatPhy::m_deviceRateSet.
   *
   * It is important to note that the DeviceRateSet is a superset (not
   * necessarily proper) of the OperationalRateSet (which is
   * logically, if not actually, a property of the associated
   * WifiRemoteStationManager), which itself is a superset (again, not
   * necessarily proper) of the BSSBasicRateSet.
   *
   * \param mode index in array of supported modes
   * \return the mode whose index is specified.
   *
   * \sa CouwbatPhy::GetNModes()
   */
  virtual CouwbatMode GetMode (uint32_t mode) const = 0;
  
  /**
   * \param txMode the transmission mode
   * \param ber the probability of bit error rate
   * \return the minimum snr which is required to achieve
   *          the requested ber for the specified transmission mode. (W/W)
   */
  virtual double CalculateSnr (CouwbatMode txMode, double ber) const = 0;

  /**
   * Return the channel this phy is connected to.
   * \return The channel this Phy is connceted to.
   */
  virtual Ptr<CouwbatChannel> GetChannel (void) const = 0;

  /**
   * Public method used to fire a PhyTxBegin trace.  Implemented for encapsulation
   * purposes.
   *
   * \param packet the packet being transmitted
   */
  void NotifyTxBegin (Ptr<const Packet> packet);

  /**
   * Public method used to fire a PhyTxEnd trace.  Implemented for encapsulation
   * purposes.
   *
   * \param packet the packet that was transmitted
   */
  void NotifyTxEnd (Ptr<const Packet> packet);

  /**
   * Public method used to fire a PhyTxDrop trace.  Implemented for encapsulation
   * purposes.
   *
   * \param packet the packet that was failed to transmitted
   */
  void NotifyTxDrop (Ptr<const Packet> packet);

  /**
   * Public method used to fire a PhyRxBegin trace.  Implemented for encapsulation
   * purposes.
   *
   * \param packet the packet being received
   */
  void NotifyRxBegin (Ptr<const Packet> packet);

  /**
   * Public method used to fire a PhyRxEnd trace.  Implemented for encapsulation
   * purposes.
   *
   * \param packet the packet received
   */
  void NotifyRxEnd (Ptr<const Packet> packet);

  /**
   * Public method used to fire a PhyRxDrop trace.  Implemented for encapsulation
   * purposes.
   *
   * \param packet the packet that was not successfully received
   */
  void NotifyRxDrop (Ptr<const Packet> packet);

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  virtual int64_t AssignStreams (int64_t stream) = 0;

  /**
   * \param freq the operating frequency on this node.
   */
  virtual void SetFrequency (uint32_t freq)=0;
  
  /**
   * \return the operating frequency on this node
   */
  virtual uint32_t GetFrequency (void) const=0;

  //virtual void ConfigureStandard ();

private:
  /**
   * The trace source fired when a packet begins the transmission process on
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyTxBeginTrace;

  /**
   * The trace source fired when a packet ends the transmission process on
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyTxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet as it tries
   * to transmit it.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyTxDropTrace;

  /**
   * The trace source fired when a packet begins the reception process from
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxBeginTrace;

  /**
   * The trace source fired when a packet ends the reception process from
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet it has received.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;

};

} // namespace ns3
#endif /* COUWBAT_PHY_H */
