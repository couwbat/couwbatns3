/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "couwbat.h"
#include "ns3/log.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("Couwbat");

namespace ns3 {

/* **************************
 * default values
 * ************************** */

// General PHY related parameters
uint32_t Couwbat::center_freq = 1320000000; //!< The center frequency of NC-OFDM in Hz
uint32_t Couwbat::bandwidth = 512000000; //!< in Hz. -> TODOD should be 528MHz
uint32_t Couwbat::fft_size = 2048;
uint32_t Couwbat::fft_duration_mus = ((double)fft_size)/bandwidth * std::pow(static_cast<double>(10),6); //!< in musec. // TODO delete
uint32_t Couwbat::sc_frequency_spacing = bandwidth/fft_size; //!< in Hz.
uint32_t Couwbat::symbol_duration_mus = ((double)fft_size)/bandwidth * std::pow(static_cast<double>(10),6); //!< in musec.
//uint32_t Couwbat::cp_duration_mus = 0; //!< in musec.
uint32_t Couwbat::symbols_preamble = 2;

uint32_t Couwbat::n_sc = fft_size; //!< The total number of subcarriers.
uint32_t Couwbat::n_sc_per_sch = 32; //!< The number of subcarrier per subchannel.
uint32_t Couwbat::n_dsc_per_sch = 24; //4 pilots, 4 guards
uint32_t Couwbat::n_gsc_per_sch = 4; // 4 guards
uint32_t Couwbat::sch0_center_freq = center_freq-bandwidth/2+n_sc_per_sch/2*sc_frequency_spacing;

double Couwbat::tx_power_BS_lin = 100; // 100=20dBm

// MAC related parameters; see TR
uint32_t Couwbat::superframe_duration_mus = 10000; // 10 ms
// TODO different guards???
uint32_t Couwbat::superframe_guard_symbols = 2;
//uint32_t Couwbat::control_phase_duration_mus = 2000; // 2 ms // TODO delete
//uint32_t Couwbat::data_phase_duration_mus; //!< in mus. // TODO delete
uint32_t Couwbat::narrowband_phase_duration_mus = 2000; // 2ms // TODO delete
uint32_t Couwbat::wideband_phase_duration_mus = 8000; // 8ms // TODO delete
uint32_t Couwbat::contention_slot_count = 4;
uint32_t Couwbat::sta_max_assoc_tries = 12;
bool Couwbat::sta_assoc_backoff = true;
uint32_t Couwbat::sta_max_backoff_delay = 16;
uint32_t Couwbat::aloha_nr_guard_symbols = 2; // also used for guard between data phase frames
uint32_t Couwbat::sta_max_nr_missed_pss = 5;
bool Couwbat::enable_backup_subchannels = false;
uint32_t Couwbat::backup_subch_count = 4;
double Couwbat::data_phase_downlink_portion = 0.8;
// only for Hardware
enum CouwbatMCS Couwbat::default_mcs = COUWBAT_MCS_QPSK_1_2;

Callback<void, std::vector<double>, std::vector<double>, double > Couwbat::sinrPerSubchannelCallback = MakeNullCallback<void, std::vector<double>, std::vector<double>, double > ();
bool Couwbat::m_sinrPerSubchannelCallbackIdEnabled = false;
uint32_t Couwbat::m_sinrPerSubchannelCallbackNodeId = 0;
bool Couwbat::m_sinrPerSubchannelCallbackWideband = true;
bool Couwbat::m_sinrPerSubchannelCallbackEnableMacFilter = false;
Mac48Address Couwbat::m_sinrPerSubchannelCallbackMacFilterAddress = Mac48Address ();

bool Couwbat::mac_queue_limit_enabled = false;
int Couwbat::mac_queue_limit_size = 2000;

bool Couwbat::mac_queue_prio_enabled = true;
double Couwbat::mac_queue_prio_ratio = 0.8;
int Couwbat::mac_queue_prio_ratio_count = 10;
unsigned int Couwbat::mac_queue_prio_size_threshold = 100;

unsigned int Couwbat::mac_dlul_slot_limit_size = 500;

bool Couwbat::mac_avoid_low_cqi_wb_subchannels = true;
double Couwbat::mac_against_threshold_avoid_low_cqi_wb_subchannels = 0.4;
uint8_t Couwbat::mac_below_value_avoid_low_cqi_wb_subchannels = 3;

/* **************************
 * methods
 * ************************** */

// setter

void
Couwbat::SetCenterFreq (uint32_t new_center_freq)
{
    NS_LOG_FUNCTION (new_center_freq);
	center_freq = new_center_freq;
}

void
Couwbat::SetFFTDuration (uint32_t new_fft_duration_mus)
{
	NS_LOG_FUNCTION (new_fft_duration_mus);
	fft_duration_mus = new_fft_duration_mus;
}

void
Couwbat::SetSCFrequencySpacing (uint32_t new_sc_frequency_spacing)
{
	NS_LOG_FUNCTION (new_sc_frequency_spacing);
	sc_frequency_spacing = new_sc_frequency_spacing;
}

void
Couwbat::SetSymbolDuration (uint32_t new_symbol_duration_mus)
{
	NS_LOG_FUNCTION (new_symbol_duration_mus);
	symbol_duration_mus = new_symbol_duration_mus;
}

// remove
/*void
Couwbat::SetCPDuration (uint32_t new_cp_duration_mus)
{
	NS_LOG_FUNCTION (new_cp_duration_mus);
	cp_duration_mus = new_cp_duration_mus;
}*/

void
Couwbat::SetNumberOfSubcarriers (uint32_t new_n_sc)
{
	NS_LOG_FUNCTION (new_n_sc);
	n_sc = new_n_sc;
}

void
Couwbat::SetNumberOfSubcarriersPerSubchannel (uint32_t new_n_sc_per_sch)
{
	NS_LOG_FUNCTION (new_n_sc_per_sch);
	n_sc_per_sch = new_n_sc_per_sch;
}

void
Couwbat::SetNumberOfDataSubcarriersPerSubchannel (uint32_t new_n_dsc_per_sch)
{
	NS_LOG_FUNCTION (new_n_dsc_per_sch);
	n_dsc_per_sch = new_n_dsc_per_sch;
}

void
Couwbat::SetNumberOfGuardSubcarriersPerSubchannel (uint32_t new_n_gsc_per_sch)
{
	NS_LOG_FUNCTION (new_n_gsc_per_sch);
	n_gsc_per_sch = new_n_gsc_per_sch;
}

void
Couwbat::SetTxPowerBS (double new_tx_power_BS_lin)
{
	NS_LOG_FUNCTION (new_tx_power_BS_lin);
	tx_power_BS_lin = new_tx_power_BS_lin;
}

void
Couwbat::SetSuperframeDuration (uint32_t new_superframe_duration_mus)
{
	NS_LOG_FUNCTION (new_superframe_duration_mus);
	superframe_duration_mus = new_superframe_duration_mus;
}

void
Couwbat::SetNarrowbandPhaseDuration (uint32_t new_narrowband_phase_duration_mus)
{
	NS_LOG_FUNCTION (new_narrowband_phase_duration_mus);
	narrowband_phase_duration_mus = new_narrowband_phase_duration_mus;
}

void
Couwbat::SetWidebandPhaseDuration (uint32_t new_wideband_phase_duration_mus)
{
	NS_LOG_FUNCTION (new_wideband_phase_duration_mus);
	wideband_phase_duration_mus = new_wideband_phase_duration_mus;
}

// getter


uint32_t
Couwbat::GetCenterFreq (void)
{
	return center_freq;
}

uint32_t
Couwbat::GetFFTDuration (void)
{
	return fft_duration_mus;
}

uint32_t
Couwbat::GetSCFrequencySpacing (void)
{
	return sc_frequency_spacing;
}

uint32_t
Couwbat::GetSymbolDuration (void)
{
	return symbol_duration_mus;
}

// remove
/*
uint32_t
Couwbat::GetCPDuration (void)
{
	return cp_duration_mus;
}
*/

uint32_t
Couwbat::GetNumberOfSubcarriers (void)
{
	return n_sc;
}

uint32_t
Couwbat::GetNumberOfSubchannels (void)
{
	return MAX_SUBCHANS;
}

uint32_t
Couwbat::GetNumberOfSubcarriersPerSubchannel (void)
{
	return n_sc_per_sch;
}

uint32_t
Couwbat::GetNumberOfDataSubcarriersPerSubchannel (void)
{
	return n_dsc_per_sch;
}

uint32_t
Couwbat::GetNumberOfGuardSubcarriersPerSubchannel (void)
{
	return n_gsc_per_sch;
}

double
Couwbat::GetTxPowerBS (void)
{
	return tx_power_BS_lin;
}

uint32_t
Couwbat::GetSuperframeDuration (void)
{
	return superframe_duration_mus;
}

uint32_t
Couwbat::GetNarrowbandPhaseDuration (void)
{
	return narrowband_phase_duration_mus;
}

uint32_t
Couwbat::GetWidebandPhaseDuration (void)
{
	return wideband_phase_duration_mus;
}

uint32_t
Couwbat::GetContentionSlotCount (void)
{
  return contention_slot_count;
}

void
Couwbat::SetContentionSlotCount (uint32_t n)
{
  contention_slot_count = n;
}

uint32_t
Couwbat::GetStaMaxAssocTries (void)
{
  return sta_max_assoc_tries;
}

void
Couwbat::SetStaMaxAssocTries (uint32_t n)
{
  sta_max_assoc_tries = n;
}

bool
Couwbat::StaAssocBackoff (void)
{
  return sta_assoc_backoff;
}

void
Couwbat::SetStaAssocBackoff (bool val)
{
  sta_assoc_backoff = val;
}

uint32_t
Couwbat::GetSch0CenterFreq (void)
{
  return sch0_center_freq;
}

void
Couwbat::SetSch0CenterFreq (uint32_t n)
{
  sch0_center_freq = n;
}

uint32_t
Couwbat::GetStaMaxBackoffDelay (void)
{
  return sta_max_backoff_delay;
}

void
Couwbat::SetStaMaxBackoffDelay (uint32_t n)
{
  sta_max_backoff_delay = n;
}

uint32_t
Couwbat::GetAlohaNrGuardSymbols (void)
{
  return aloha_nr_guard_symbols;
}

void
Couwbat::SetAlohaNrGuardSymbols (uint32_t n)
{
  aloha_nr_guard_symbols = n;
}

uint32_t
Couwbat::GetStaMaxNrMissedPss (void)
{
  return sta_max_nr_missed_pss;
}

void
Couwbat::SetStaMaxNrMissedPss (uint32_t n)
{
  sta_max_nr_missed_pss = n;
}

bool
Couwbat::EnableBackupSubchannels (void)
{
  return enable_backup_subchannels;
}

void
Couwbat::SetEnableBackupSubchannels (bool n)
{
  enable_backup_subchannels = n;
}

uint32_t
Couwbat::GetBackupSubchCount (void)
{
  return backup_subch_count;
}

void
Couwbat::SetBackupSubchCount (uint32_t n)
{
  backup_subch_count = n;
}

uint32_t
Couwbat::GetSuperframeGuardSymbols (void)
{
  return superframe_guard_symbols;
}

void
Couwbat::SetSuperframeGuardSymbols (uint32_t n)
{
  superframe_guard_symbols = n;
}

double
Couwbat::GetDataPhaseDownlinkPortion (void)
{
  return data_phase_downlink_portion;
}

void
Couwbat::SetDataPhaseDownlinkPortion (double n)
{
  data_phase_downlink_portion = n;
}

enum CouwbatMCS
Couwbat::GetDefaultMcs (void)
{
  return default_mcs;
}

void
Couwbat::SetDefaultMcs (enum CouwbatMCS mcs)
{
  default_mcs = mcs;
}

uint32_t
Couwbat::GetSymbolspreamble (void)
{
  return symbols_preamble;
}

} // namespace ns3

