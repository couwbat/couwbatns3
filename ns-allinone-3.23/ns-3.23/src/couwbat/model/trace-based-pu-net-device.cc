#include "trace-based-pu-net-device.h"
#include "spectrum-map.h"
#include "spectrum-db.h"
#include "couwbat.h"
#include "ns3/network-module.h"
#include "ns3/log.h"
#include <sys/stat.h>
#include <fstream>
#include <limits>
#include <cstdlib>
#include <iterator>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("TraceBasedPuNetDevice");

namespace ns3
{

/**
 * File utils
 */
bool FileExists (std::string fileName)
{
  struct stat buffer;
  return stat (fileName.c_str(), &buffer) == 0;
}

unsigned int FileNumberOfLines (std::string fileName)
{
  int number_of_lines = 0;
  std::string line;
  std::ifstream myfile (fileName.c_str ());

  while (std::getline (myfile, line))
    {
      ++number_of_lines;
    }
  return number_of_lines;
}

std::fstream& FileGotoLine(std::fstream& file, unsigned int num)
{
    file.seekg (std::ios::beg);
    for (unsigned int i = 0; i < num - 1; ++i){
        file.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return file;
}

/*
 * CSV utils
 */
class CsvRow
{
  public:
    std::string const& operator[] (std::size_t index) const
    {
        return m_data[index];
    }
    std::size_t size () const
    {
        return m_data.size();
    }
    void readNextRow (std::istream& str)
    {
        std::string line;
        std::getline (str, line);

        std::stringstream lineStream (line);
        std::string cell;

        m_data.clear ();
        while (std::getline (lineStream, cell, ','))
        {
            m_data.push_back (cell);
        }
    }
  private:
    std::vector<std::string> m_data;
};

std::istream& operator>> (std::istream& str, CsvRow& data)
{
    data.readNextRow(str);
    return str;
}

/**
 * TraceBasedPuNetDevice
 */

NS_OBJECT_ENSURE_REGISTERED (TraceBasedPuNetDevice);

TypeId
TraceBasedPuNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::TraceBasedPuNetDevice")
    .SetParent<Object> ()
    .AddConstructor<TraceBasedPuNetDevice> ()
    .SetGroupName("Couwbat")
  ;
  return tid;
}

TraceBasedPuNetDevice::TraceBasedPuNetDevice (void)
  : m_specDb (0),
    m_onOffModel (0),
    m_specMap (0),
    m_node (0),
    m_fileName (""),
    m_samplingInterval (0),
    m_currentLine (0),
    m_lineCount (0)
{
  NS_LOG_FUNCTION (this);
}

TraceBasedPuNetDevice::~TraceBasedPuNetDevice ()
{
  m_specDb = 0;
  m_onOffModel = 0;
  m_specMap = 0;
  m_node = 0;
  NS_LOG_FUNCTION (this);
}

void
TraceBasedPuNetDevice::SetSpectrumTraceFile (std::string fileName,
                                             Time samplingInterval)
{
  NS_LOG_FUNCTION (this << fileName << samplingInterval);
  NS_LOG_INFO ("TBPU using file " << fileName << " with sampling interval of "
               << samplingInterval);

  m_lineCount = FileNumberOfLines (fileName);

  if (!FileExists (fileName) || m_lineCount < 1)
    {
      NS_LOG_WARN ("TraceBasedPuNetDevice::SetSpectrumTraceFile(): Invalid arguments or file.");
      return;
    }

  if (samplingInterval <= Time ())
    {
      NS_LOG_INFO ("TBPU running in manual update mode.");
    }

  m_fileName = fileName;
  m_samplingInterval = samplingInterval;
}

void
TraceBasedPuNetDevice::SetSpectrumDb (Ptr<SpectrumDb> db)
{
  NS_LOG_FUNCTION (this << db);
  m_specDb = db;
}

void
TraceBasedPuNetDevice::SetOnOffModel (Ptr<OnOffModel> model)
{
  NS_LOG_FUNCTION (this << model);
  m_onOffModel = model;
}

void
TraceBasedPuNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node>
TraceBasedPuNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void
TraceBasedPuNetDevice::Start (int startingLine)
{
  NS_LOG_FUNCTION (this);

  if (!m_specDb || m_fileName == "")
    {
      NS_LOG_WARN ("TraceBasedPuNetDevice::Start(): Invalid member variables, cannot start");
      return;
    }

  if (m_samplingInterval > Time ())
    {
      Simulator::Cancel (m_nextUpdate);
    }
  m_specMap = CreateObject<SpectrumMap> ();

  if (startingLine <= 0)
    {
      m_currentLine = std::rand () % m_lineCount;
    }
  else
    {
      m_currentLine = startingLine;
    }
  NS_LOG_INFO ("TBPU starting from line " << startingLine);

  if (!m_onOffModel)
    {
      NS_LOG_INFO ("No OnOffModel available for TBPU, turning on.");
      TurnOn ();
    }
  else
    {
      m_onOffModel->Start ();
    }
}

void
TraceBasedPuNetDevice::TurnOff (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("TBPU turning off");
  if (m_specMap)
    {
      m_specDb->LeaveSpectrum (m_specMap);
    }
  m_specMap = CreateObject<SpectrumMap> ();
  if (m_samplingInterval > Time ())
    {
      Simulator::Cancel (m_nextUpdate);
    }
}

void
TraceBasedPuNetDevice::TurnOn (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("TBPU turning on");

  if (m_samplingInterval > Time ())
    {
      Simulator::Cancel (m_nextUpdate);
      m_nextUpdate = Simulator::ScheduleNow (&TraceBasedPuNetDevice::Update, this, true);
    }
  else
    {
      Update (true);
    }
}

void
TraceBasedPuNetDevice::Update (bool first)
{
  NS_LOG_FUNCTION (this);
  if (!m_specDb || !m_specMap || m_fileName == "" || !FileExists (m_fileName)
     || m_currentLine <= 0)
    {
      NS_LOG_ERROR ("TraceBasedPuNetDevice::Update(): Invalid member variables, cannot update");
      return;
    }

  std::fstream file (m_fileName.c_str ());
  if (file.fail ())
    {
      NS_LOG_DEBUG ("TraceBasedPuNetDevice::Update(): Failed to open file");
    }

  if (m_currentLine > m_lineCount)
    {
      NS_LOG_LOGIC ("TraceBasedPuNetDevice::Update(): wrapping back to line 1 of trace file");
      m_currentLine = 1;
    }
  FileGotoLine (file, m_currentLine);

  NS_LOG_LOGIC ("TraceBasedPuNetDevice::Update(): reading line " << m_currentLine);

  CsvRow row;
  if (file >> row)
    {
      if (!first)
        {
          m_specDb->LeaveSpectrum (m_specMap);
          m_specMap = CreateObject<SpectrumMap> ();
        }

      for (uint32_t i = 0; i < Couwbat::GetNumberOfSubcarriers (); ++i)
        {
          if (row[i] == "1")
            {
              m_specMap->SetSpectrum (i, 1);
            }
        }

      m_specDb->OccupySpectrum (m_specMap);
    }
  else
    {
      NS_LOG_ERROR ("TraceBasedPuNetDevice::Update(): Error reading from file, cannot update occupied spectrum");
    }

  ++m_currentLine;

  NS_LOG_INFO ("TBPU occupied spectrum:\n" << m_specMap);
  if (m_samplingInterval > Time ())
    {
      m_nextUpdate = Simulator::Schedule (m_samplingInterval, &TraceBasedPuNetDevice::Update, this, false);
    }
}

} // namespace ns3
