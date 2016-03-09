/*
 * Scantable2MSFiller.h
 *
 *  Created on: Jan 5, 2016
 *      Author: nakazato
 */

#ifndef SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_
#define SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_

#include <string>
#include <memory>
#include <map>

#include <singledish/Filler/DataAccumulator.h>
#include <singledish/Filler/SysCalRecord.h>
#include <singledish/Filler/WeatherRecord.h>
#include <singledish/Filler/FillerUtil.h>
#include <singledish/Filler/PThreadUtil.h>

#include <casacore/casa/OS/File.h>
#include <casacore/casa/OS/Path.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSMainColumns.h>
#include <casacore/ms/MeasurementSets/MSDataDescColumns.h>
#include <casacore/ms/MeasurementSets/MSSysCalColumns.h>
#include <casacore/ms/MeasurementSets/MSPointingColumns.h>
#include <casacore/ms/MeasurementSets/MSPolColumns.h>
#include <casacore/ms/MeasurementSets/MSFeedColumns.h>
#include <casacore/ms/MeasurementSets/MSStateColumns.h>
#include <casacore/ms/MeasurementSets/MSWeatherColumns.h>
#include <casacore/tables/Tables/TableRow.h>
#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/tables/Tables/ScalarColumn.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class DataAccumulator;

template<typename Reader>
class SingleDishMSFiller {
public:
  // static methods for parallel processing
  inline static void create_context();
  inline static void destroy_context();
  static void *consume(void *arg);
  static void *produce(void *arg);
  inline static void fillMainMT(SingleDishMSFiller<Reader> *filler);

  // constructor
  SingleDishMSFiller(std::string const &name);

  // destructor
  ~SingleDishMSFiller();

  // access to reader object
  Reader const &getReader() const {
    return *reader_;
  }

  std::string const &getDataName() const {
    return reader_->getName();
  }

  // top level method to fill MS by reading input data
  void fill();

  // save
  void save(std::string const &name);

private:
  // initialization
  void initialize();

  // finalization
  void finalize();

  // setup MS as Scratch table
  // The table will be non-Scratch when it is saved
  void setupMS();

  // fill tables that can be processed prior to main loop
  void fillPreProcessTables();

  // fill tables that must be processed after main loop
  void fillPostProcessTables();

  // fill MAIN table
  void fillMain();

  // flush accumulated data
  inline void flush(DataAccumulator &accumulator);

  void sortPointing();

  // Fill subtables
  // fill ANTENNA table
  void fillAntenna();

  // fill OBSERVATION table
  void fillObservation();

  // fill PROCESSOR table
  void fillProcessor();

  // fill SOURCE table
  void fillSource();

  // fill SOURCE table
  void fillField();

  // fill SPECTRAL_WINDOW table
  void fillSpectralWindow();

  // fill HISTORY table
  void fillHistory();

  // update POLARIZATION table
  // @param[in] corr_type polarization type list
  // @param[in] num_pol number of polarization components
  // @return polarization id
  inline Int updatePolarization(Vector<Int> const &corr_type,
      Int const &num_pol);

  // update DATA_DESCRIPTION table
  // @param[in] polarization_id polarization id
  // @param[in] spw_id spectral window id
  // @return data description id
  inline Int updateDataDescription(Int const &polarization_id,
      Int const &spw_id);

  // update STATE table
  // @param[in] subscan subscan number
  // @param[in] obs_mode observing mode string
  // @return state id
  inline Int updateState(Int const &subscan, String const &obs_mode);

  // update FEED table
  // @param[in] feed_id feed ID
  // @param[in] spw_id spectral window ID
  // @param[in] pol_type polarization type
  // @return feed row number
  inline Int updateFeed(Int const &feed_id, Int const &spw_id,
      String const &pol_type);

  // update POINTING table
  // @param[in] antenna_id antenna id
  // @param[in] time time stamp
  // @param[in] direction pointing direction
  inline Int updatePointing(Int const &antenna_id, Int const &feed_id,
      Double const &time, Double const &interval,
      Matrix<Double> const &direction);

  inline void updateWeather(Int const &antenna_id, Double const &time,
      Double const &interval, MSDataRecord const &data_record);

  inline void updateWeather(MSWeatherColumns &columns, uInt irow,
      WeatherRecord const &record);

  inline void updateSysCal(Int const &antenna_id, Int const &feed_id,
      Int const &spw_id, Double const &time, Double const &interval,
      MSDataRecord const &data_record);

  inline void updateSysCal(MSSysCalColumns &columns, uInt irow,
      SysCalRecord const &record);

  // update MAIN table
  // @param[in] fieldId field id
  // @param[in] feedId feed id
  // @param[in] dataDescriptionId data description id
  // @param[in] stateId state id
  // @param[in] mainSpec main table row specification except id
  inline void updateMain(Int const &antenna_id, Int field_id, Int feedId,
      Int dataDescriptionId, Int stateId, Int const &scan_number,
      Double const &time, MSDataRecord const &dataRecord);

  std::unique_ptr<MeasurementSet> ms_;
  std::unique_ptr<MSMainColumns> ms_columns_;
  std::unique_ptr<MSDataDescColumns> data_description_columns_;
  std::unique_ptr<MSFeedColumns> feed_columns_;
  std::unique_ptr<MSPointingColumns> pointing_columns_;
  std::unique_ptr<MSPolarizationColumns> polarization_columns_;
  std::unique_ptr<MSSysCalColumns> syscal_columns_;
  std::unique_ptr<MSStateColumns> state_columns_;
  std::unique_ptr<MSWeatherColumns> weather_columns_;
  std::unique_ptr<Reader> reader_;
  bool is_float_;
  String data_key_;

  // for POINTING table
  Int reference_feed_;
  std::map<Int, Vector<Double>> pointing_time_;
  std::map<Int, Double> pointing_time_max_;
  std::map<Int, Double> pointing_time_min_;
  Vector<uInt> num_pointing_time_;

  // for SYSCAL table
  std::vector<SysCalTableRecord> syscal_list_;

  // for STATE table
  std::vector<Int> subscan_list_;

  // for FEED table
  std::vector<Vector<String> *> polarization_type_pool_;

  // for WEATHER table
  std::vector<WeatherRecord> weather_list_;

  // Data storage to interact with DataAccumulator
  MSDataRecord record_;

}
;

} //# NAMESPACE CASA - END

#include <singledish/Filler/SingleDishMSFiller.tcc>

#endif /* SINGLEDISH_FILLER_SINGLEDISHMSFILLER_H_ */
