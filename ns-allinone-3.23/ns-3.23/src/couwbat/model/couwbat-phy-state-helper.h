#ifndef COUWBAT_PHY_STATE_HELPER_H
#define COUWBAT_PHY_STATE_HELPER_H

#include "couwbat-phy.h"
#include "ns3/traced-callback.h"
#include "ns3/object.h"
#include <vector>
#include <queue>
#include "simple-couwbat-phy.h"
#include "couwbat-meta-header.h"

namespace ns3 {

/**
 * \ingroup couwbat
 *
 * This objects implements the PHY state machine of SimpleCouwbatPhy. Original code base taken from corresponding class in ns3 Wi-Fi module.
 */
class CouwbatPhyStateHelper : public Object
{
public:
  static TypeId GetTypeId (void);

  CouwbatPhyStateHelper ();

  /**
   * Set a callback for a successful reception.
   *
   * \param callback
   */
  void SetReceiveOkCallback (CouwbatPhy::RxOkCallback callback);
  /**
   * Set a callback for a failed reception.
   *
   * \param callback
   */
  void SetReceiveErrorCallback (CouwbatPhy::RxErrorCallback callback);
  /**
   * Register CouwbatPhyListener to this CouwbatPhyStateHelper.
   *
   * \param listener
   */
  void RegisterListener (CouwbatPhyListener *listener);
  /**
   * Return the current state of CouwbatPhy.
   *
   * \return the current state of CouwbatPhy
   */
  enum CouwbatPhy::State GetState (void);
  /**
   * Check whether the current state is IDLE.
   *
   * \return true if the current state is IDLE, false otherwise
   */
  bool IsStateIdle (void);
  /**
   * Check whether the current state is not IDLE.
   *
   * \return true if the current state is not IDLE, false otherwise
   */
  bool IsStateBusy (void);
  /**
   * Check whether the current state is RX.
   *
   * \return true if the current state is RX, false otherwise
   */
  bool IsStateRx (void);
  /**
   * Check whether the current state is TX.
   *
   * \return true if the current state is TX, false otherwise
   */
  bool IsStateTx (void);
  /**
   * Check whether the current state is SWITCHING.
   *
   * \return true if the current state is SWITCHING, false otherwise
   */
  bool IsStateSwitching (void);
  /**
   * Return the elapsed time of the current state.
   *
   * \return the elapsed time of the current state
   */
  Time GetStateDuration (void);
  /**
   * Return the time before the state is back to IDLE.
   *
   * \return the delay before the state is back to IDLE
   */
  Time GetDelayUntilIdle (void);
  /**
   * Return the time the last RX start.
   *
   * \return the time the last RX start.
   */
  Time GetLastRxStartTime (void) const;

  /**
   * Switch state to TX for the given duration.
   *
   * \param txDuration the duration of the TX
   * \param packet the packet
   * \param txMode the transmission mode of the packet
   * \param preamble the preamble of the packet
   * \param txPower the transmission power
   */
  void SwitchToTx (Time txDuration, Ptr<const Packet> packet, CouwbatMode txMode, uint8_t txPower);
  /**
   * Switch state to RX for the given duration.
   *
   * \param rxDuration the duration of the RX
   */
  void SwitchToRx (Time rxDuration);
  /**
   * Switch from RX after the reception was successful.
   *
   * \param packet the successfully received packet
   * \param snr the SNR of the received packet
   * \param mode the transmission mode of the packet
   * \param preamble the preamble of the received packet
   */
  void SwitchFromRxEndOk (Ptr<Packet> packet, std::vector<double> snr, CouwbatMode mode);
  /**
   * Switch from RX after the reception failed.
   *
   * \param packet the packet that we failed to received
   * \param snr the SNR of the received packet
   */
  void SwitchFromRxEndError (Ptr<const Packet> packet, std::vector<double> snr);

  TracedCallback<Time,Time,enum CouwbatPhy::State> m_stateLogger;

  void SetPhy (Ptr<SimpleCouwbatPhy> phy);

  void EnqueueRx (CouwbatMetaHeader mh);
private:
  /**
   * typedef for a list of CouwbatPhyListeners
   */
  typedef std::vector<CouwbatPhyListener *> Listeners;

  /**
   * Notify all CouwbatPhyListener that the transmission has started for the given duration.
   *
   * \param duration the duration of the transmission
   */
  void NotifyTxStart (Time duration);
  //void NotifyWakeup (void);
  /**
   * Notify all CouwbatPhyListener that the reception has started for the given duration.
   *
   * \param duration the duration of the reception
   */
  void NotifyRxStart (Time duration);
  /**
   * Notify all CouwbatPhyListener that the reception was successful.
   */
  void NotifyRxEndOk (void);
  /**
   * Notify all CouwbatPhyListener that the reception was not successful.
   */
  void NotifyRxEndError (void);
  /**
   * Switch the state from RX.
   */
  void DoSwitchFromRx (void);

  void CheckScheduleRxStatus (CouwbatMetaHeader mh);

  void ForwardUpDummy (CouwbatMetaHeader mh, uint32_t sizeBytes);

  uint32_t MhGetBytes (CouwbatMetaHeader mh);

  bool m_rxing;
  Time m_endTx;
  Time m_endRx;
  Time m_endCcaBusy;
  Time m_endSwitching;
  Time m_startTx;
  Time m_startRx;
  Time m_startCcaBusy;
  Time m_startSwitching;
  Time m_previousStateChangeTime;

  Listeners m_listeners;
  TracedCallback<Ptr<const Packet> > m_rxOkTrace;
  TracedCallback<Ptr<const Packet> > m_rxErrorTrace;
  TracedCallback<Ptr<const Packet> > m_txTrace;
  CouwbatPhy::RxOkCallback m_rxOkCallback;
  CouwbatPhy::RxErrorCallback m_rxErrorCallback;

  Ptr<SimpleCouwbatPhy> m_phy;

  struct pshRxQueue
  {
    CouwbatMetaHeader mh;
    EventId e;
  };
  
  /**
   * Strict FIFO RX scheduling queue.
   * Correct order of events in queue is essential to avoid breakage!
   */
  std::queue<pshRxQueue> m_rxQueue;
};

} // namespace ns3

#endif /* COUWBAT_PHY_STATE_HELPER_H */
