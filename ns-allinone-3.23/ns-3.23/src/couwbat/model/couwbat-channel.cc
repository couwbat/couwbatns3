#include "couwbat-channel.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("CouwbatChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CouwbatChannel);

TypeId
CouwbatChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatChannel")
    .SetParent<Channel> ()
    .SetGroupName ("Couwbat")
  ;
  return tid;
}

CouwbatChannel::CouwbatChannel ()
{
  NS_LOG_FUNCTION (this);
}

CouwbatChannel::~CouwbatChannel ()
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
