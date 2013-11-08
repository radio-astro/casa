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
#include <ms/MeasurementSets/MSSysCalColumns.h>
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
#include <measures/TableMeasures/TableMeasValueDesc.h>
#include <measures/TableMeasures/TableMeasOffsetDesc.h>
#include <measures/TableMeasures/TableMeasRefDesc.h>
#include <measures/TableMeasures/TableMeasDesc.h>
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
  unsigned int  start;   // The index of the first row of the range. 
  unsigned int  len;     // The number of consecutive rows in the range.
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
  const string   itsName;
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

         
  int createMS(const string& msName, 
               bool complexData, 
               bool withCompression, 
               const string& telName, 
               int maxNumCorr,
               int maxNumChan,
               bool withCorrectedData=false,
	       bool useAsdmStMan4DATA=false);

  const char** getPolCombinations(int numCorr);
    
  static map<string, MDirection::Types> string2MDirection;
  static map<string, MDirection::Types> string2MDirectionInit();
   
 public:  
  ASDM2MSFiller (const string&	name_,
		 double		creation_time_,
		 bool		withRadioMeters,
		 bool		complexData,
		 bool		withCompression,
                 const string&  telName, 
                 int            intintmaxNumCorr,
                 int            maxNumChan,
		 bool		withCorrectedData=false,
		 bool           useAsdmStMan4DATA=false);
  
  // Destructor
  ~ASDM2MSFiller();

  const casa::MeasurementSet* ms();

  int addAntenna(const string&	 name_,
		 const string&	 station_,
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
		int                       fieldId_,
		vector<double>            &interval_,
		vector<double>            &exposure_,
		vector<double>            &timeCentroid_,
		int                       scanNumber_,
		int                       arrayId_,
		int                       observationId_,
		vector<int>               &stateId_,
		vector<pair<int, int> >   &nChanNPol_,
		vector<double>            &uvw_);

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
	       vector<double> &   beam_offset_,
	       vector<std::string> & pol_type_,
	       vector<std::complex<float> > & polarization_response_,
	       vector<double>&   position_,  // Must be a 3 elements vector !!!
	       vector<double>&   receptor_angle_);
  
  void addField( const string&			name_,
		 const string&			code_,
		 double				time_,
		 int                            num_poly_,
		 vector<vector<double> >&	delay_dir_,
		 vector<vector<double> >&	phase_dir_,
		 vector<vector<double> >&	reference_dir_,
		 const string&			direction_code_,
		 int				source_id_);

  void updateEphemerisIdInField(vector<pair<int, int> >& idxEphemerisId_v);

  void addFlagCmd(double	time_,
		  double	interval_,
		  const string& type_,
		  const string& reason_,
		  int		level_,
		  int		severity_,
		  int		applied_,
		  string&	command_);

  void addHistory( double		time_,
		   int			observation_id_,
		   const string&	message_,
		   const string&	priority_,
		   const string&	origin_,
		   int			object_id_,
		   const string&	application_,
		   const string&	cli_command_,
		   const string&	app_parms_ );

  void addObservation(const string&		telescopeName_,
		      double			startTime_,
		      double			endTime_,
		      const string&		observer_,
		      const vector<string>&	log_,
		      const string&		schedule_type_,
		      const vector<string>&	schedule_,
		      const string&		project_,
		      double			release_date_);

  void addPointingSlice(unsigned int                  n_row_,
			vector<int>&                  antenna_id_,
			vector<double>&               time_,
			vector<double>&               interval_,
			vector<double>&               direction_,
			vector<double>&               target_,
			vector<double>&               pointing_offset_,
			vector<double>&               encoder_,
			vector<bool>&                 tracking_,
			bool                          overTheTopExists4All_,
			vector<bool>&                 v_overTheTop_,
			vector<s_overTheTop>&         v_s_overTheTop_);

  int  addPolarization(int num_corr_,
		       vector<int>& corr_type_,
		       vector<int>& corr_product_);

  int addUniquePolarization(int num_corr_,
			    //			    const vector<Stokes::StokesTypes>& corr_type_,
			    const vector<int>& corr_type_,
			    const vector<int>& corr_product_);

  void addProcessor(string& type_,
		    string& sub_type_,
		    int  type_id_,
		    int  mode_id_);

  void addSource(int             source_id_,
		 double          time_,
		 double          interval_,
		 int             spectral_window_id_,
		 int             num_lines_,
		 string&         name_,
		 int             calibration_group_,
		 string&         code_,
		 vector<double>& direction_,
		 string&         direction_code_,
		 vector<double>& position_,
		 vector<double>& proper_motion_,
		 vector<string>& transition_,
		 vector<double>& rest_frequency_,
		 vector<double>& sysvel_);
		 
  int  addSpectralWindow(int			num_chan_,
			 const string&          name_,
			 double			ref_frequency_,
			 const vector<double>&	chan_freq_,
			 const vector<double>&	chan_width_,
			 int			meas_freq_ref_,
			 const vector<double>&	effective_bw_,
			 const vector<double>&	resolution_,
			 double			total_bandwidth_,
			 int			net_sideband_,
			 int			bbc_no_,
			 int			if_conv_chain_,
			 int			freq_group_,
			 const string&		freq_group_name_,
			 int			num_assoc_,
			 const vector<int>&	assoc_sp_id_,
			 const vector<string>&	assoc_nature_);

  int  addUniqueState(bool sig_,
		      bool ref_,
		      double cal_,
		      double load_,
		      unsigned int sub_scan_,
		      string& obs_mode_,
		      bool flag_row_);
  
  
  void addState(bool    sig_,
		bool    ref_,
		double  cal_,
		double  load_,
		int     sub_scan_,
		string& obs_mode_);
  
  void addSysCal(int    antenna_id,
		 int    feed_id,
		 int    spectral_window_id,
		 double time_,
		 double interval_,
		 int    numReceptor_,
		 int    numChan_,
		 pair<bool, vector<float> >& tcal_spectrum_pair,
		 pair<bool, bool>&           tcal_flag_pair,
		 pair<bool, vector<float> >& trx_spectrum_pair,
		 pair<bool, bool>&           trx_flag_pair,
		 pair<bool, vector<float> >& tsky_spectrum_pair,
		 pair<bool, bool>&           tsky_flag_pair,
		 pair<bool, vector<float> >& tsys_spectrum_pair,
		 pair<bool, bool>&           tsys_flag_pair,
		 pair<bool, vector<float> >& tant_spectrum_pair,
		 pair<bool, bool>&           tant_flag_pair,
		 pair<bool, vector<float> >& tant_tsys_spectrum_pair,
		 pair<bool, bool>&           tant_tsys_flag_pair);

 void addWeather(int				antenna_id_,
		  double			time_,
		  double			interval_,
		  const pair<bool, float>&	pressure_opt_,
		  const pair<bool, float>&	relHumidity_opt_,
		  const pair<bool, float>&	temperature_opt_,
		  const pair<bool, float>&	windDirection_opt_,
		  const pair<bool, float>&	windSpeed_opt_,
		  const pair<bool, float>&	dewPoint_opt_,
		  int				wx_station_id_,
		  vector<double>&		wx_station_position_);

  /**
   * Add one row in the MS CALDEVICE table.
   *
   * @param antennaId the index in the ANTENNA table of the antenna for which this row is defined.
   * @param feedId the index in the FEED table of the feeds for which this row is defined.
   * @param spectralWindowId the index in the SPECTRAL WINDOW table of the spectral window for which this row is defined.
   * @param time the midpoint time of measurement.
   * @param interval the interval of measurement.
   * @param numCalload the number of calibration loads.
   * @param calLoadNames a vector of strings.
   */
  void addCalDevice(int				antennaId,
		    int				feedId,
		    int				spectralWindowId,
		    double			time,
		    double			interval,
		    unsigned int		numCalLoad,
		    vector<string>		calloadNames,
		    unsigned int		numReceptor,
		    vector<vector<float> >&	calEff,
		    vector<vector<float> >&	noiseCal,
		    vector<double >&		temperatureLoad);

  /**
   * Adds one row in the MS SYSPOWER table.
   *
   *
   * @param antennaId the index in the ANTENNA table of the antenna for which this row is defined.
   * @param feedId the index in the FEED table of the feeds for which this row is defined.
   * @param spectralWindowId the index in the SPECTRAL WINDOW table of the spectral window for which this row is defined.
   * @param time the midpoint time of measurement.
   * @param interval the interval of measurement.
   * @param numReceptor a null value will be interpreted as "all the optional attributes" are absent, otherwise it will be considered as
   * as the number of useful values to read in the next three vectors. More precisely, for any of the parameters switchedPowerDifference, 
   * switchedPowerSum and requantizedGain, if its size is null then the parameter is considered as "absent" and ignored otherwise
   * its first numReceptor values will be copied into the MS CalDevice table corresponding field. If the size of one the parameters is not null
   * but smaller than numReceptor then the code will crash miserably.
   *
   * @param switchedPowerDifference a vector of float numbers containing the switched power differences in its numReceptor first elements on
   * the basis of one value per receptor. If the size of the vector is null then it'll be ignored.
   * @param switchedPowerSum a vector of float numbers containing the switched power sums in its numReceptor first elements on
   * the basis of one value per receptor. If the size of the vector is null then it'll be ignored. 
   * @param temperatureLoad a vector of float number containing the requantizer gains on the basis of one value per receptor. If the size
   * of the vector is null then it'll be ignored. 
   */
  void addSysPower(int			antennaId,
		   int			feedId,
		   int			spectralWindowId,
		   double		time,
		   double		interval,
		   unsigned int         numReceptor,
		   vector<float>&	switchedPowerDifference,
		   vector<float>&	switchedPowerSum,
		   vector<float>&	requantizerGain); 

  void addSysPowerSlice(unsigned int	nRow,
			vector<int>&    antennaId,
			vector<int>&	spectralWindowId,
			vector<int>&	feedId,
			vector<double>&	time,
			vector<double>&	interval,
			unsigned int    numReceptor,
			vector<float>&	switchedPowerDifference,
			vector<float>&	switchedPowerSum,
			vector<float>&	requantizerGain);

  void end(double time_);
};
#endif
  
