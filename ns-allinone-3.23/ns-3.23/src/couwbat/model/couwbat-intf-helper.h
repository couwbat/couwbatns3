#ifndef COUWBAT_INTERFERENCE_HELPER_H
#define COUWBAT_INTERFERENCE_HELPER_H

#include <stdint.h>
#include <vector>
#include <list>
#include "couwbat-mode.h"
//#include "wifi-phy-standard.h"
#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"
#include "couwbat-tx-vector.h"
#include "couwbat-err-rate-model.h"

namespace ns3 {

/**
 * \ingroup couwbat
 * \brief Handles interference calculations. Original code base taken from corresponding class in ns3 Wi-Fi module.
 */
class CouwbatInterferenceHelper : public Object
{
public:
  /**
   * Signal event for a packet.
   */
  class Event : public SimpleRefCount<CouwbatInterferenceHelper::Event>
  {
public:
    /**
     * Create an Event with the given parameters.
     *
     * \param size packet size
     * \param payloadMode Wi-Fi mode used for the payload
     * \param duration duration of the signal
     * \param rxPower the receive power (w)
     * \param txvector TXVECTOR of the packet
     */
    Event (uint32_t size, CouwbatMode payloadMode,
           Time duration, double rxPower, CouwbatTxVector txvector,
           uint32_t mcsIndex);
    ~Event ();

    /**
     * Return the duration of the signal.
     *
     * \return the duration of the signal
     */
    Time GetDuration (void) const;
    /**
     * Return the start time of the signal.
     *
     * \return the start time of the signal
     */
    Time GetStartTime (void) const;
    /**
     * Return the end time of the signal.
     *
     * \return the end time of the signal
     */
    Time GetEndTime (void) const;
    /**
     * Return the receive power (w).
     *
     * \return the receive power (w)
     */
    double GetRxPowerW (void) const;
    /**
     * Return the size of the packet (bytes).
     *
     * \return the size of the packet (bytes)
     */
    uint32_t GetSize (void) const;
    /**
     * Return the Wi-Fi mode used for the payload.
     *
     * \return the Wi-Fi mode used for the payload
     */
    CouwbatMode GetPayloadMode (void) const;
    /**
     * Return the TXVECTOR of the packet.
     *
     * \return the TXVECTOR of the packet
     */
    CouwbatTxVector GetTxVector (void) const;

    uint32_t GetMcsIndex (void) const;
private:
    uint32_t m_size;
    CouwbatMode m_payloadMode;
    Time m_startTime;
    Time m_endTime;
    double m_rxPowerW;
    CouwbatTxVector m_txVector;
    uint32_t m_mcsIndex;
  };
  /**
   * A struct for both SNR and PER
   */
  struct SnrPer
  {
    double minSnr;
    double maxSnr;
    double per;
  };

  CouwbatInterferenceHelper ();
  ~CouwbatInterferenceHelper ();

  static TypeId GetTypeId (void);

  /**
   * Set the noise figure.
   *
   * \param value noise figure
   */
  void SetNoiseFigure (double value);
  /**
   * Set the error rate model for this interference helper.
   *
   * \param rate Error rate model
   */
  void SetErrorRateModel (Ptr<CouwbatErrorRateModel> rate);

  /**
   * Return the noise figure.
   *
   * \return the noise figure
   */
  double GetNoiseFigure (void) const;
  /**
   * Return the error rate model.
   *
   * \return Error rate model
   */
  Ptr<CouwbatErrorRateModel> GetErrorRateModel (void) const;


  /**
   * \param energyW the minimum energy (W) requested
   * \returns the expected amount of time the observed
   *          energy on the medium will be higher than
   *          the requested threshold.
   */
  Time GetEnergyDuration (double energyW);

  /**
   * Add the packet-related signal to interference helper.
   *
   * \param size packet size
   * \param payloadMode Wi-Fi mode for the payload
   * \param preamble Wi-Fi preamble for the packet
   * \param duration the duration of the signal
   * \param rxPower receive power (w)
   * \param txvector TXVECTOR of the packet
   * \return CouwbatInterferenceHelper::Event
   */
  Ptr<CouwbatInterferenceHelper::Event> Add (uint32_t size, CouwbatMode payloadMode, uint32_t mcsIndex,
                                      Time duration, double rxPower, CouwbatTxVector txvector);

  /**
   * Calculate the SNIR at the start of the packet and accumulate
   * all SNIR changes in the snir vector.
   *
   * \param event the event corresponding to the first time the packet arrives
   * \return struct of SNR and PER
   */
  struct CouwbatInterferenceHelper::SnrPer CalculateSnrPer (Ptr<CouwbatInterferenceHelper::Event> event);
  /**
   * Notify that RX has started.
   */
  void NotifyRxStart ();
  /**
   * Notify that RX has ended.
   */
  void NotifyRxEnd ();
  /**
   * Erase all events.
   */
  void EraseEvents (void);
private:
  /**
   * Noise and Interference (thus Ni) event.
   */
  class NiChange
  {
public:
    /**
     * Create a NiChange at the given time and the amount of NI change.
     *
     * \param time time of the event
     * \param delta the power
     */
    NiChange (Time time, double delta);
    /**
     * Return the event time.
     *
     * \return the event time.
     */
    Time GetTime (void) const;
    /**
     * Return the power
     *
     * \return the power
     */
    double GetDelta (void) const;
    /**
     * Compare the event time of two NiChange objects (a < o).
     *
     * \param o
     * \return true if a < o.time, false otherwise
     */
    bool operator < (const NiChange& o) const;
private:
    Time m_time;
    double m_delta;
  };
  /**
   * typedef for a vector of NiChanges
   */
  typedef std::vector <NiChange> NiChanges;
  /**
   * typedef for a list of Events
   */
  typedef std::list<Ptr<Event> > Events;

  //CouwbatInterferenceHelper (const CouwbatInterferenceHelper &o);
  //CouwbatInterferenceHelper &operator = (const CouwbatInterferenceHelper &o);
  /**
   * Append the given Event.
   *
   * \param event
   */
  void AppendEvent (Ptr<Event> event);
  /**
   * Calculate noise and interference power in W.
   *
   * \param event
   * \param ni
   * \return noise and interference power
   */
  double CalculateNoiseInterferenceW (Ptr<Event> event, NiChanges *ni) const;
  /**
   * Calculate SNR (linear ratio) from the given signal power and noise+interference power.
   * (Mode is not currently used)
   *
   * \param signal
   * \param noiseInterference
   * \param mode
   * \return SNR in liear ratio
   */
  double CalculateSnr (double signal, double noiseInterference, CouwbatMode mode) const;
  /**
   * Calculate the success rate of the chunk given the SINR, duration, and Wi-Fi mode.
   * The duration and mode are used to calculate how many bits are present in the chunk.
   *
   * \param snir SINR
   * \param duration
   * \param mode
   * \return the success rate
   */
  double CalculateChunkSuccessRate (double snir, Time duration, CouwbatMode mode, enum CouwbatMCS mcs) const;
  /**
   * Calculate the error rate of the given packet. The packet can be divided into
   * multiple chunks (e.g. due to interference from other transmissions).
   *
   * \param event
   * \param ni
   * \return the error rate of the packet
   */
  double CalculatePer (Ptr<const Event> event, NiChanges *ni, SnrPer *snrPer) const;

  double m_noiseFigure; /**< noise figure (linear) */
  Ptr<CouwbatErrorRateModel> m_errorRateModel;
  /// Experimental: needed for energy duration calculation
  NiChanges m_niChanges;
  double m_firstPower;
  bool m_rxing;
  /// Returns an iterator to the first nichange, which is later than moment
  NiChanges::iterator GetPosition (Time moment);
  /**
   * Add NiChange to the list at the appropriate position.
   *
   * \param change
   */
  void AddNiChangeEvent (NiChange change);
};

} // namespace ns3

#endif /* COUWBAT_INTERFERENCE_HELPER_H */
