#ifndef BS_COUWBAT_MAC_H
#define BS_COUWBAT_MAC_H

#include "couwbat-mac.h"
#include "couwbat-tx-queue.h"
#include "couwbat-meta-header.h"
#include "couwbat-packet-helper.h"
#include "couwbat-tx-history-buffer.h"
#include <set>
#include <bitset>
#include <map>
#include <deque>

namespace ns3
{

class SpectrumManager;

/**
 * \ingroup couwbat
 * 
 * \brief Couwbat CR-BS state machine, the main component of the MAC layer.
 *
 * Implements almost all features of the CR-BS MAC layer.
 * This includes basics such as control channel selection, PSS, contention phase
 * UL/DL mapping, association and disassociation of CR-STAs and
 * data transmission, as well as advanced features of the MAC protocol such as MCS adaptation based on CQI feedback.
 */
class BsCouwbatMac : public CouwbatMac
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  BsCouwbatMac (); //!< Default constructor
  ~BsCouwbatMac (); //!< Destructor

  /**
   * Return link state used in CouwbatNetDevice. Currently always returns true.
   * \return true
   */
  bool IsLinkUp (void) const;

  /**
   * Takes packets for transmission from upper layers and adds them to TxQueue.
   * \param packet packet to enqueue
   * \param to destination MAC address
   */
  void Enqueue (Ptr<Packet> packet, Mac48Address to);

  /**
   * Return the current number of packets in the TxQueue for a certain destination MAC address.
   * \param dest destination address
   */
  int GetTxQueueSize (const Mac48Address dest);

  /**
   * Set the spectrum manager member m_specManager explicitly. Only used in m_netlinkMode.
   */
  void SetSpectrumManager (Ptr<SpectrumManager> sm);

  /**
   * Getter for m_rxOkCallback. Used in m_netlinkMode to forward up packets from lower layers to MAC.
   */
  Callback<void,Ptr<Packet> > GetRxOkCallback ();

  /**
   * Set Couwbat Netlink Send Callback member m_cwnlSendCallback.
   * Used in m_netlinkMode for sending packets to lower layers.
   */
  void SetCwnlSendCallback (Callback<int, CouwbatMetaHeader, Ptr<Packet> > cb);

protected:
  /**
   * Connects the MAC with the spectrum manager of its device,
   * sets the initial state and starts operation of the BsCouwbatMac.
   */
  void DoInitialize (void);

private:
  /**
   * Find new ccId. Update m_allocatedNbSubChannels, m_ccSelected, m_ccId.
   * Return true if ccId found.
   */
  bool SelectNewCcNoBackup (void);

  /**
   * Find new backup CC and update m_backupCc
   */
  void SelectBackupCc (void);

  /**
   * Update m_allocatedWbSubChannels with all currently free
   * subchannels.
   * Return true if at least one channel is available.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  bool UpdateWidebandDetails (const CouwbatMetaHeader &mh);

  /**
   * Select a new control channel. Updates all relevant member variables.
   */
  bool SelectCc (void);

  /**
   * If a control channel is selected and free, initiates a superframe.
   * If not initiates CC selection.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void StartSuperframe (const CouwbatMetaHeader &mh);

  /**
   * Send the MAP.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void SendMap (const CouwbatMetaHeader &mh);

  /**
   * Send downlink frames.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void SendDl (const CouwbatMetaHeader &mh);

  /**
   * Receive uplink frames.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void RecvUl (const CouwbatMetaHeader &mh);

  /**
   * Send an SF_START to PHY if in m_netlinkMode
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void SendSfStart (const CouwbatMetaHeader &mh);

  /**
   *  Send the PSS
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void SendPss (const CouwbatMetaHeader &mh);

  /**
   * Receive association frames.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void RecvAssoc (const CouwbatMetaHeader &mh);

  /**
   * Send to phy (netlink or CouwbatPhy)
   * 
   * \param mh Metaheader specifying parameters of the data packet.
   * \param packet Packet content that will be transmitted
   */
  void Send (CouwbatMetaHeader mh, Ptr<Packet> packet);

  /**
   * Returns true if this packet is invalid and should be disregarded.
   */
  bool CheckControlPacket (Ptr<Packet> packet);

  /**
   * Callback function on successful receipt of data from below. Called by PHY.
   * Main handler for all incoming packets.
   */
  void RxOk (Ptr<Packet> packet);

  /**
   * Subfunction of RxOk.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void RxOkHandleExtraRx (const CouwbatMetaHeader &mh, Ptr<Packet> packet);

  /**
   * Subfunction of RxOk.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void RxOkHandleZeroSfStart (const CouwbatMetaHeader &mh, Ptr<Packet> packet);

  /**
   * Adds a STA with this address to m_newStas. This STA is not yet actually associated.
   * It becomes associated when it is removed from m_newStas and added to m_associatedStas.
   */
  void AddSta (Mac48Address addr);

  /**
   * Return the optimal MCS vector for a particular desination STA.
   * Uses the current set of allocated subchannels and the CQI history entries for this STA, if available.
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   * \param dest destination address
   * \param allocatedSubchannels subchannels that will be used
   */
  std::vector<CouwbatMCS> GetOptimalMcs (const Mac48Address &dest, std::bitset<Couwbat::MAX_SUBCHANS> allocatedSubchannels, const CouwbatMetaHeader &mh);

  /** \struct MapLengthRetType
   * A struct to hold the return values of GetMapLength.
   */
  struct MapLengthRetType {
    unsigned int ulDlSlotsPerSta; //!< Number of uplink and downlink slots that each STA will have
    double wasted; //!< Percentage of wasted symbols from total number of available symbols
    unsigned int mapSymbols; //!< Size of MAP in symbols
    unsigned int mapSizeBytes; //!< Size of MAP in bytes
    unsigned int mapPaddingBytes; //!< Number of padding bytes for MAP
  };
  
  /**
   * Calculate an optimal bandwidth allocation based on the number of STAs,
   * number of used subchannels, number of total available wideband symbols
   * and the target MCS for the data bursts to each STA
   * 
   * \param stas number of STAs
   * \param subchannels number of subchannels
   * \param symbWideband number of total available wideband symbols
   * \param mcs vector of the target MCS vector for each STA. Number of elements must be equal to number of STAs.
   * \return results in MapLengthRetType struct
   */
  MapLengthRetType GetMapLength (unsigned int stas, unsigned int subchannels, unsigned int symbWideband, const std::vector<std::vector<CouwbatMCS> > &mcs);

  /**
   * Add an entry to the CQI history.
   * All blank arrays cqi[i] == 255 are skipped and not added.
   * 
   * \param source the source address
   * \param cqi array of CQI values
   * \param sframe_count sframe count to which this entry pertains
   */
  void AddCqiHist (Mac48Address source, uint8_t cqi[], uint32_t sframe_count);

  /**
   * Delete older CQI history entries.
   * Based on a cutoff age where all entries that are older than (sframe_count - sfCutoffDifference) are deleted.
   * 
   * \param sframe_count current sframe count
   */
  void CleanCqiHist (uint32_t sframe_count);

  /*
   * --------------------------------
   * - MEMBER VARIABLES
   * --------------------------------
   */

  /**
   * Direct access to the spectrum manager of the device.
   */
  Ptr<SpectrumManager> m_specManager;

  /**
   * Callback of this class, called externally to forward up packets from below to MAC.
   */
  Callback<void, Ptr<Packet> > m_rxOkCallback;
  
  /**
   * Callback of Couwbat netlink module which is called to send packets down.
   */
  Callback<int, CouwbatMetaHeader, Ptr<Packet> > m_cwnlSendCallback;

  /**
   * true if a control channel is selected.
   */
  bool m_ccSelected;

  /**
   * The selected control channel number, only valid if m_isCcSelected is true.
   */
  uint32_t m_ccId[3];
  
  /**
   * Current backup CC information.
   */
  CouwbatPssHeader::BackupChannel_t m_backupCc;

  /**
   * Variables for PHY mode, currently allocated narrowband subchannels
   */
  std::bitset<Couwbat::MAX_SUBCHANS> m_allocatedNbSubChannels[3];
  /**
   * Variables for PHY mode, currently allocated wideband subchannels
   */
  std::bitset<Couwbat::MAX_SUBCHANS> m_allocatedWbSubChannels[3];

  /**
   * List of all associated STAs
   *
   * m_associatedStas[0] is STAs in current (n-0) Superframe
   * m_associatedStas[1] is STAs in previous (n-1) Superframe
   * m_associatedStas[2] is STAs in (n-2) Superframe
   */
  std::vector<Mac48Address> m_associatedStas[3];

  /**
   * Temporary storage for associated STAs during change to backup CC.
   * Will be readded to m_associatedStas[0] in n+3rd superframe if the
   * backup CC change occurred in nth superframe
   */
  std::vector<Mac48Address> m_associatedStasBackupCcTemp;

  /**
   * Countdown for restoring m_associatedStasBackupCcTemp
   */
  int32_t m_restoreBackupCcTempStasTimeout;

  /**
   * New STAs are first added to this list on assoc receipt. In the beginning of the next SF,
   * the contents of this list is moved to m_associatedStas[0] and m_newStas cleared
   */
  std::vector<Mac48Address> m_newStas;

  /**
   * TX queue instance, holds all packet pointers to packets in the transmission queue
   */
  CouwbatTxQueue m_txQueue;

  /**
   * Time of last control channel change, purely for statistics.
   */
  Time m_lastCcChange;

  /**
   * Fixed packet size in bytes for PSS, set during initialization
   */
  uint32_t m_pssSizeBytes;
  
  /**
   * Fixed packet size in bytes for association frame, set during initialization
   */
  uint32_t m_assocSizeBytes;
  
  /**
   * Storage for the dynamically changing number of padding bytes in MAP for the last 3 superframes.
   * 
   * [0] for current Superframe
   * [n] for current Superframe - n
   */
  uint32_t m_mapPaddingBytes[3];
  uint32_t m_mapSizeSymbols[3]; //!< Storage for MAP size in symbols. Analogous to m_mapPaddingBytes
  uint32_t m_mapUlDlSlotsPerSta[3]; //!< Storage for number of downlink/uplink slots per STA. Analogous to m_mapPaddingBytes

  uint32_t m_wbSubchannelCnt[3]; //!< Storage for the dynamically changing number of used number of wideband subchannels for the last 3 superframes.

  CouwbatPssHeader m_pssHistory[3]; //!< Storage for parameters of the PSS frames that were sent in the last 3 superframes.

  std::vector<std::vector<CouwbatMCS> > m_staDataMcs[2]; //!< Storage for MCS vectors used for each STA in the last 2 superframes.

  /**
   * Storage for downlink packets sent in last 2 MAPs
   * 
   * Map subpackets are saved to [0] by SendMap()
   * Map subpackets are read from [1] by SendDl() to access scheduling made in last Superframe
   *
   * [0] for current Superframe (n)
   * [1] for previous Superframe (n-1)
   */
  std::vector<CouwbatMapSubpacket> m_downlinkMapSubpacketHistory[2];
  std::vector<CouwbatMapSubpacket> m_uplinkMapSubpacketHistory[2]; //!< Storage for uplink packets sent in last 2 MAPs. Analogous to m_downlinkMapSubpacketHistory.

  /*
   * SEQ/ACK
   */
  uint8_t m_seq; //!< SEQ number counter for sending DL packets
  
  /**
   * Storage for sent payload packets from upper layers
   */
  CouwbatTxHistoryBuffer m_txHistory;

  std::map <Mac48Address, std::vector<uint8_t> > m_lastSeq; //!< SEQ of last successfully received DL frame(s)

  /*
   * CQI history
   */
  
  /** \typedef cqiHistEntry_t
   * CQI history entry
   */
  typedef struct
    {
      Mac48Address src;
      std::vector<uint8_t> cqi;
      uint32_t sframe_count;
    } cqiHistEntry_t;
    
  /** \typedef staCqiHist_t
   * List of CQI history entries cqiHistEntry_t
   */
  typedef std::deque<cqiHistEntry_t> staCqiHist_t;

  std::map<Mac48Address, staCqiHist_t> m_cqiHist; //!< Storage of all CQI history entries by source address
};

} // namespace ns3

#endif /* BS_COUWBAT_MAC_H */
