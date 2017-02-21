#ifndef TRACE_BASED_PU_NET_DEVICE_H
#define TRACE_BASED_PU_NET_DEVICE_H

#include "ns3/core-module.h"
#include "onoff-model.h"
#include <string>

namespace ns3
{

class SpectrumMap;
class SpectrumDb;
class Node;

/**
 * \brief Model for a Primary User that occupies a spectrum
 * according to a trace file and informs the SpectrumDb
 * about the usage.
 *
 * \ingroup couwbat
 *
 * The TraceBasedPuNetDevice uses a definable spectrum that it
 * occupies and leaves according to a CSV trace file.
 * The TraceBasedPuNetDevice informs a spectrum database via direct
 * memory access when it occupies and leaves the spectrum.
 */
class TraceBasedPuNetDevice : public Object
{
public:
  static TypeId GetTypeId (void);
  TraceBasedPuNetDevice (void);
  ~TraceBasedPuNetDevice ();

  /**
   * Set the spectrum to occupy while turned on.
   * \param fileName The name (and location if not in working directory)
   * of the CSV file to use for the spectrum usage.
   * \param samplingInterval Sampling interval of the input file.
   * The spectrum usage will be updated this often. If it is zero,
   * the TBPU operates in manual update mode.
   *
   * TODO: document CSV format
   */
  void SetSpectrumTraceFile (std::string fileName, Time samplingInterval);

  /**
   * Set the spectrum database.
   * \param db The spectrum database.
   */
  void SetSpectrumDb (Ptr<SpectrumDb> db);

  /**
   * Set the on-off model.
   * \param The on-off model.
   */
  void SetOnOffModel (Ptr<OnOffModel> model);

  /**
   * Set the node that this PU is associated to.
   * \param node The node that this PU is associated to.
   */
  void SetNode (Ptr<Node> node);

  /**
   * Return the node that this PU is associated to.
   * \return The node that this PU is associated to.
   */
  Ptr<Node> GetNode (void) const;

  /**
   * (Re-)Initialize this TBPU and start operation.
   * \param startingLine The line number from which to start operation.
   * If less than or equal to 0, a random starting line is chosen.
   */
  void Start (int startingLine);

  /**
   * Resumes PU operation if currently stopped.
   */
  void TurnOn (void);

  /**
   * Disables PU operation until started again.
   */
  void TurnOff (void);

  /**
   * Reads the current line from the file, updates spectrum usage and
   * schedules next update.
   *
   * NOTE: Only call from outside the class in manual update mode, otherwise the scheduler and OnOffModel handle this
   *
   * \param first If true, do not leave current spectrum.
   */
  void Update (bool first = false);

private:
  Ptr<SpectrumDb> m_specDb; //!< The spectrum database.
  Ptr<OnOffModel> m_onOffModel; //!< The on-off model.
  Ptr<SpectrumMap> m_specMap; //!< The currently occupied spectrum
  Ptr<Node> m_node; //!< The node that this PU is associated to.
  std::string m_fileName; //!< Filename of CSV
  Time m_samplingInterval; //!< Sampling interval of CSV
  unsigned int m_currentLine; //!< Keep track of file line
  EventId m_nextUpdate; //!< Store next update event (in case of need to e.g. cancel)
  unsigned int m_lineCount; //!< Number of lines in current CSV file
}; // class PrimaryUserNetDevice

} // namespace ns3

#endif /* TRACE_BASED_PU_NET_DEVICE_H */
