/*
 * NROReader.h
 *
 *  Created on: May 9, 2016
 *      Author: wataru kawasaki
 */

#ifndef SINGLEDISH_FILLER_NRO2MSREADER_H_
#define SINGLEDISH_FILLER_NRO2MSREADER_H_

#define STRING2CHAR(s) const_cast<char *>((s).c_str())

#include <singledish/Filler/ReaderInterface.h>
#include <singledish/Filler/NROData.h>
#include <string>
#include <memory>

namespace casa { //# NAMESPACE CASA - BEGIN
  
constexpr double kDay2Sec = 86400.0;
constexpr double kSec2Day = 1.0 / kDay2Sec;

class NRO2MSReader: public ReaderInterface {
public:
  NRO2MSReader(std::string const &scantable_name);
  virtual ~NRO2MSReader();

  // get number of rows
  virtual size_t getNumberOfRows();

  virtual MDirection::Types getDirectionFrame();
  
  virtual Bool isFloatData() const {
    return True;
  }

  // to get OBSERVATION table
  virtual Bool getObservationRow(sdfiller::ObservationRecord &record) {
    POST_START;
    Bool return_value = (*this.*get_observation_row_)(record);
    POST_END;
    return return_value;
  }

  // to get ANTENNA table
  virtual Bool getAntennaRow(sdfiller::AntennaRecord &record) {
    POST_START;
    Bool return_value = (*this.*get_antenna_row_)(record);
    POST_END;
    return return_value;
  }

  // to get PROCESSOR table
  virtual Bool getProcessorRow(sdfiller::ProcessorRecord &record) {
    POST_START;
    Bool return_value = (*this.*get_processor_row_)(record);
    POST_END;
    return return_value;
  }

  // to get SOURCE table
  virtual Bool getSourceRow(sdfiller::SourceRecord &record) {
    POST_START;
    Bool return_value = (*this.*get_source_row_)(record);
    POST_END;
    return return_value;
  }

  // to get FIELD table
  virtual Bool getFieldRow(sdfiller::FieldRecord &record) {
    POST_START;
    Bool return_value = (*this.*get_field_row_)(record);
    POST_END;
    return return_value;
  }

  // to get SOURCE table
  virtual Bool getSpectralWindowRow(sdfiller::SpectralWindowRecord &record) {
    POST_START;
    Bool return_value = (*this.*get_spw_row_)(record);
    POST_END;
    return return_value;
  }

  // for DataAccumulator
  virtual Bool getData(size_t irow, sdfiller::DataRecord &record);

protected:
  void initializeSpecific();
  void finalizeSpecific();

private:
  FILE *fp_;
  sdfiller::NRODataObsHeader obs_header_;
  void readObsHeader();
  void readScanData(int const irow, sdfiller::NRODataScanData &data);
  void checkEndian();
  template<typename T>
  void convertEndian(T &value) {
    char volatile *first = reinterpret_cast<char volatile *>(&value) ;
    char volatile *last = first + sizeof(T) ;
    std::reverse(first, last) ;
  }

  bool same_endian_;

  template<typename T>
  void readHeader(T &v) {
    if ((int)fread(&v, 1, sizeof(T), fp_) != sizeof(T)) {
      cout << "read failed." << endl;
    }
    if (!same_endian_) {
      convertEndian(v);
    }
  }

  template<typename T>
  void readHeader(T *v, size_t numArray) {
    for (size_t i = 0; i < numArray; ++i) {
      readHeader<T>(v[i]);
    }
  }

  void readHeader(string &v, size_t strLength) {
    v.resize(strLength);
    if (fread(STRING2CHAR(v), 1, strLength, fp_) != strLength) {
      cout << "read failed." << endl;
    }
    v.resize(strlen(v.c_str())); // remove trailing null characters
  }

  void readHeader(string *v, size_t strLength, size_t numArray) {
    for (size_t i = 0; i < numArray; ++i) {
      readHeader(v[i], strLength);
    }
  }

  int beam_id_counter_;
  int source_spw_id_counter_;
  int spw_id_counter_;
  Vector<Double> time_range_sec_;
  int const len_obs_header_ = 15136;
  double getMJD(string const &time);
  double getIntMiddleTimeSec(sdfiller::NRODataScanData const &data);
  double getIntStartTimeSec(int const scanno);
  double getIntEndTimeSec(int const scanno);
  void getFullTimeRange();
  double getMiddleOfTimeRangeSec();

  Double const posx_ = -3.8710235e6;
  Double const posy_ =  3.4281068e6;
  Double const posz_ =  3.7240395e6;

  double getRestFrequency(int const spwno);

  std::vector<double> getSpectrum(int const irow, sdfiller::NRODataScanData const &data);
  Int getPolNo(string const &rx);

  Bool (NRO2MSReader::*get_antenna_row_)(sdfiller::AntennaRecord &);
  Bool (NRO2MSReader::*get_field_row_)(sdfiller::FieldRecord &);
  Bool (NRO2MSReader::*get_observation_row_)(sdfiller::ObservationRecord &);
  Bool (NRO2MSReader::*get_processor_row_)(sdfiller::ProcessorRecord &);
  Bool (NRO2MSReader::*get_source_row_)(sdfiller::SourceRecord &);
  Bool (NRO2MSReader::*get_spw_row_)(sdfiller::SpectralWindowRecord &);

  Bool getAntennaRowImpl(sdfiller::AntennaRecord &record);
  Bool getFieldRowImpl(sdfiller::FieldRecord &record);
  Bool getObservationRowImpl(sdfiller::ObservationRecord &record);
  Bool getProcessorRowImpl(sdfiller::ProcessorRecord &record);
  Bool getSourceRowImpl(sdfiller::SourceRecord &record);
  Bool getSpectralWindowRowImpl(sdfiller::SpectralWindowRecord &record);

  template<class _Record>
  Bool noMoreRowImpl(_Record &) {
    POST_START;POST_END;
    return False;
  }
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_NRO2MSREADER_H_ */
