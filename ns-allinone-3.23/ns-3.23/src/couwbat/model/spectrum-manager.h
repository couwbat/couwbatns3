#ifndef SPECTRUM_MANAGER_H
#define SPECTRUM_MANAGER_H

#include "ns3/object.h"

namespace ns3
{

/**
 * \brief The SpectrumManager performs spectrum managements tasks inside a CouwbatNetDevice.
 * \ingroup couwbat
 *
 * The SpectrumManager should be aggregated to CouwbatNetDevices that implement a CR-BS by using a BsCouwbatMac.
 * The SpectrumManager has access to the mac and phy of the device and know the mapping between channel and subcarriers.
 * It uses a spectrum database to give information if a control channel is available or not.
 *
 * TODO implement any further spectrum managements tasks here (e.g spectrum sensing,
 * spectrum decicion, spectrum sharing, spectrum mobility, communication with a client
 * spectrum database application)
 */
class SpectrumManager : public Object
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  SpectrumManager (); //!< Default constructor
  ~SpectrumManager (); //!< Destructor

  /**
   * \param specDb The spectrum database to use.
   * Set the spectrum database to use. The class of the specDb
   * determines how to communicate with the specDb.
   */
  void SetSpectrumDb (Ptr<Object> specDb);

  /**
   * \param ccId The id of the control channel.
   * \return true if the control channel is considered free, false if the control channel is occupied.
   */
  bool IsCcFree (uint32_t ccId);
  
protected:
  virtual void DoInitialize (void);

private:
 /*
  * The spectrum database this device is connected to.
  * Using Object, to allow different types of implementation.
  * If m_specDB is of type SpectrumDb, god-like access to the database is perfored through
  * direct memory access. For later implementations of a spectrum database model as an
  * client/server application the member methods of this class can check that case and
  * use the apropriate interface.
  */
  Ptr<Object> m_specDb;
};

} // namespace ns3

#endif /* SPECTRUM_MANAGER_H */
