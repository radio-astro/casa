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
  virtual Bool getObservationRow(TableRecord &row);

  // to get ANTENNA table
  virtual Bool getAntennaRow(TableRecord &row);

  // to get PROCESSOR table
  virtual Bool getProcessorRow(TableRecord &row);

  // to get SOURCE table
  virtual Bool getSourceRow(TableRecord &row);

  // to get FIELD table
  virtual Bool getFieldRow(TableRecord &row);

  // to get SOURCE table
  virtual Bool getSpectralWindowRow(TableRecord &row);

  // to get SYSCAL table
  virtual Bool getSyscalRow(TableRecord &row);

  // to get WEATHER table
  virtual Bool getWeatherRow(TableRecord &row);

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
