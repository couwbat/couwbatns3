#ifndef SRC_COUWBAT_MODEL_COUWBAT_TX_QUEUE_H_
#define SRC_COUWBAT_MODEL_COUWBAT_TX_QUEUE_H_

#include "ns3/network-module.h"
#include <deque>
#include <map>

namespace ns3
{

typedef Ptr<Packet> PacketType;
typedef std::deque<PacketType> PacketQueueType;

/**
 * \ingroup couwbat
 * 
 * \brief A simple transmission queue for CR-BS and CR-STA.
 * 
 * Contains separate queues for every destination MAC address.
 * Furthermore, it has two queues per destination MAC: normal and high priority.
 * Priorities are assigned according to packet size, whereby smaller packets are placed
 * into the high priority queue and mostly sent before larger packets. This is done
 * to prevent TCP connections from choking by prioritizing smaller service packets (e.g. SYN/ACK pairs)
 * over larger data packets.
 * 
 * The parameters and thresholds for prioritization are configured in couwbat.h/.cc
 */
class CouwbatTxQueue
{
public:
  // TODO handle broadcast addr
  void Enqueue (const Mac48Address dest, PacketType packet);
  void Reenqueue (const Mac48Address dest, PacketType packet);
  PacketType Peek (const Mac48Address dest);
  PacketType Pop (const Mac48Address dest, bool &retransmission);
  void Clear (const Mac48Address dest);
  void ClearAll ();
  int GetQueueSize (const Mac48Address dest);

private:
  std::map<const Mac48Address, PacketQueueType> m_queues;
  std::map<const Mac48Address, PacketQueueType> m_priorityQueues;
  std::map<const Mac48Address, uint32_t> m_reenqueueCount;
  std::map<const Mac48Address, uint32_t> m_priorityReenqueueCount;
  int m_priorityRatioCounter;
  bool m_flagLastPeekPriority;
};

}

#endif /* SRC_COUWBAT_MODEL_COUWBAT_TX_QUEUE_H_ */
