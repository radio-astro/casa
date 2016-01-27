/*
 * Scantable2MSReader.cc
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#include <singledish/Filler/Scantable2MSReader.h>

#include <iostream>
#include <string>

#include <casacore/casa/Containers/Record.h>
#include <casacore/tables/Tables/TableRecord.h>

using namespace casacore;

namespace casa { //# NAMESPACE CASA - BEGIN

Scantable2MSReader::Scantable2MSReader(std::string const &scantable_name) :
    ReaderInterface(scantable_name) {
  std::cout << "Scantabl2MSReader::Scantable2MSReader" << std::endl;
}

Scantable2MSReader::~Scantable2MSReader() {
  std::cout << "Scantabl2MSReader::~Scantable2MSReader" << std::endl;
}

void Scantable2MSReader::initializeSpecific() {
  std::cout << "Scantabl2MSReader::initialize" << std::endl;
}

void Scantable2MSReader::finalizeSpecific() {
  std::cout << "Scantabl2MSReader::finalize" << std::endl;
}

Bool Scantable2MSReader::getAntennaRow(TableRecord &record) {
  std::cout << "Scantabl2MSReader::getAntennaRow" << std::endl;

  return True;
}

Bool Scantable2MSReader::getMainRecord(TableRecord &record) {
  std::cout << "Scantabl2MSReader::getMainRecord" << std::endl;

  return True;
}

Bool Scantable2MSReader::getObservationRow(TableRecord &record) {
  std::cout << "Scantabl2MSReader::getObservationRow" << std::endl;



  return True;
}

Bool Scantable2MSReader::getProcessorRow(TableRecord &record) {
  std::cout << "Scantabl2MSReader::getProcessorRow" << std::endl;

  return True;
}

Bool Scantable2MSReader::getSourceRow(TableRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getFieldRow(TableRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getSpectralWindowRow(TableRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getSyscalRow(TableRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getWeatherRow(TableRecord &record) {
  POST_START;

  POST_END;

  return False;
}

Bool Scantable2MSReader::getData(size_t irow, TableRecord &record) {
  std::cout << "Scantable2MSReader::getData" << std::endl;

  return True;
}

} //# NAMESPACE CASA - END
