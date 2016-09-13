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
#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/tables/Tables/ScalarColumn.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Scantable2MSReader: public ReaderInterface {
public:
  Scantable2MSReader(std::string const &scantable_name);
  virtual ~Scantable2MSReader();

  // get number of rows for MAIN table
  virtual size_t getNumberOfRows() {
    if (!main_table_) {
      return 0;
    }
    return main_table_->nrow();
  }

  virtual casacore::Bool isFloatData() const {
    casacore::Bool is_float = true;
    if (!main_table_) {
      is_float = false;
    } else {
      casacore::String pol_type = main_table_->keywordSet().asString("POLTYPE");
      casacore::ROScalarColumn<casacore::uInt> polno_column(*main_table_, "POLNO");
      casacore::uInt max_pol = max(polno_column.getColumn());
//      std::cout << "pol_type=" << pol_type << " max_pol=" << max_pol << std::endl;
      if ((max_pol == 3) && (pol_type == "linear" || pol_type == "circular")) {
        is_float = false;
      }
    }
//    std::cout << "is_float = " << is_float << std::endl;
    return is_float;
  }

  // to get OBSERVATION table
  virtual casacore::Bool getObservationRow(sdfiller::ObservationRecord &record) {
    POST_START;

    casacore::Bool return_value = (*this.*get_observation_row_)(record);

    POST_END;
    return return_value;
  }

  // to get ANTENNA table
  virtual casacore::Bool getAntennaRow(sdfiller::AntennaRecord &record) {
    POST_START;

    casacore::Bool return_value = (*this.*get_antenna_row_)(record);

    POST_END;
    return return_value;
  }

  // to get PROCESSOR table
  virtual casacore::Bool getProcessorRow(sdfiller::ProcessorRecord &record) {
    POST_START;

    casacore::Bool return_value = (*this.*get_processor_row_)(record);

    POST_END;
    return return_value;
  }

  // to get SOURCE table
  virtual casacore::Bool getSourceRow(sdfiller::SourceRecord &record) {
    POST_START;

    casacore::Bool return_value = (*this.*get_source_row_)(record);

    POST_END;
    return return_value;
  }

  // to get FIELD table
  virtual casacore::Bool getFieldRow(sdfiller::FieldRecord &record) {
    POST_START;

    casacore::Bool return_value = (*this.*get_field_row_)(record);

    POST_END;
    return return_value;
  }

  // to get SOURCE table
  virtual casacore::Bool getSpectralWindowRow(sdfiller::SpectralWindowRecord &record) {
    POST_START;

    casacore::Bool return_value = (*this.*get_spw_row_)(record);

    POST_END;
    return return_value;
  }

  // for DataAccumulator
  virtual casacore::Bool getData(size_t irow, sdfiller::DataRecord &record);

protected:
  void initializeSpecific();
  void finalizeSpecific();

private:
  std::unique_ptr<casacore::Table> main_table_;
  casacore::Table tcal_table_;
  casacore::Table weather_table_;

  casacore::ROScalarColumn<casacore::uInt> scan_column_;casacore::ROScalarColumn<casacore::uInt> cycle_column_;casacore::ROScalarColumn<
      casacore::uInt> ifno_column_;casacore::ROScalarColumn<casacore::uInt> polno_column_;casacore::ROScalarColumn<casacore::uInt> beam_column_;casacore::ROScalarColumn<
      casacore::uInt> flagrow_column_;casacore::ROScalarColumn<casacore::Double> time_column_;casacore::ROScalarColumn<
      casacore::Double> interval_column_;casacore::ROScalarColumn<casacore::Int> srctype_column_;
  casacore::ArrayColumn<casacore::Float> data_column_;
  casacore::ArrayColumn<casacore::uChar> flag_column_;
  casacore::ArrayColumn<casacore::Double> direction_column_;
  casacore::ArrayColumn<casacore::Double> scanrate_column_;casacore::ROScalarColumn<casacore::String> fieldname_column_;
  casacore::ArrayColumn<casacore::Float> tsys_column_;casacore::ROScalarColumn<casacore::uInt> tcal_id_column_;casacore::ROScalarColumn<
      casacore::uInt> weather_id_column_;
  casacore::ArrayColumn<casacore::Float> tcal_column_;casacore::ROScalarColumn<casacore::Float> temperature_column_;casacore::ROScalarColumn<
      casacore::Float> pressure_column_;casacore::ROScalarColumn<casacore::Float> humidity_column_;casacore::ROScalarColumn<
      casacore::Float> wind_speed_column_;casacore::ROScalarColumn<casacore::Float> wind_direction_column_;
  casacore::Vector<casacore::uInt> sorted_rows_;
  ScantableFieldIterator::Product field_map_;
  ScantableFrequenciesIterator::Product num_chan_map_;
  std::map<casacore::uInt, casacore::uInt> tcal_id_map_;
  std::map<casacore::uInt, casacore::uInt> weather_id_map_;
  casacore::String pol_type_;

  casacore::Bool (Scantable2MSReader::*get_antenna_row_)(sdfiller::AntennaRecord &);
  casacore::Bool (Scantable2MSReader::*get_field_row_)(sdfiller::FieldRecord &);
  casacore::Bool (Scantable2MSReader::*get_observation_row_)(
      sdfiller::ObservationRecord &);
  casacore::Bool (Scantable2MSReader::*get_processor_row_)(sdfiller::ProcessorRecord &);
  casacore::Bool (Scantable2MSReader::*get_source_row_)(sdfiller::SourceRecord &);
  casacore::Bool (Scantable2MSReader::*get_spw_row_)(sdfiller::SpectralWindowRecord &);

  std::unique_ptr<ScantableFieldIterator> field_iter_;
  std::unique_ptr<ScantableFrequenciesIterator> freq_iter_;
  std::unique_ptr<ScantableSourceIterator> source_iter_;

  casacore::Bool getAntennaRowImpl(sdfiller::AntennaRecord &record);
  casacore::Bool getFieldRowImpl(sdfiller::FieldRecord &record);
  casacore::Bool getObservationRowImpl(sdfiller::ObservationRecord &record);
  casacore::Bool getProcessorRowImpl(sdfiller::ProcessorRecord &record);
  casacore::Bool getSourceRowImpl(sdfiller::SourceRecord &record);
  casacore::Bool getSpectralWindowRowImpl(sdfiller::SpectralWindowRecord &record);

  template<class _Record>
  casacore::Bool noMoreRowImpl(_Record &) {
    POST_START;POST_END;
    return false;
  }

  template<class _Iterator, class _Record, class _Func>
  casacore::Bool getRowImplTemplate(std::unique_ptr<_Iterator> &iter, _Record &record,
      _Func &func, typename _Iterator::Product *product = nullptr) {
    POST_START;

    if (!iter) {
      iter.reset(new _Iterator(*main_table_));
    }

    casacore::Bool more_data = iter->moreData();
    if (more_data) {
      iter->getEntry(record);
      iter->next();
    } else {
      // seems to be passed through all the table, deallocate iterator
      iter->getProduct(product);
      iter.reset(nullptr);
      // and then redirect function pointer to noMoreRowImpl
      func = &Scantable2MSReader::noMoreRowImpl<_Record>;
    }

    POST_END;

    return more_data;
  }
};

} //# NAMESPACE CASA - END

#endif /* SINGLEDISH_FILLER_SCANTABLE2MSREADER_H_ */
