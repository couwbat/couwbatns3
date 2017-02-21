#ifndef STA_COUWBAT_MAC_H
#define STA_COUWBAT_MAC_H

#include <string>
#include "couwbat-mac.h"
#include "couwbat-tx-queue.h"
#include "couwbat-meta-header.h"
#include "couwbat-tx-history-buffer.h"
#include "couwbat-pss-header.h"

namespace ns3
{

/**
 * \ingroup couwbat
 * 
 * \brief Couwbat CR-STA state machine, the main component of the MAC layer.
 *
 * Handle the control PSS scanning, association request,
 * UL/DL map detection, association reply detection and data
 * transmission, as well as advanced features of the MAC protocol.
 */
class StaCouwbatMac : public CouwbatMac
{
private:
  /** \enum CrStaState
   * Available states for CR-STA state machine
   */
  enum CrStaState
  {
    CR_STA_INIT, //!< first CW_CMD_WIFI_EXTRA_ZERO_SF_START not yet received
    CR_STA_SCAN, //!< not associated, scanning for BSs
    CR_STA_ASSOC, //!< in the process of association
    CR_STA_OPERATING //!< normal, associated operation state
  };

  /** \struct ScannedPss
   * Struct for keeping the details of received PSS when scanning
   */
  struct ScannedPss
  {
    CouwbatMetaHeader mh; //!< Metaheader of received frame, contains PHY configuration such as used subchannels
    Ptr<Packet> pss; //!< PSS frame contents
  };

public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  StaCouwbatMac (); //!< Default constructor
  virtual ~StaCouwbatMac (); //!< Destructor

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
   * Initialize class
   */
  virtual void DoInitialize (void);

private:
  /**
   * Change the state m_state
   * \param newState change to this state
   */
  void SetState (const CrStaState newState);

  /**
   * Disassociates STA (if applicable) and starts a full scan for PSS on all subchannels.
   */
  void StartScanning ();
  
  /**
   * Helper function for StartScanning().
   */
  void ScanRxPss (uint32_t subchannel);

  /**
   * Select a CR-BS for association after a scan has been completed based on m_scannedPss
   * and initiate association to this BS.
   */
  void SelectCrBs (void);

  /**
   * Check FCS and destination address. Return true if FCS check fails or packet is not for us (by MAC address).
   * Removes FCS tailer.
   */
  bool FilterPacket (Ptr<Packet> packet);

  /**
   * The callback for all received packets from below.
   * Handles all cases of packet receipt and ignores any erroneous packets.
   */
  void RxOk (Ptr<Packet>);
  void RxOkHandleZeroSfStart (CouwbatMetaHeader mh, Ptr<Packet> packet); //!< Handling of specific RxOk cases, logically a part of RxOk
  void RxOkHandleExtraRx (CouwbatMetaHeader mh, Ptr<Packet> packet); //!< Handling of specific RxOk cases, logically a part of RxOk
  void RxSavePssDetails (CouwbatMetaHeader mh, Ptr<Packet> packet); //!< Handling of specific RxOk cases, logically a part of RxOk
  void RxProcessPss (CouwbatMetaHeader mh, Ptr<Packet> packet); //!< Handling of specific RxOk cases, logically a part of RxOk
  void RxProcessMap (CouwbatMetaHeader mh, Ptr<Packet> packet); //!< Handling of specific RxOk cases, logically a part of RxOk
  void RxProcessData (CouwbatMetaHeader mh, Ptr<Packet> packet); //!< Handling of specific RxOk cases, logically a part of RxOk

  /**
   * Send an association frame to currently selected BS.
   */
  void SendAssoc ();
  
  /**
   * Send an SF_START to PHY if in m_netlinkMode
   * 
   * \param mh Metaheader specifying the current superframe that is being processed.
   *    This is always the SF_START metaheader received from PHY.
   */
  void SendSfStart (const CouwbatMetaHeader &mh);

  /**
   * Send to phy (netlink or CouwbatPhy)
   * 
   * \param mh Metaheader specifying parameters of the data packet.
   * \param packet Packet content that will be transmitted
   */
  void Send (CouwbatMetaHeader mh, Ptr<Packet> packet, std::string message = "");

  /*
   * --------------------------------
   * - MEMBER VARIABLES
   * --------------------------------
   */

  /**
   * Callback of this class, called externally to forward up packets from below to MAC.
   */
  Callback<void, Ptr<Packet> > m_rxOkCallback;
  
  /**
   * Callback of Couwbat netlink module which is called to send packets down.
   */
  Callback<int, CouwbatMetaHeader, Ptr<Packet> > m_cwnlSendCallback;

  /*
   * Current state and extra state information
   */
  CrStaState m_state; //!< The current state of STA
  bool m_xstate_pss_rx_scheduled[2]; //!< PSS receipt has been scheduled, extra state information for last 2 superframes.
  bool m_xstate_map_rx_scheduled[2]; //!< MAP receipt has been scheduled, extra state information for last 2 superframes.

  /*
   * Variables for scanning
   */
  std::vector<ScannedPss> m_scannedPss; //!< a list of all detected PSS during last full scan
  uint32_t m_currentScanCcId; //!< state information used during scan, current control channel ID
  uint32_t m_currentScanEndedSfCnt; //!< state information used during scan, sfcount at which the current scan ends

  /**
   * True if STA is currently associated
   */
  bool m_associated;
  uint32_t m_ccId; //!< Current control channel ID used by BS.
  bool m_useBackupCc; //!< True if m_backupCc is valid and can be used
  CouwbatPssHeader::BackupChannel_t m_backupCc; //!< Current backup CC information.

  /**
   * MAC superframe counter, in sync and determined by with PHY superframe counter
   */
  uint32_t m_sfCnt;
  std::vector<uint8_t> m_lastSeq; //!< SEQ of last successfully received DL frame(s)
  std::vector<std::vector<uint8_t> > m_lastCqi; //!< CQI of last received DL frame(s)

  /**
   * The address of the BS with which STA is currently associated
   * or attempting to associate with,
   */
  Mac48Address m_currentBsAddr;

  int32_t m_pssTimeout; //!< State variable used to timeout in case of multiple unreceived PSS

  /**
   * TX queue
   */
  CouwbatTxQueue m_txQueue;

  /**
   * Fixed packet size in bytes for PSS, set during initialization
   */
  uint32_t m_pssSizeBytes;
  
  /**
   * Fixed packet size in bytes for association frame, set during initialization
   */
  uint32_t m_assocSizeBytes;

  /*
   * SEQ/ACK
   */
  uint8_t m_seq; //!< SEQ number to be used for next DL packet
  CouwbatTxHistoryBuffer m_txHistory; //!< Storage for sent DL upper layer payload packets
};

} // namespace ns3

#endif /* STA_COUWBAT_MAC_H */
