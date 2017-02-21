#ifndef COUWBAT_MAC_H
#define COUWBAT_MAC_H

#include "ns3/object.h"
#include "ns3/network-module.h"
#include "couwbat-phy.h"
#include "couwbat-net-device.h"

namespace ns3
{

/**
 * \brief Abstract base class for all MAC-level couwbat objects.
 * \ingroup couwbat
 *
 * This class encapsulates all the base MAC functionality.
 */
class CouwbatMac : public Object
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  CouwbatMac (); //!< Default constructor
  ~CouwbatMac (); //!< Destructor

  /**
   * Set the MAC address of this mac.
   * \param ad Mac48Address of this mac.
   */
  void SetAddress (Mac48Address ad);

  /**
   * Get MAC address of this mac.
   * \return Mac48Address of this mac.
   */
  Mac48Address GetAddress (void) const;
  
  /**
   * Set the device this mac is associated with.
   * \param device The device this mac is associated with.
   */
  void SetDevice (Ptr<Object> device);

  /**
   * Get the device this mac is associated with.
   * \return Device this mac is associated with.
   */
  Ptr<Object> GetDevice (void);
  
   /**
   * Set the PHY this mac is associated with.
   * \param device The PHY this mac is associated with.
   */
  void SetPhy (Ptr<CouwbatPhy> phy);

  /**
   * Get the PHY this mac is associated with.
   * \return PHY this mac is associated with.
   */
  Ptr<CouwbatPhy> GetPhy (void);

  /**
   * Set m_netlinkMode to a certain value
   */
  void SetNetlinkMode (bool value);

  /**
   * \param packet the packet to send.
   * \param to the address to which the packet should be sent.
   *
   * The packet should be enqueued in a tx queue, and should be
   * dequeued when it is being sent
   */
  virtual void Enqueue (Ptr<Packet> packet, Mac48Address to) = 0;

  /**
   * Return the number of packets in the TxQueue
   */
  virtual int GetTxQueueSize (const Mac48Address dest) = 0;

  /**
   * Forward received payload packets to higher layers (CouwbatNetDevice)
   */
  void ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to);

  /**
   * Used only in netlink mode to set a forwardup callback for packets going from MAC to upper layers (external applications), gets called by MAC.
   */
  void SetNetlinkForwardUpCallback (Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> cb);

  /**
   * Calculate the number of bits per symbol for a certain MCS.
   */
  double BitsPerSymbol (enum CouwbatMCS mcs);

  /**
   * Calculate the number of transmittable bytes with a certain number of symbols and a configuration
   * 
   * \param num_symbols number of used symbols
   * \param num_subchannels number of used subchannels
   * \param mcs MCS vector for used subchannels, size must be equal to num_subchannels, i.e. no entry for unused subchannels
   * \returns number of transmittable bytes
   */
  static double TransmittableBytesWithSymbols (uint32_t num_symbols, uint32_t num_subchannels, const std::vector<enum CouwbatMCS> &mcs);

  /**
   * Calculate the number of necessary symbols in order to transmit a payload packet of a certain size in bytes.
   * 
   * \param size_bytes size in bytes of the payload that needs to be transmitted
   * \param num_subchannels number of used subchannels
   * \param mcs MCS vector for used subchannels, size must be equal to num_subchannels, i.e. no entry for unused subchannels
   * \param padding_bytes [out] this variable gets modified and will contain the necessary padding bytes for the given configuration
   * \returns number of necessary symbols
   */
  static double NecessarySymbolsForBytes (uint32_t size_bytes, uint32_t num_subchannels, const std::vector<enum CouwbatMCS> &mcs, double &padding_bytes);

protected:
  Mac48Address m_address; //!< Mac48Address of this mac.
  Ptr<Object> m_device;  //!< Parent CouwbatNetDevice
  Ptr<CouwbatPhy> m_phy; //!< Access to PHY
  bool m_netlinkMode; //!< If true, MAC runs in real time netlink mode (forwardup to m_netlinkForwardUpCallback), else in complete ns3 simulator mode (forawrdup to CouwbatNetDevice)
  Callback<void, Ptr<Packet>, Mac48Address, Mac48Address> m_netlinkForwardUpCallback; //!< Used only in netlink mode, forwardup callback for packets going from MAC to upper layers (external applications), gets called by MAC. 
};

} // namespace ns3
#endif /* COUWBAT_MAC_H */
