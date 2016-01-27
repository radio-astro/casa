/*
 * ScantableReader.h
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SCANTABLE2MSREADER_H_
#define SINGLEDISH_FILLER_SCANTABLE2MSREADER_H_

#include<singledish/Filler/ReaderInterface.h>

#include <string>

// casacore includes
#include <casacore/casa/Containers/Record.h>
#include <casacore/tables/Tables/TableRecord.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Scantable2MSReader : public ReaderInterface
{
public:
  Scantable2MSReader(std::string const &scantable_name);
  virtual ~Scantable2MSReader();

  // get number of rows for MAIN table
  virtual size_t getNumberOfRows() { return 0;}

  // to get OBSERVATION table
  virtual Bool getObservationRow(ObservationRecord &record);

  // to get ANTENNA table
  virtual Bool getAntennaRow(AntennaRecord &record);

  // to get PROCESSOR table
  virtual Bool getProcessorRow(ProcessorRecord &row);

  // to get SOURCE table
  virtual Bool getSourceRow(SourceRecord &row);

  // to get FIELD table
  virtual Bool getFieldRow(FieldRecord &row);

  // to get SOURCE table
  virtual Bool getSpectralWindowRow(SpectralWindowRecord &row);

  // to get SYSCAL table
  virtual Bool getSyscalRow(SysCalRecord &row);

  // to get WEATHER table
  virtual Bool getWeatherRow(WeatherRecord &row);

  // to get MAIN table
  virtual Bool getMainRecord(TableRecord &record);

  // for DataAccumulator
  virtual Bool getData(size_t irow, TableRecord &record);

protected:
  void initializeSpecific();
  void finalizeSpecific();
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SCANTABLE2MSREADER_H_ */
