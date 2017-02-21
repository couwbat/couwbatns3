#include "couwbat-tx-history-buffer.h"
#include "ns3/log.h"
#include <stdexcept>

NS_LOG_COMPONENT_DEFINE ("CouwbatTxHistoryBuffer");

namespace ns3
{

CouwbatTxHistoryBuffer::CouwbatTxHistoryBuffer ()
: m_hist ()
{
  NS_LOG_FUNCTION (this);
}

CouwbatTxHistoryBuffer::~CouwbatTxHistoryBuffer ()
{
  NS_LOG_FUNCTION (this);
  delete m_hist[0];
  delete m_hist[1];
  delete m_hist[2];
  delete m_hist[3];
  delete m_hist[4];
}

void
CouwbatTxHistoryBuffer::SuperframeTick (CouwbatTxQueue& txQueue)
{
  NS_LOG_FUNCTION (this);
  delete m_hist[4];
  m_hist[4] = m_hist[3];
  m_hist[3] = m_hist[2];
  m_hist[2] = m_hist[1];
  m_hist[1] = m_hist[0];
  m_hist[0] = new TxHistory ();

  EnqueueRetransmissions (txQueue);
}

CouwbatTxHistoryBuffer::PacketList*
CouwbatTxHistoryBuffer::GetNewList (Mac48Address dest, SeqType seq)
{
  NS_LOG_FUNCTION (this << dest << seq);
  TxHistory *hist = m_hist[0];
  return &(*hist)[dest][seq];
}

void
CouwbatTxHistoryBuffer::RegisterAck (Mac48Address dest, SeqType seq)
{
  NS_LOG_FUNCTION (this << dest << seq);
  TxHistory *hist = m_hist[3];
  try
    {
      hist->at (dest).at (seq);
    }
  catch (const std::out_of_range& oor)
    {
      return;
    }
  (*hist)[dest].erase (seq);
}

void
CouwbatTxHistoryBuffer::EnqueueRetransmissions (CouwbatTxQueue& txQueue)
{
  NS_LOG_FUNCTION (this);
  TxHistory *hist = m_hist[4];
  if (!hist)
    {
      return;
    }

  for (TxHistory::const_iterator it1 = hist->begin (); it1 != hist->end (); ++it1)
    {
      Mac48Address dest = it1->first;
      for (PacketListBySeq::const_iterator it2 = it1->second.begin (); it2 != it1->second.end (); ++it2)
        {
          const PacketList *packList = &(it2->second);
          for (uint32_t i = 0; i < packList->size (); ++i)
            {
              NS_LOG_DEBUG ("CouwbatTxHistoryBuffer reenqueue packet: " << (*packList)[i]->ToString ());
              txQueue.Reenqueue (dest, (*packList)[i]);
            }
        }
    }

  hist->clear ();
}

}
