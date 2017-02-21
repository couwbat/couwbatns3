#ifndef SRC_COUWBAT_MODEL_COUWBAT_TX_HISTORY_BUFFER_H_
#define SRC_COUWBAT_MODEL_COUWBAT_TX_HISTORY_BUFFER_H_

#include "ns3/network-module.h"
#include "couwbat-tx-queue.h"
#include <map>
#include <vector>

namespace ns3
{

/**
 * \ingroup couwbat
 * 
 * Helper class for CR-BS and CR-STA for storing all recently sent
 * packets for potential retransmission.
 */
class CouwbatTxHistoryBuffer
{
public:

  typedef uint8_t SeqType;
  typedef std::vector<PacketType> PacketList;

  CouwbatTxHistoryBuffer ();
  ~CouwbatTxHistoryBuffer ();

  void SuperframeTick (CouwbatTxQueue &txQueue); //!< Called the MAC at the start of each superframe
  
  PacketList *GetNewList (Mac48Address dest, SeqType seq); //!< Get a new list for storing sent packets, added to during packet creation
  
  void RegisterAck (Mac48Address dest, SeqType seq); //!< Register a received ACK number as successfully received by recipient.

private:

  typedef std::map<SeqType, PacketList> PacketListBySeq;
  typedef std::map<Mac48Address, PacketListBySeq> TxHistory;

  void EnqueueRetransmissions (CouwbatTxQueue &txQueue);

  TxHistory *m_hist[5];

};

}

#endif /* SRC_COUWBAT_MODEL_COUWBAT_TX_HISTORY_BUFFER_H_ */
