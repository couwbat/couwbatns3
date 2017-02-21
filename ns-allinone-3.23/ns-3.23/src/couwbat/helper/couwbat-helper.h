/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef COUWBAT_HELPER_H
#define COUWBAT_HELPER_H

#include "ns3/core-module.h"

namespace ns3 {

class Node;
class PrimaryUserNetDevice;
class TraceBasedPuNetDevice;
class SpectrumDb;
class SpectrumMap;
class OnOffModel;
class OnceOnOffModel;
class CouwbatPhy;
class CouwbatMac;
class CouwbatNetDevice;
class NetDevice;

/**
 * \ingroup helper
 * 
 * \brief create PHY objects
 *
 * This base class must be implemented by new PHY implementations which wish to integrate
 * with the \ref ns3::CouwbatHelper class.
 */
class CouwbatPhyHelper {
public:
  virtual ~CouwbatPhyHelper ();

  /**
   * \param node the node on which the PHY object will reside
   * \param device the device within which the phy object will reside
   * \returns a new PHY object.
   *
   * Subclasses must implement this method to allow the ns3::CouwbatHelper class
   * to create PHY objects from ns3::CouwbatHelper::Install.
   */
  virtual Ptr<CouwbatPhy> Create (Ptr<Node> node, Ptr<CouwbatNetDevice> device ) const = 0;

};

/**
 * \ingroup helper
 * 
 * \brief create MAC objects
 *
 * This base class must be implemented by new MAC implementations which wish to integrate
 * with the \ref ns3::CouwbatHelper class.
 */
class CouwbatMacHelper {
public:
  virtual ~CouwbatMacHelper ();

  /**
   * \param device the device within which the mac object will reside
   * \returns a new MAC object.
   *
   * Subclasses must implement this method to allow the ns3::CouwbatHelper class
   * to create MAC objects from ns3::CouwbatHelper::Install.
   */
  virtual Ptr<CouwbatMac> Create (Ptr<CouwbatNetDevice> device) const = 0;
};

/**
 * \ingroup helper
 * 
 * \brief helps to manage and create CouwbatNetDevice objects
 *
 * This class can help to create a large set of similar
 * CouwbatNetDevice objects and to configure their attributes
 * during creation.
 */
class CouwbatHelper
{
public:
  /**
   *\brief Create a CouwbatHelper in an empty state.
   */
  CouwbatHelper (void);
  ~CouwbatHelper ();

  /**
   * \param node The node on which to install the Spectrum Database.
   * \return The created spectrum database , null if there already exists a spectrum database.
   *
   * Install a Couwbat Spectrum Database on a given node.
   * A spectrum database can only be installed on one node,
   * if this helper already has a node with an installed database,
   * that one will be used for all further device installations.
   */
  Ptr<SpectrumDb> InstallSpectrumDb (Ptr<Node> node);

  /**
   * \param node A node on which to install a Primary User.
   * \param map The spectrum map that this primary user will use.
   * \param onOffModel The on off model that switches the primary user on an off.
   * \return The created net device for the primary user, null if PU could not be created.
   *
   * Install a Couwbat Primary User on a given node.
   * The Primary user will inform the spectrum database that was installed
   * with InstallSpectrumDb about its used subcarriers.
   * If no spectrum database is available in this helper,
   * a primary user cannot be installed.
   */
  Ptr<PrimaryUserNetDevice> InstallPrimaryUser (Ptr<Node> node, Ptr<SpectrumMap> map, Ptr<OnOffModel> onOffModel);

  /**
   * \param node A node on which to install a TB Primary User.
   * \param onOffModel The on off model that switches the PU on an off.
   * \param fileName The name (and location if not in working directory)
   * of the CSV file to use for the spectrum usage.
   * \param samplingInterval Sampling interval of the input file.
   * The spectrum usage will be updated this often.
   * \param startingLine The line number from which to start operation.
   * If less than or equal to 0, a random starting line is chosen.
   * \return The created net device for the primary user, null if PU could not be created.
   *
   * Install a Couwbat TB Primary User on a given node.
   * The Primary user will inform the spectrum database that was installed
   * with InstallSpectrumDb about its used subcarriers.
   * If no spectrum database is available in this helper,
   * a primary user cannot be installed.
   */
  Ptr<TraceBasedPuNetDevice> InstallTraceBasedPu (
      Ptr<Node> node, Ptr<OnOffModel> onOffModel, std::string fileName,
      Time samplingInterval, int startingLine);

  /**
   * \param start_subcarrier The index of the first subcarrier that is set in the new spectrum map.
   * \param n_subcarriers The number of subcarriers to set.
   * \return A new Spectrum map, that has the selected subcarriers set.
   *
   * Create a spectrum map that has set n_subcarriers to beginning from start_subcarrier.
   */
  Ptr<SpectrumMap> CreateSpectrumMap (uint32_t start_subcarrier, uint32_t n_subcarriers);

  /**
   * \param firstOn the time when to first turn on the primary user
   * \param finalOff the time when to finaly turn off the primary user.
   * \return a new on off model, that turn the primary user on at firstOn and off at finalOff
   *
   * Create a on off model that turns a primary user once on at firstOn and once off at finalOff.
   */
  Ptr<OnceOnOffModel> CreateOnceOnOffModel (TimeValue firstOn, TimeValue finalOff);

  /**
   * \param phy the PHY helper to create PHY objects
   * \param mac the MAC helper to create MAC object
   * \param node the node on which a couwbat device must be created
   * \return a device created by this method
   *
   * Create a CouwbatNetDevice with the given mac and phy installed on the specified node.
   * If the mac is of type base station, the CouwbatNetDevice will use the SpectrumDb that
   * was created first by InstallSpectrumDb.
   */
  virtual Ptr<CouwbatNetDevice> Install
    (
      const CouwbatPhyHelper &phy,
      const CouwbatMacHelper &mac,
      Ptr<Node> node
    ) const;

  /**
   * Enable logging for all Couwbat models on all levels.
   */
  static void EnableLogComponents (void);

private:
  Ptr<Node> m_specDbNode; //!< the node on which the spectrum db gets installed

};

} //namespace ns3

#endif /* COUWBAT_HELPER_H */

