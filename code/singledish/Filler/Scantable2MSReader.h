/*
 * ScantableReader.h
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SCANTABLE2MSREADER_H_
#define SINGLEDISH_FILLER_SCANTABLE2MSREADER_H_

#include <singledish/Filler/ReaderInterface.h>
#include <singledish/Filler/ScantableIterator.h>

#include <string>
#include <memory>

// casacore includes
#include <casacore/casa/Containers/Record.h>
#include <casacore/tables/Tables/TableRecord.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Scantable2MSReader: public ReaderInterface {
public:
  Scantable2MSReader(std::string const &scantable_name);
  virtual ~Scantable2MSReader();

  // get number of rows for MAIN table
  virtual size_t getNumberOfRows() {
    return 0;
  }

  // to get OBSERVATION table
  virtual Bool getObservationRow(ObservationRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_observation_row_)(record);

    POST_END;
    return return_value;
  }

  // to get ANTENNA table
  virtual Bool getAntennaRow(AntennaRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_antenna_row_)(record);

    POST_END;
    return return_value;
  }

  // to get PROCESSOR table
  virtual Bool getProcessorRow(ProcessorRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_processor_row_)(record);

    POST_END;
    return return_value;
  }

  // to get SOURCE table
  virtual Bool getSourceRow(SourceRecord &row);

  // to get FIELD table
  virtual Bool getFieldRow(FieldRecord &row);

  // to get SOURCE table
  virtual Bool getSpectralWindowRow(SpectralWindowRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_spw_row_)(record);

    POST_END;
    return return_value;
  }

  // to get SYSCAL table
  virtual Bool getSyscalRow(SysCalRecord &row);

  // to get WEATHER table
  virtual Bool getWeatherRow(WeatherRecord &record) {
    POST_START;

    Bool return_value = (*this.*get_weather_row_)(record);

    POST_END;
    return return_value;
  }

  // to get MAIN table
  virtual Bool getMainRecord(TableRecord &record);

  // for DataAccumulator
  virtual Bool getData(size_t irow, TableRecord &record);

protected:
  void initializeSpecific();
  void finalizeSpecific();

private:
  std::unique_ptr<Table> main_table_;

  Bool (Scantable2MSReader::*get_antenna_row_)(AntennaRecord &);
  Bool (Scantable2MSReader::*get_observation_row_)(ObservationRecord &);
  Bool (Scantable2MSReader::*get_processor_row_)(ProcessorRecord &);
  Bool (Scantable2MSReader::*get_spw_row_)(SpectralWindowRecord &);
  Bool (Scantable2MSReader::*get_weather_row_)(WeatherRecord &);

  std::unique_ptr<ScantableWeatherIterator> weather_iter_;
  std::unique_ptr<ScantableFrequenciesIterator> freq_iter_;

  Bool getAntennaRowImpl(AntennaRecord &record);
  Bool getObservationRowImpl(ObservationRecord &record);
  Bool getProcessorRowImpl(ProcessorRecord &record);
  Bool getSpectralWindowRowImpl(SpectralWindowRecord &record);
  Bool getWeatherRowImpl(WeatherRecord &record);

  template<class _Record>
  Bool noMoreRowImpl(_Record &) {
    POST_START;
    POST_END;
    return False;
  }
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SCANTABLE2MSREADER_H_ */
