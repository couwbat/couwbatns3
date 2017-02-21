#ifndef SIMPLE_COUWBAT_CHANNEL_H
#define SIMPLE_COUWBAT_CHANNEL_H

#include <vector>
#include <stdint.h>
#include "ns3/packet.h"
#include "couwbat-channel.h"
#include "couwbat-mode.h"
#include "couwbat-tx-vector.h"

namespace ns3
{

class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
class SimpleCouwbatPhy;

/**
 * \brief A simple couwbat channel.
 * \ingroup couwbat
 *
 * This couwbat channel implements a simple propagation model
 * described in the COUWBAT model library.
 *
 * This class is expected to be used in tandem with the ns3::SimpleCouwbatPhy
 * class.
 * By default no properties are set, so it is the callers responsibility
 * to set them before using the channel.
 */
class SimpleCouwbatChannel : public CouwbatChannel
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  SimpleCouwbatChannel (); //!< Default constructor
  virtual ~SimpleCouwbatChannel (); //!< Destructor

  // inherited from Channel.
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

  /**
   * Adds the given CouwbatPhy to the PHY list
   *
   * \param phy the CouwbatPhy to be added to the PHY list
   */
  void Add (Ptr<SimpleCouwbatPhy> phy);

  /**
   * \param loss the new propagation loss model.
   */
  void SetPropagationLossModel (std::vector<Ptr<PropagationLossModel> > loss);
  /**
   * \param delay the new propagation delay model.
   */
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);

  /**
   * \param sender the device from which the packet is originating.
   * \param packet the packet to send
   * \param txPowerDbm the tx power associated to the packet
   * \param txVector the TXVECTOR associated to the packet
   * \param preamble the preamble associated to the packet
   *
   * This method should not be invoked by normal users. It is
   * currently invoked only from CouwbatPhy::Send. SimpleCouwbatChannel
   * delivers packets only between PHYs with the same m_channelNumber,
   * e.g. PHYs that are operating on the same channel.
   */
  void Send (Ptr<SimpleCouwbatPhy> sender, Ptr<const Packet> packet, double txPowerDbm,
		  CouwbatTxVector txVector) const;

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
   int64_t AssignStreams (int64_t stream);

private:
  typedef std::vector<Ptr<SimpleCouwbatPhy> > PhyList; //!< A vector of Pointers to SimpleCouwbatPhy.
  PhyList m_phyList;//!< List of SimpleCouwbatPhys connected to this SimpleCouwbatChannel
  std::vector<Ptr<PropagationLossModel> > m_loss; //!< Propagation loss model for every subchannel
  Ptr<PropagationDelayModel> m_delay; //!< Propagation delay model

  /**
   * This method is scheduled by Send for each associated SimpleCouwbatPhy.
   * The method then calls the corresponding SimpleCouwbatPhy that the first
   * bit of the packet has arrived.
   *
   * \param i index of the corresponding SimpleCouwbatPhy in the PHY list
   * \param packet the packet being sent
   * \param rxPowerDbm the received power of the packet for each used subchannel
   * \param txVector the TXVECTOR of the packet
   * \param preamble the type of preamble being used to send the packet
   */
  void Receive (uint32_t i, Ptr<Packet> packet, std::vector<double> rxPowerDbm,
                CouwbatTxVector txVector) const;
};

} // namespace ns3

#endif /* SIMPLE_COUWBAT_CHANNEL_H */
