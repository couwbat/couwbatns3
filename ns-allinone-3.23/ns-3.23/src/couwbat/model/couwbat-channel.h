#ifndef COUWBAT_CHANNEL_H
#define COUWBAT_CHANNEL_H

#include "ns3/channel.h"

namespace ns3
{

/**
 * \brief Couwbat channel interface specification.
 * \ingroup couwbat
 *
 * This class works in tandem with the ns3::CouwbatPhy class. If you
 * want to provide a new Couwbat Phy layer, you have to subclass both 
 * ns3::CouwbatChannel and ns3::CouwbatPhy.
 *
 * By default no properties are set, so it is the callers responsability
 * to set them before using the channel.
 * 
 * Typically, MyCouwbatChannel will define a Send method whose job is to distribute
 * packets from a MyCouwbatPhy source to a set of MyCouwbatPhy destinations. MyCouwbatPhy
 * also typically defines a Receive method which is invoked by MyCouwbatPhy.
 */
class CouwbatChannel : public Channel
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  CouwbatChannel (); //!< Default constructor
  virtual ~CouwbatChannel (); //!< Destructor
};

} // namespace ns3

#endif /* COUWBAT_CHANNEL_H */

