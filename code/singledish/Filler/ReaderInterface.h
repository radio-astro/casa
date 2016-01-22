/*
 * ReaderInterface.h
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_READERINTERFACE_H_
#define SINGLEDISH_FILLER_READERINTERFACE_H_

// std includes
#include <string>

// casacore includes
#include <casacore/casa/Containers/Record.h>
#include <casacore/tables/Tables/TableRecord.h>

#define POST_START std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl
#define POST_END std::cout << "End " << __PRETTY_FUNCTION__ << std::endl

namespace casa { //# NAMESPACE CASA - BEGIN

// NonCopyable Mixin (CRTP)
template<class T>
class NonCopyable
{
protected:
  NonCopyable() {}
  ~NonCopyable() {}
private:
  NonCopyable(NonCopyable const &) {}
  T &operator=(T const &) {}
};

class ReaderInterface : private NonCopyable<ReaderInterface>{
public:
  ReaderInterface(std::string const &name) :
      name_(name) {
  }

  virtual ~ReaderInterface() {
  }

  std::string const &getName() const {
    return name_;
  }

  virtual Bool isFloatData() const {
    return False;
  }

  void initialize() {
    initializeCommon();
    initializeSpecific();
  }

  void finalize() {
    finalizeCommon();
    finalizeSpecific();
  }

  // get number of rows for MAIN table
  virtual size_t getNumberOfRows() = 0;

  // to get OBSERVATION table
  // The method should return True if row entry is available.
  // If it return False, row will be invalid so it should not be used.
  virtual Bool getObservationRow(TableRecord &row) = 0;

  // to get ANTENNA table
  // The method should return True if row entry is available.
  // If it return False, row will be invalid so it should not be used.
  virtual Bool getAntennaRow(TableRecord &row) = 0;

  // to get PROCESSOR table
  // The method should return True if row entry is available.
  // If it return False, row will be invalid so it should not be used.
  virtual Bool getProcessorRow(TableRecord &row) = 0;

  // to get SOURCE table
  // The method should return True if row entry is available.
  // If it return False, row will be invalid so it should not be used.
  virtual Bool getSourceRow(TableRecord &row) = 0;

  // to get SPECTRAL WINDOW table
  // The method should return True if row entry is available.
  // If it return False, row will be invalid so it should not be used.
  virtual Bool getSpectralWindowRow(TableRecord &row) = 0;

  // to get SYSCAL table
  // The method should return True if row entry is available.
  // If it return False, row will be invalid so it should not be used.
  virtual Bool getSyscalRow(TableRecord &row) = 0;

  // to get WEATHER table
  // The method should return True if row entry is available.
  // If it return False, row will be invalid so it should not be used.
  virtual Bool getWeatherRow(TableRecord &row) = 0;

  // for DataAccumulator
  virtual Bool getData(size_t irow, TableRecord &record) = 0;

protected:
  virtual void initializeSpecific() = 0;
  virtual void finalizeSpecific() = 0;

  std::string const name_;

private:
  // common initialization/finalization actions
  void initializeCommon() {}
  void finalizeCommon() {}
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_READERINTERFACE_H_ */
