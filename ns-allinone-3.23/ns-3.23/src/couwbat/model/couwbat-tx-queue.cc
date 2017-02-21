#include "couwbat-tx-queue.h"
#include "couwbat.h"
#include <stdexcept>
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("CouwbatTxQueue");

void
CouwbatTxQueue::Enqueue (const Mac48Address dest, PacketType packet)
{
  // Initialize queue containers if not yet created
  try
  {
    m_queues.at (dest);
    m_priorityQueues.at (dest);
  }
  catch (const std::out_of_range& oor)
  {
    m_queues[dest] = PacketQueueType ();
    m_priorityQueues[dest] = PacketQueueType ();
  }

  if (Couwbat::mac_queue_prio_enabled && packet->GetSize () <= Couwbat::mac_queue_prio_size_threshold)
    {
      NS_LOG_DEBUG (this << " TxQueue> enqueue prio, size="<<packet->GetSize ());
      m_priorityQueues.at (dest).push_back (packet);
    }
  else
    {
      NS_LOG_DEBUG (this << " TxQueue> enqueue nonprio, size="<<packet->GetSize ());
      m_queues.at (dest).push_back (packet);
    }
}

void
CouwbatTxQueue::Reenqueue (const Mac48Address dest, PacketType packet)
{
  // Initialize queue containers if not yet created
  try
  {
    m_queues.at (dest);
    m_priorityQueues.at (dest);
  }
  catch (const std::out_of_range& oor)
  {
    m_queues[dest] = PacketQueueType ();
    m_priorityQueues[dest] = PacketQueueType ();
  }

  if (Couwbat::mac_queue_prio_enabled && packet->GetSize () <= Couwbat::mac_queue_prio_size_threshold)
    {
      NS_LOG_DEBUG (this << " TxQueue> reenqueue prio, size="<<packet->GetSize ());
      m_priorityQueues.at (dest).push_back (packet);
      ++m_priorityReenqueueCount[dest];
    }
  else
    {
      NS_LOG_DEBUG (this << " TxQueue> reenqueue nonprio, size="<<packet->GetSize ());
      m_queues.at (dest).push_front (packet);
      ++m_reenqueueCount[dest];
    }
}

PacketType
CouwbatTxQueue::Peek (const Mac48Address dest)
{
  try
  {
    if (Couwbat::mac_queue_prio_enabled
        && (!m_priorityQueues.at (dest).empty ())
        && (m_priorityRatioCounter < Couwbat::mac_queue_prio_ratio_count * Couwbat::mac_queue_prio_ratio))
      {
        m_flagLastPeekPriority = true;
        NS_LOG_DEBUG (this << " TxQueue> peek prio standard" << ", m_priorityRatioCounter=" << m_priorityRatioCounter << ", addr=" << m_priorityQueues.at (dest).front ());
        return m_priorityQueues.at (dest).front ();
      }
    else
      {
        if (!m_queues.at (dest).empty ())
          {
            m_flagLastPeekPriority = false;
            NS_LOG_DEBUG (this << " TxQueue> peek nonprio" << ", m_priorityRatioCounter=" << m_priorityRatioCounter << ", addr=" << m_queues.at (dest).front ());
            return m_queues.at (dest).front ();
          }

        if (!m_priorityQueues.at (dest).empty ())
          {
            m_flagLastPeekPriority = true;
            NS_LOG_DEBUG (this << " TxQueue> peek prio, nonprio empty" << ", m_priorityRatioCounter=" << m_priorityRatioCounter << ", addr=" << m_priorityQueues.at (dest).front ());
            return m_priorityQueues.at (dest).front ();
          }
      }
  }
  catch (const std::out_of_range& oor)
  {
  }
  return PacketType ();
}

PacketType
CouwbatTxQueue::Pop (const Mac48Address dest, bool &retransmission)
{
  try
  {
    PacketType ret = Peek (dest);
    NS_LOG_DEBUG (this << " TxQueue> pop" << ", m_priorityRatioCounter=" << m_priorityRatioCounter << ", addr=" << ret
        << ", m_priorityQueues_size=" << m_priorityQueues[dest].size () << ", m_queues_size=" << m_queues[dest].size ());
    if (ret)
      {
        m_priorityRatioCounter = (m_priorityRatioCounter + 1) %  Couwbat::mac_queue_prio_ratio_count;
        if (m_flagLastPeekPriority)
          {
            m_priorityQueues.at (dest).pop_front ();
          }
        else
          {
            m_queues.at (dest).pop_front ();
          }
      }

    if (!m_flagLastPeekPriority && m_reenqueueCount[dest] > 0)
      {
        retransmission = true;
        --m_reenqueueCount[dest];
      }

    if (m_flagLastPeekPriority && m_priorityReenqueueCount[dest] > 0)
      {
        retransmission = true;
        --m_reenqueueCount[dest];
      }

    return ret;
  }
  catch (const std::out_of_range& oor)
  {
  }
  return PacketType ();
}

void
CouwbatTxQueue::Clear (const Mac48Address dest)
{
  try
  {
    m_reenqueueCount[dest] = 0;
    m_priorityReenqueueCount[dest] = 0;
    m_queues.at (dest).clear ();
    m_priorityQueues.at (dest).clear ();
  }
  catch (const std::out_of_range& oor)
  {
  }
}

void
CouwbatTxQueue::ClearAll ()
{
  m_queues.clear ();
  m_priorityQueues.clear ();
  m_reenqueueCount.clear ();
  m_priorityReenqueueCount.clear ();
}

int
CouwbatTxQueue::GetQueueSize (const Mac48Address dest)
{
  try
  {
    m_queues.at (dest);
    m_priorityQueues.at (dest);
  }
  catch (const std::out_of_range& oor)
  {
    return 0;
  }
  return m_queues.at (dest).size () + m_priorityQueues.at (dest).size ();
}

}
