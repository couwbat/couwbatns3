#ifndef COUWBAT_H
#define COUWBAT_H

#include <stdint.h>
#include <vector>
#include "ns3/callback.h"
#include "ns3/mac48-address.h"
#include "couwbat-mode.h"

namespace ns3 {

/**
 * \brief Manages configuration parameters for the whole COUWBAT module.
 * \ingroup couwbat
 *
 * The configuration parameters managed here are collected in a central static
 * place, to allow access from several other classes that are inpendent from each
 * other. This central location makes sure that they only have to be set once and then
 * all users can rely on the same parameters.
 */
class Couwbat
{
public:

  // setter

  static void SetCenterFreq (uint32_t n);
  static void SetFFTDuration (uint32_t n);
  static void SetSCFrequencySpacing (uint32_t n);
  static void SetSymbolDuration (uint32_t n);
  // remove
  //static void SetCPDuration (uint32_t n);

  /**
   * Set the number of subcarriers.
   * \param n The number of subcarriers.
   */
  static void SetNumberOfSubcarriers (uint32_t n);

  /**
   * Set the number of subcarriers per subchannel.
   * \param n The number of subcarrier per subchannel.
   */
  static void SetNumberOfSubcarriersPerSubchannel (uint32_t n);

  /**
   * Set the number of data subcarriers per subchannel.
   * \param n The number of data subcarriers per subchannel.
   */
  static void SetNumberOfDataSubcarriersPerSubchannel (uint32_t n);

  static void SetNumberOfGuardSubcarriersPerSubchannel (uint32_t n);

  static void SetTxPowerBS (double n);
  static void SetSuperframeDuration (uint32_t n);
  static void SetNarrowbandPhaseDuration (uint32_t n);
  static void SetWidebandPhaseDuration (uint32_t n);

  // getter

  static uint32_t GetCenterFreq (void);
  static uint32_t GetFFTDuration (void);
  static uint32_t GetSCFrequencySpacing (void);
  static uint32_t GetSymbolDuration (void);
  // remove
  //static uint32_t GetCPDuration (void);

  /**
   * Return the number of subcarriers.
   * \return The number of subcarriers.
   */
  static uint32_t GetNumberOfSubcarriers (void);

  static uint32_t GetNumberOfSubchannels (void);

  /**
   * Return the number of subcarriers per subchannel.
   * \return The number of subcarriers per subchannel.
   */
  static uint32_t GetNumberOfSubcarriersPerSubchannel (void);

  static uint32_t GetNumberOfDataSubcarriersPerSubchannel (void);

  static uint32_t GetNumberOfGuardSubcarriersPerSubchannel (void);

  static double GetTxPowerBS (void);
  static uint32_t GetSuperframeDuration (void);
  static uint32_t GetNarrowbandPhaseDuration (void);
  static uint32_t GetWidebandPhaseDuration (void);

  static uint32_t GetContentionSlotCount (void);
  static void SetContentionSlotCount (uint32_t n);

  static uint32_t GetStaMaxAssocTries (void);
  static void SetStaMaxAssocTries (uint32_t n);

  static bool StaAssocBackoff (void);
  static void SetStaAssocBackoff (bool value);

  static uint32_t GetSch0CenterFreq (void);
  static void SetSch0CenterFreq (uint32_t n);

  static uint32_t GetStaMaxBackoffDelay (void);
  static void SetStaMaxBackoffDelay (uint32_t n);

  static uint32_t GetAlohaNrGuardSymbols (void);
  static void SetAlohaNrGuardSymbols (uint32_t n);

  static uint32_t GetStaMaxNrMissedPss (void);
  static void SetStaMaxNrMissedPss (uint32_t n);

  static bool EnableBackupSubchannels (void);
  static void SetEnableBackupSubchannels (bool n);

  static uint32_t GetBackupSubchCount (void);
  static void SetBackupSubchCount (uint32_t n);

  static uint32_t GetSuperframeGuardSymbols (void);
  static void SetSuperframeGuardSymbols (uint32_t n);

  static double GetDataPhaseDownlinkPortion (void);
  static void SetDataPhaseDownlinkPortion (double n);

  static enum CouwbatMCS GetDefaultMcs (void);
  static void SetDefaultMcs (enum CouwbatMCS mcs);

  static uint32_t GetSymbolspreamble(void);

  static const uint32_t MAX_SUBCHANS = 64; //!< Number of subchannels used in Couwbat

  static const uint32_t STA_PSS_TIMEOUT_SF = 2; //!< Timeout after this many superframes without received PSS in CR-STA

private:

  ////
  // PHY related parameters
  static uint32_t center_freq; //!< The center frequency of NC-OFDM
  static uint32_t bandwidth;
  static uint32_t fft_size;
  static uint32_t fft_duration_mus; //!< in musec.
  static uint32_t sc_frequency_spacing; //!< in Hz.
  static uint32_t symbol_duration_mus; //!< in musec.
  //static uint32_t cp_duration_mus; //!< in musec.
  static uint32_t sch0_center_freq; // center frequency of first subchannel

  static uint32_t n_sc; //!< The total number of subcarriers.
  static uint32_t n_sc_per_sch; //!< The number of subcarrier per subchannel.
  static uint32_t n_dsc_per_sch; //!< The number of data subcarriers per subchannel.
  static uint32_t n_gsc_per_sch; //!< The number of guards per subchannel.
  static double tx_power_BS_lin; //!< The tx power of the BS in linear units

  static uint32_t symbols_preamble;

  ////
  // MAC related parameters; see TR
  static uint32_t superframe_duration_mus; //!< in mus.
  static uint32_t superframe_guard_symbols; // between MAP and data
  //static uint32_t control_phase_duration_mus; //!< in mus.
  //static uint32_t data_phase_duration_mus; //!< in mus.
  static uint32_t narrowband_phase_duration_mus; //!< in mus.
  static uint32_t wideband_phase_duration_mus; //!< in mus.
  static uint32_t contention_slot_count;
  static uint32_t sta_max_assoc_tries;
  static bool sta_assoc_backoff;
  static uint32_t sta_max_backoff_delay;
  static uint32_t aloha_nr_guard_symbols;
  static uint32_t sta_max_nr_missed_pss;
  static bool enable_backup_subchannels;
  static uint32_t backup_subch_count;
  static double data_phase_downlink_portion;
  static enum CouwbatMCS default_mcs;

public:
  /**
   * Misc MAC parameters and settings
   */
  static bool mac_queue_limit_enabled; //!< If true, enable the limit for CouwbatTxQueue size to a number of packets. Does not play well with ns3 applications. 
  static int mac_queue_limit_size; //!< Limit CouwbatTxQueue size to a number of packets. Only valid if mac_queue_limit_enabled is true.

  /**
   * CouwbatTxQueue priority for smaller packets
   *
   * Example: mac_queue_prio_ratio = 0.8; mac_queue_prio_ratio_count = 10;
   *    => queue returns in this order: 8 priority, 2 nonpriority, 8 priority,...
   */
  static bool mac_queue_prio_enabled; //!< Enable or disable functionality
  static double mac_queue_prio_ratio; //!< Percentage (0.0 < mac_queue_prio_ratio < 1.0) of returned packets that are with priority vs nonpriority
  static int mac_queue_prio_ratio_count; //!< The number of packets to base the percentage/ratio on.
  static unsigned int mac_queue_prio_size_threshold; //!< If packet size in bytes is lesser or equal to this number, the packet is put in priority queue, else nonpriority queue

  static unsigned int mac_dlul_slot_limit_size; //!< Max number of DL/UL slots per superframe (set artificial limit)

  /**
   * Avoid unoccupied but low CQI wideband subchannels according to STA feedback
   * during CR-BS wideband channel selection on a superframe basis
   */
  static bool mac_avoid_low_cqi_wb_subchannels; //!< Enable or disable this feature
  
  static double mac_against_threshold_avoid_low_cqi_wb_subchannels; //!< If the ratio of negative feedback to number of all feedbacks exceeds this percentage, avoid subchannel
  
  static uint8_t mac_below_value_avoid_low_cqi_wb_subchannels; //!< CQIs below this value will be considered bad and to be potentially avoided, if percentage threshold is exceeded



  /**
   * Callbacks for stats
   */

  /**
   * Call this (if available) whenever a wideband
   * transmission happens (PHY or intf model). The argument
   * is the vector of SINRs for every subchannel.
   */
  static Callback<void, std::vector<double>, std::vector<double>, double > sinrPerSubchannelCallback;
  
  static bool m_sinrPerSubchannelCallbackIdEnabled; //!< True if only a specific Node should use callback
  
  static uint32_t m_sinrPerSubchannelCallbackNodeId; //!< ID of the node that should use this callback
  
  static bool m_sinrPerSubchannelCallbackWideband; //!< True if callback is to be invoked only on wideband packets
 
  static bool m_sinrPerSubchannelCallbackEnableMacFilter; //!< True if MAC address filter is enabled
 
  static Mac48Address m_sinrPerSubchannelCallbackMacFilterAddress; //!< Let only following MAC through
};

} // namespace ns3

#endif /* COUWBAT_H */

