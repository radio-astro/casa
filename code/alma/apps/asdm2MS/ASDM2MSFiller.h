// ASDM2MSFiller.h: implementation of a MeasurementSet's filler
// for Francois Viallefond & Frederic Badia ALMA Simulator
//
//  Copyright (C) 2001
//  OBSERVATOIRE DE PARIS - DEMIRM
//  Avenue Denfert Rochereau - 75014 - PARIS
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//
//
//////////////////////////////////////////////////////////////////////
#if !defined(ALMA_ASDM2MSFILLER_H)
#define ALMA_ASDM2MSFILLER_H
//# Includes

#include <casa/aips.h>
#include <casa/Utilities/Assert.h>
#include <tables/Tables.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSFeedColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSFlagCmdColumns.h>
#include <ms/MeasurementSets/MSHistoryColumns.h>
#include <ms/MeasurementSets/MSMainColumns.h>

#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPointingColumns.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSProcessorColumns.h>
#include <ms/MeasurementSets/MSSourceColumns.h>
#include <ms/MeasurementSets/MSStateColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

#include <ms/MeasurementSets/MSWeatherColumns.h>

#include <tables/Tables/StandardStMan.h>
#include <tables/Tables/TiledShapeStMan.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/OrderedMap.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/Muvw.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/MeasConvert.h>
#include "measures/Measures/Stokes.h"
#include <casa/BasicSL/Constants.h>
#include <casa/OS/File.h>
#include <casa/OS/Path.h>
#include <complex>

#include <vector>


using namespace casa;
using namespace std;

//# Forward Declarations

class TimeRange;
class MPosition;
class MeasFrame;
class MeasurementSet;
class MSMainColumns;

//
// A structure to define a range of rows in the Pointing table where the attribute overTheTop is defined and with which value.
//
struct s_overTheTop {
  int  start;   // The index of the first row of the range. 
  int  len;     // The number of consecutive rows in the range.
  bool value;   // The value of overTheTop in that range.
};

//
// Class timeMgr is a utility to help for the management
// of time in tables with TIME and INTERVAL columns
class timeMgr {
 private:
  int index;
  double startTime;

 public:
  timeMgr();
  timeMgr(int i, double t);
  void   setIndex(int i);
  void   setStartTime(double t);
  int    getIndex();
  double getStartTime();
};


// Class ddMgr is a utility to help for the management
// of DataDescription, SpectralWindow and Polarization ids.
// Here we provide enough space to store 100 values for 
// each quantity; this is very likeky far beyond the actual
// needs.
class ddMgr {
 private:
  int     numCorr[100];
  int     numChan[100];
  struct  {
    int polId;
    int swId;
  } dd[100];
  
 public:

  ddMgr();

  int setNumCorr(int i, int numChan);
  int setNumChan(int i, int numCorr);

  int getNumCorr(int i);
  int getNumChan(int i);

  int setDD(int i, int polId, int swId); 

  int getPolId(int i);
  int getSwId(int i);
};


 
// Class ASDM2MSFiller
class ASDM2MSFiller
{
 private:
  double         itsCreationTime;
  const char*    itsName;
  int            itsNumAntenna;
  int            itsNumChan;
  int            itsNumCorr;
  casa::MeasurementSet *itsMS;
  casa::MSMainColumns  *itsMSCol;
  /*
    Block<timeMgr> itsFeedTimeMgr;
    Block<timeMgr> itsPointingTimeMgr;
    Block<timeMgr> itsSyscalTimeMgr;
    Block<timeMgr> itsWeatherTimeMgr;
    Block<timeMgr> itsObservationTimeMgr;
  */
    
  String     itsMSPath;
  timeMgr* itsFeedTimeMgr;
  timeMgr* itsFieldTimeMgr;
  timeMgr* itsObservationTimeMgr;
  timeMgr* itsPointingTimeMgr;
  //OrderedMap<int, timeMgr> itsSourceTimeMgr;
  timeMgr* itsSourceTimeMgr;
  timeMgr* itsSyscalTimeMgr;
  timeMgr* itsWeatherTimeMgr;
    
  Bool     itsWithRadioMeters;     /* Are we building an ALMA MS ?*/
  Bool     itsFirstScan;
  uInt     itsMSMainRow;
  /*TiledDataStManAccessor itsImWgtAcc;*/
  Block<IPosition> itsDataShapes;

  int itsScanNumber;
  int itsNCat;
    
  ddMgr    itsDDMgr;

         
  int createMS(const char* msName, Bool complexData, Bool withCompression, Bool withCorrectedData=false);

  const char** getPolCombinations(int numCorr);
    
   
 public:  
  ASDM2MSFiller (const char*	name_,
		 double		creation_time_,
		 Bool		withRadioMeters,
		 Bool		complexData,
		 Bool		withCompression,
		 Bool		withCorrectedData=false);
  
  // Destructor
  ~ASDM2MSFiller();

  const casa::MeasurementSet* ms();

  int addAntenna(const char	*name_,
		 const char	*station_,
		 double		 lx_,
		 double		 ly_,
		 double		 lz_,
		 double		 offset_x_,
		 double		 offset_y_,
		 double		 offset_z_,
		 float		 dish_diam_);

  void addData (bool                      complexData,
		vector<double>            &time_,
		vector<int>               &antennaId1_,
		vector<int>               &antennaId2_,
		vector<int>               &feedId1_,
		vector<int>               &feedId2_,
		vector<int>               &dataDescId_,
		int                       processorId_,
		vector<int>               &fieldId_,
		vector<double>            &interval_,
		vector<double>            &exposure_,
		vector<double>            &timeCentroid_,
		int                       scanNumber_,
		int                       arrayId_,
		int                       observationId_,
		vector<int>               &stateId_,
		vector<double>            &uvw_,
		vector<vector<unsigned int> >      &dataShape_,
		vector<float *>           &uncorrectedData_,
		vector<float *>           &correctedData_,
		vector<unsigned int>      &flag_);

  void addData (bool                      complexData,
		vector<double>            &time_,
		vector<int>               &antennaId1_,
		vector<int>               &antennaId2_,
		vector<int>               &feedId1_,
		vector<int>               &feedId2_,
		vector<int>               &dataDescId_,
		int                       processorId_,
		vector<int>               &fieldId_,
		vector<double>            &interval_,
		vector<double>            &exposure_,
		vector<double>            &timeCentroid_,
		int                       scanNumber_,
		int                       arrayId_,
		int                       observationId_,
		vector<int>               &stateId_,
		vector<double>            &uvw_,
		vector<vector<unsigned int> >      &dataShape_,
		vector<float *>           &data_,
		vector<unsigned int>      &flag_);
  

	       
  int  addDataDescription(int spectral_window_id_,
			  int polarizarion_id_);

  int  addUniqueDataDescription(int spectral_window_id_,
				int polarizarion_id_);

  int  exists(char *path);
  String msPath();


  void addFeed(int      antenna_id_,
	       int      feed_id_,
	       int      spectral_window_id_,
	       double   time_,
	       double   interval_,
	       int      num_receptors_,
	       int      beam_id_,
	       double   beam_offset_[],
	       const    vector<string>& pol_type_,
	       double   polarization_responseR_[],
	       double   polarization_responseI_[],
	       double   position_[3],
	       double   receptor_angle_[]);
  
  void addField( const char   *name_,
		 const char   *code_,
		 double time_,
		 double delay_dir_[2],
		 double phase_dir_[2],
		 double reference_dir_[2],
		 int     source_id_);

  void addFlagCmd(double    time_,
		  double    interval_,
		  const char     *type_,
		  const char     *reason_,
		  int       level_,
		  int       severity_,
		  int       applied_,
		  const char     *command_);

  void addHistory( double time_,
		   int    observation_id_,
		   const char  *message_,
		   const char  *priority_,
		   const char  *origin_,
		   int    object_id_,
		   const char  *application_,
		   const char  *cli_command_,
		   const char  *app_parms_ );

  void addObservation(const char   *telescopeName_,
		      double startTime_,
		      double endTime_,
		      const char  *observer_,
		      const char  **log_,
		      const char  *schedule_type_,
		      const char  **schedule_,
		      const char  *project_,
		      double release_date_);


  void addPointing(int     antenna_id_,
		   double  time_,
		   double  interval_,
		   const char   *name_,
		   double  direction_[2],
		   double  target_[2],
		   double  pointing_offset_[2],
		   double  encoder_[2],
		   int     tracking_);

  void addPointingSlice(unsigned int                  n_row_,
			int*                          antenna_id_,
			double*                       time_,
			double*                       interval_,
			double*                       direction_,
			double*                       target_,
			double*                       pointing_offset_,
			double*                       encoder_,
			bool*                         tracking_,
			bool                          overTheTopExists4All_,
			bool*                         a_overTheTop_,
			const vector<s_overTheTop>&   v_overTheTop_);

  int  addPolarization(int num_corr_,
		       int corr_type_[],
		       int corr_product_[]);

  int addUniquePolarization(int num_corr_,
		       Stokes::StokesTypes corr_type_[],
		       int corr_product_[]);

  void addProcessor(const char *type_,
		    const char*sub_type_,
		    int  type_id_,
		    int  mode_id_);

  void addSource(int    source_id_,
		 double time_,
		 double interval_,
		 int    spectral_window_id_,
		 int    num_lines_,
		 const char  *name_,
		 int    calibration_group_,
		 const char  *code_,
		 double direction_[2],
		 double position_[2],
		 double proper_motion_[2],
		 const char  *transition_[],
		 double rest_frequency_[],
		 double sysvel_[]);
		 

  int  addSpectralWindow(int    num_chan_,
			 const char  *name_,
			 double ref_frequency_,
			 double chan_freq_[],
			 double chan_width_[],
			 int    meas_freq_ref_,
			 double effective_bw_[],
			 double resolution_[],
			 double total_bandwidth_,
			 int    net_sideband_,
			 int    bbc_no_,
			 int    if_conv_chain_,
			 int    freq_group_,
			 const char  *freq_group_name_,
			 int    num_assoc_,
			 int    assoc_spectral_window_[],
			 char** assoc_nature_);

int  addUniqueState(Bool sig_,
		      Bool ref_,
		      double cal_,
		      double load_,
		      unsigned int sub_scan_,
		      const char* obs_mode_,
		      Bool flag_row_);


  void addState(Bool   sig_,
		Bool   ref_,
		double cal_,
		double load_,
		int    sub_scan_,
		const char   *obs_mode_);

  void addWeather(int    antennaId_,
		  double time_,
		  double interval_,
		  float  pressure_,
		  bool   pressure_flag_,
		  float  rel_humidity_,
		  bool   rel_humidity_flag_,
		  float  temperature_,
		  bool   temperature_flag_,
		  float  wind_direction_,
		  bool   wind_direction_flag_,
		  float  wind_speed_,
		  bool   wind_speed_flag_,
		  bool   has_dew_point_,
		  float  dew_point_,
		  bool   dew_point_flag_,
		  int    wx_station_id_,
		  double* wx_station_position_);

  void end(double time_);
};
#endif
  
