// #ifdef _OPENMP
// #include <omp.h>
// #endif 

#include	"stdio.h"			/* <stdio.h> */
#include	"stddef.h"			/* <stddef.h> */
#include        <math.h>
#include	"time.h"			/* <time.h> */
/*#include	"gipsyc.h" */			/* GIPSY definitions */

#if	defined(__sysv__)

#include	<sys/time.h>

#elif	defined(__bsd__)

#define		ftime	FTIME

#include	<sys/timeb.h>			/* from system */

#undef		ftime

extern	void	ftime( struct timeb * );	/* this is the funtion */

#else

#endif

#include <boost/foreach.hpp>

void	timer( double *cpu_time ,		/* cpu timer */
	       double *real_time ,		/* real timer */
	       int  *mode )			/* the mode */
{
  clock_t	tc;				/* clock time */
  double	ct;				/* cpu time in seconds */
  double	rt = 0.0;			/* real time in seconds */
#if	defined(__sysv__)
  struct timeval 	Tp;
  struct timezone	Tzp;
#elif	defined(__bsd__)
  struct timeb tr;				/* struct from ftime */
#else
#endif
  tc = clock( );				/* get clock time */
  ct = (double)(tc) / (double)CLOCKS_PER_SEC;	/* to seconds */
#if	defined(__sysv__)
  gettimeofday( &Tp, &Tzp );			/* get timeofday */
  rt = (double) Tp.tv_sec + 0.000001 * (double) Tp.tv_usec;
#elif	defined(__bsd__)
  ftime( &tr );				/* get real time */
  rt = (double) tr.time + 0.001 * (double) tr.millitm;	/* in seconds */
#else
#endif
  if (*mode) {					/* calculate difference */
    (*cpu_time)  = ct - (*cpu_time);		/* cpu time */
    (*real_time) = rt - (*real_time);		/* real time */
  } else {
    (*cpu_time)  = ct;			/* set cpu time */
    (*real_time) = rt;			/* set real time */
  }
}

#include "ASDM2MSFiller.h"
#include "synthesis/MSVis/SubMS.h"

using namespace casa;

// Methods of timeMgr class.
timeMgr::timeMgr() {
  index = -1;
  startTime = 0.0;
}

timeMgr::timeMgr(int i, double t) {
  index = i;
  startTime = t;
}

void   timeMgr::setIndex(int i) { index = i;}
void   timeMgr::setStartTime(double t) {startTime = t;}
int    timeMgr::getIndex() {return index;}
double timeMgr::getStartTime() {return startTime;}

// Methods of ddMgr class.
ddMgr::ddMgr() {
  int i;
  for (i=0; i<100; i++) {
    numCorr[i]  = 0;
    numChan[i]  = 0;
    dd[i].polId = -1;
    dd[i].swId  = -1;
  }
}

int ddMgr::setNumCorr(int i, int numCorr) {
  if ((i < 0) || (i > 100) || (numCorr <= 0)) {
    return -1;
  }
  else {
    this->numCorr[i] = numCorr;
    return numCorr;
  }
}


int ddMgr::setNumChan(int i, int numChan) {
  if ((i < 0) || (i >= 100) || (numChan <= 0)) {
    return -1;
  }
  else {
    this->numChan[i] = numChan;
    return numChan;
  }
}


int ddMgr::getNumCorr(int i) {
  if ((i<0) || (i >= 100)) {
    return -1;
  }
  else 
    return numCorr[this->dd[i].polId];
}


int ddMgr::getNumChan(int i) {
  if ((i<0) || (i >= 100)) {
    return -1;
  }
  else 
    return numChan[this->dd[i].swId];
}


int ddMgr::setDD(int i, int polId, int swId) {
  if ((i<0) || (i >= 100) ||
      (polId < 0) || (polId >= 100) ||
      (swId  < 0) || (swId  >= 100)) {
    return -1;
  }
  else {
    dd[i].polId = polId;
    dd[i].swId  = swId;
    return i;
  }
}
      
int ddMgr::getPolId(int i) {
  if ((i<0) || (i >= 100)) {
    return -1;
  }
  else {
    return dd[i].polId;
  }
}


int ddMgr::getSwId(int i) {
  if ((i<0) || (i >= 100)) {
    return -1;
  }
  else {
    return dd[i].swId;
  }
}


map<string, MDirection::Types> ASDM2MSFiller::string2MDirectionInit() {
  map<string, MDirection::Types> string2MDirection;
  string2MDirection.clear();

  string2MDirection["J2000"]     = MDirection::J2000; 	  
  string2MDirection["JMEAN"]     = MDirection::JMEAN; 	  
  string2MDirection["JTRUE"]     = MDirection::JTRUE; 	  
  string2MDirection["APP"]       = MDirection::APP; 	  
  string2MDirection["B1950"]     = MDirection::B1950; 	  
  string2MDirection["B1950_VLA"] = MDirection::B1950_VLA; 	
  string2MDirection["BMEAN"]     = MDirection::BMEAN; 	  
  string2MDirection["BTRUE"]     = MDirection::BTRUE; 	  
  string2MDirection["GALACTIC"]  = MDirection::GALACTIC;  	
  string2MDirection["HADEC"]     = MDirection::HADEC; 	  
  string2MDirection["AZELSW"]    = MDirection::AZELSW; 	  
  string2MDirection["AZELGEO"]   = MDirection::AZELGEO;   	
  string2MDirection["AZELSWGEO"] = MDirection::AZELSWGEO; 	
  string2MDirection["AZELNEGEO"] = MDirection::AZELNEGEO; 	  
  string2MDirection["JNAT"]      = MDirection::JNAT; 	  
  string2MDirection["ECLIPTIC"]  = MDirection::ECLIPTIC;  	
  string2MDirection["MECLIPTIC"] = MDirection::MECLIPTIC; 	
  string2MDirection["TECLIPTIC"] = MDirection::TECLIPTIC; 	
  string2MDirection["SUPERGAL"]  = MDirection::SUPERGAL;  	
  string2MDirection["ITRF"]      = MDirection::ITRF; 	  
  string2MDirection["TOPO"]      = MDirection::TOPO; 	  
  string2MDirection["ICRS"]      = MDirection::ICRS;      

  string2MDirection["MERCURY"] = MDirection::MERCURY; 
  string2MDirection["VENUS"]   = MDirection::VENUS;
  string2MDirection["MARS"]    = MDirection::MARS;
  string2MDirection["JUPITER"] = MDirection::JUPITER;
  string2MDirection["SATURN"]  = MDirection::SATURN;
  string2MDirection["URANUS"]  = MDirection::URANUS;
  string2MDirection["NEPTUNE"] = MDirection::NEPTUNE;
  string2MDirection["PLUTO"]   = MDirection::PLUTO;
  string2MDirection["SUN"]     = MDirection::SUN;
  string2MDirection["MOON"]    = MDirection::MOON;

  return string2MDirection;
}

map<string, MDirection::Types> ASDM2MSFiller::string2MDirection = ASDM2MSFiller::string2MDirectionInit();



// Methods of ASDM2MSFiller classe.
// The constructor
ASDM2MSFiller::ASDM2MSFiller(const string& name_,
			     double        creation_time_,
			     bool          withRadioMeters_,
			     bool          complexData,
			     bool          withCompression,
			     //bool          withCorrectedData):
			     const string& telname_,
                             int           maxNumCorr,
                             int           maxNumChan,
                             bool          withCorrectedData,
			     bool          useAsdmStMan4DATA):
  itsFeedTimeMgr(0),
  itsFieldTimeMgr(0),
  itsObservationTimeMgr(0),
  itsPointingTimeMgr(0),
  //itsSourceTimeMgr(timeMgr()),
  itsSourceTimeMgr(0),
  itsSyscalTimeMgr(0),
  itsWeatherTimeMgr(0),

  itsWithRadioMeters(withRadioMeters_),
  itsFirstScan(True),
  itsMSMainRow(0),
  itsDataShapes(0),
  itsNCat(3)
{
  int status;

  itsCreationTime = creation_time_;
   
  itsMS			= 0;
  itsMSCol		= 0;
  itsNumAntenna		= 0;
  itsObservationTimeMgr = new timeMgr[1]; 
  itsScanNumber         = 0;

  //cout << "About to call createMS" << endl;
  status = createMS(name_, 
                    complexData, 
                    withCompression, 
                    telname_, 
                    maxNumCorr,
                    maxNumChan,
                    withCorrectedData,
		    useAsdmStMan4DATA);
  //cout << "Back from call createMS" << endl;

}

// The destructor
ASDM2MSFiller::~ASDM2MSFiller() {
  ;
}

int ASDM2MSFiller::createMS(const string& msName,
			    bool complexData,
			    bool withCompression,
			    const string& telescopeName,
			    int maxNumCorr,
			    int maxNumChan,
			    bool withCorrectedData,
			    bool useAsdmStMan4DATA) {

  String aName(msName);

  // FLAG CATEGORY stuff.
  Vector<String>  cat(itsNCat);
  cat(0) = "FLAG_CMD";
  cat(1) = "ORIGINAL";
  cat(2) = "USER";


  //cout << "Entering createMS : measurement set = "<< aName <<"\n";


  const Int tileSizeKBytes = 1024;

  const Int nTileCorr = maxNumCorr;
  const Int nTileChan = maxNumChan;

  Int nTileRow;
  
  //needed to call setupMS
  String telescop(telescopeName);
  casa::Int inint = 0;
  Vector<MS::PredefinedColumns> colnames;
  if (complexData) {
    if (withCorrectedData) {
      colnames.resize(2);
      colnames(0)=MS::DATA;
      colnames(1)=MS::CORRECTED_DATA;
    }
    else {
      colnames.resize(1);
      colnames(0)= MS::DATA;
    }
  }
  else {
    colnames.resize(1);
    colnames(0)= MS::FLOAT_DATA;
  }

  //Use SubMS::setupMS() 

  itsMS = SubMS::setupMS(msName,
			 nTileChan,
			 nTileCorr,
			 telescop,
			 colnames,
			 inint,
			 withCompression,
			 (useAsdmStMan4DATA ? SubMS::USE_FOR_DATA : SubMS::DONT)
			 );

  if (! itsMS) {
    return False;
  }
  //cout << "Measurement Set just created, main table nrow=" << itsMS->nrow()<< endl;
  
  itsMSCol = new casa::MSMainColumns(*itsMS);

  // Feed (just missing column
  {
    TableDesc td;
    StandardStMan feedStMan("feed standard manager");
    MSFeed::addColumnToDesc(td, MSFeed::FOCUS_LENGTH);
    itsMS->feed().addColumn(td, feedStMan);
  }

  // Field
  // modified by TT : updated Field table
  // cout<<"update field table"<<endl;
  // And also add an Ephemeris column.
  
  const String& ephemerisId = MSField::columnName(MSField::EPHEMERIS_ID);
  itsMS->field().addColumn(ScalarColumnDesc<Int>(ephemerisId, "Ephemeris id, pointer to EPHEMERIS table"), False);

  {
    //TableDesc td = MSField::requiredTableDesc();
    //td.removeColumn("DELAY_DIR");
    //td.removeColumn("PHASE_DIR");
    //td.removeColumn("REFERENCE_DIR");
    TableDesc td;
    StandardStMan fldStMan("field standard manager");
    {
      //      ArrayColumnDesc<Double> cdMDir("PHASE_DIR", "variable phase dir",
      //				     IPosition(2, 2, 1), ColumnDesc::Direct);
      ArrayColumnDesc<Double> cdMDir("PHASE_DIR", "variable phase dir",
				     2);
      ScalarColumnDesc<Int> cdMDirRef("PhaseDir_Ref");
      td.addColumn(cdMDir);
      td.addColumn(cdMDirRef);
      TableMeasRefDesc tmrd(td, "PhaseDir_Ref");
      TableMeasValueDesc tmvd(td, "PHASE_DIR");
      TableMeasDesc<MDirection> tmdMDirection(tmvd, tmrd);
      tmdMDirection.write(td);
    }
    {
      //      ArrayColumnDesc<Double> cdMDir2("DELAY_DIR", "variable delay dir",
      //				      IPosition(2, 2, 1), ColumnDesc::Direct);
      ArrayColumnDesc<Double> cdMDir2("DELAY_DIR", "variable delay dir",
				      2);
      ScalarColumnDesc<Int> cdMDirRef2("DelayDir_Ref");
      td.addColumn(cdMDir2);
      td.addColumn(cdMDirRef2);
      TableMeasRefDesc tmrd2(td, "DelayDir_Ref");
      TableMeasValueDesc tmvd2(td, "DELAY_DIR");
      TableMeasDesc<MDirection> tmdMDirection2(tmvd2, tmrd2);
      tmdMDirection2.write(td);
    }
    {
      //      ArrayColumnDesc<Double> cdMDir3("REFERENCE_DIR", "variable reference dir",
      //				      IPosition(2, 2, 1), ColumnDesc::Direct);
      ArrayColumnDesc<Double> cdMDir3("REFERENCE_DIR", "variable reference dir",
				      2);
      ScalarColumnDesc<Int> cdMDirRef3("RefDir_Ref");
      td.addColumn(cdMDir3);
      td.addColumn(cdMDirRef3);
      TableMeasRefDesc tmrd3(td, "RefDir_Ref");
      TableMeasValueDesc tmvd3(td, "REFERENCE_DIR");
      TableMeasDesc<MDirection> tmdMDirection3(tmvd3, tmrd3);
      tmdMDirection3.write(td);
    }
    itsMS->field().removeColumn("DELAY_DIR");
    itsMS->field().removeColumn("PHASE_DIR");
    itsMS->field().removeColumn("REFERENCE_DIR");
    itsMS->field().addColumn(td,fldStMan);
  }


  // Pointing
  // need to add this (TT)
  {
    TableDesc td = MSPointing::requiredTableDesc();
    MSPointing::addColumnToDesc (td, MSPointing::POINTING_OFFSET);
    MSPointing::addColumnToDesc (td, MSPointing::ENCODER);
    MSPointing::addColumnToDesc (td, MSPointing::ON_SOURCE);
    MSPointing::addColumnToDesc (td, MSPointing::OVER_THE_TOP);
    SetupNewTable tabSetup(itsMS->pointingTableName(), td, Table::New);    
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::POINTING),
				      Table(tabSetup));
  }


  // Source
  // need to update with additional columns (TT) 
  {
    //TableDesc td = MSSource::requiredTableDesc();
    TableDesc td;
    StandardStMan srcStMan("source table col st man");
    MSSource::addColumnToDesc (td, MSSource::POSITION);
    MSSource::addColumnToDesc (td, MSSource::TRANSITION);
    MSSource::addColumnToDesc (td, MSSource::REST_FREQUENCY);
    MSSource::addColumnToDesc (td, MSSource::SYSVEL);
    itsMS->source().addColumn(td,srcStMan);
    //SetupNewTable tabSetup(itsMS->sourceTableName(),
    //			   td, Table::New);
    //itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
    //				      Table(tabSetup));
  }
  // Spectral Window
  // need to update to add extra columns (TT)
  {
    //TableDesc td = MSSpectralWindow::requiredTableDesc();
    TableDesc td;
    StandardStMan spwStMan("SpW optional column Standard Manager");
    MSSpectralWindow::addColumnToDesc (td, MSSpectralWindow::BBC_NO);
    MSSpectralWindow::addColumnToDesc (td, MSSpectralWindow::ASSOC_SPW_ID);
    MSSpectralWindow::addColumnToDesc (td, MSSpectralWindow::ASSOC_NATURE);
    itsMS->spectralWindow().addColumn(td,spwStMan);
    //SetupNewTable tabSetup(itsMS->spectralWindowTableName(),
    //			   td, Table::New);
    //itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::SPECTRAL_WINDOW),
    //				      Table(tabSetup));

  }


  // Syscal
  // need to add subtable (TT)
  {
    TableDesc td = MSSysCal::requiredTableDesc();
    MSSysCal::addColumnToDesc (td, MSSysCal::TCAL_SPECTRUM);
    MSSysCal::addColumnToDesc (td, MSSysCal::TRX_SPECTRUM);
    MSSysCal::addColumnToDesc (td, MSSysCal::TSKY_SPECTRUM);
    MSSysCal::addColumnToDesc (td, MSSysCal::TSYS_SPECTRUM);
    MSSysCal::addColumnToDesc (td, MSSysCal::TANT_SPECTRUM);
    MSSysCal::addColumnToDesc (td, MSSysCal::TANT_TSYS_SPECTRUM);
    MSSysCal::addColumnToDesc (td, MSSysCal::TCAL_FLAG);
    MSSysCal::addColumnToDesc (td, MSSysCal::TRX_FLAG);
    MSSysCal::addColumnToDesc (td, MSSysCal::TSKY_FLAG);
    MSSysCal::addColumnToDesc (td, MSSysCal::TSYS_FLAG);
    MSSysCal::addColumnToDesc (td, MSSysCal::TANT_FLAG);
    MSSysCal::addColumnToDesc (td, MSSysCal::TANT_TSYS_FLAG);
    SetupNewTable tabSetup(itsMS->sysCalTableName(), td, Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::SYSCAL),
				      Table(tabSetup));
  }
  // Weather
  {
    TableDesc td = MSWeather::requiredTableDesc();
    MSWeather::addColumnToDesc (td, MSWeather::PRESSURE);
    MSWeather::addColumnToDesc (td, MSWeather::PRESSURE_FLAG);
    MSWeather::addColumnToDesc (td, MSWeather::REL_HUMIDITY);
    MSWeather::addColumnToDesc (td, MSWeather::REL_HUMIDITY_FLAG);
    MSWeather::addColumnToDesc (td, MSWeather::TEMPERATURE);
    MSWeather::addColumnToDesc (td, MSWeather::TEMPERATURE_FLAG);
    MSWeather::addColumnToDesc (td, MSWeather::DEW_POINT);
    MSWeather::addColumnToDesc (td, MSWeather::DEW_POINT_FLAG);
    MSWeather::addColumnToDesc (td, MSWeather::WIND_DIRECTION);
    MSWeather::addColumnToDesc (td, MSWeather::WIND_DIRECTION_FLAG);
    MSWeather::addColumnToDesc (td, MSWeather::WIND_SPEED);
    MSWeather::addColumnToDesc (td, MSWeather::WIND_SPEED_FLAG);
    td.addColumn(ScalarColumnDesc<int>("NS_WX_STATION_ID",
				       "An identifier to identify uniquely a weather station"));
    td.addColumn(ArrayColumnDesc<double>("NS_WX_STATION_POSITION",
					 "The position of the station",
					 IPosition(1,3),
					 ColumnDesc::Direct));
		 
    SetupNewTable tabSetup(itsMS->weatherTableName(), td, Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::WEATHER),
				      Table(tabSetup));   
  }


  //
  // New Tables.
  //

  // SYSPOWER.
  //
  {
    TableDesc tableDesc_;
    String name_ = "SYSPOWER";
    
    //
    // Key columns.
    //
    tableDesc_.comment() = "System calibration from Cal diode demodulation (EVLA).";
    tableDesc_.addColumn(ScalarColumnDesc<Int>("ANTENNA_ID", "Antenna identifier."));
    tableDesc_.addColumn(ScalarColumnDesc<Int>("FEED_ID", "Feed's index."));
    tableDesc_.addColumn(ScalarColumnDesc<Int>("SPECTRAL_WINDOW_ID", "Spectral window identifier."));
    tableDesc_.addColumn(ScalarColumnDesc<Double>("TIME", "Midpoint of time measurement."));
    tableDesc_.addColumn(ScalarColumnDesc<Double>("INTERVAL", "Interval of measurement."));
    
    //
    // Data columns.
    //
    tableDesc_.addColumn(ArrayColumnDesc<Float>("SWITCHED_DIFF", "Switched power difference (cal on - off)."));
    tableDesc_.addColumn(ArrayColumnDesc<Float>("SWITCHED_SUM", "Switched power sum (cal on + off)."));
    tableDesc_.addColumn(ArrayColumnDesc<Float>("REQUANTIZER_GAIN", "Requantizer gain."));
    
    SetupNewTable tableSetup(itsMS->tableName() + "/" + name_,
			     tableDesc_,
			     Table::New);
    itsMS->rwKeywordSet().defineTable(name_, Table(tableSetup));
  }
  
  itsMS->rwKeywordSet().asTable("SYSPOWER").flush();

  // CALDEVICE.
  //
  {
    TableDesc tableDesc_;
    String name_ = "CALDEVICE";

    //
    // Key columns.
    //
    tableDesc_.comment() = "An immediate transcription of the ASDM's CalDevice table.";
    tableDesc_.addColumn(ScalarColumnDesc<Int>("ANTENNA_ID","Antenna's identifier"));
    tableDesc_.addColumn(ScalarColumnDesc<Int>("FEED_ID", "Feed's index"));
    tableDesc_.addColumn(ScalarColumnDesc<Int>("SPECTRAL_WINDOW_ID", "Spectral window identifier."));
    tableDesc_.addColumn(ScalarColumnDesc<Double>("TIME", "Midpoint of time measurement."));
    tableDesc_.addColumn(ScalarColumnDesc<Double>("INTERVAL", "Interval of measurement."));

    //
    // Data columns.
    //
    tableDesc_.addColumn(ScalarColumnDesc<Int>("NUM_CAL_LOAD", "Number of calibration loads."));
    tableDesc_.addColumn(ArrayColumnDesc<String>("CAL_LOAD_NAMES", "Calibration load names."));
    tableDesc_.addColumn(ScalarColumnDesc<Int>("NUM_RECEPTOR", "Number of receptors."));
    tableDesc_.addColumn(ArrayColumnDesc<Float>("NOISE_CAL", "Equivalent temperatures of the noise sources (TCAL for EVLA)."));
    tableDesc_.addColumn(ArrayColumnDesc<Float>("CAL_EFF", "Calibration efficiencies (one per receptor per load)."));
    tableDesc_.addColumn(ArrayColumnDesc<Double>("TEMPERATURE_LOAD", "Physical."));

    SetupNewTable tableSetup(itsMS->tableName() + "/" + name_,
			     tableDesc_,
			     Table::New);
    itsMS->rwKeywordSet().defineTable(name_, Table(tableSetup));    
  }

  itsMS->initRefs(True);
  MSPointing& mspointing = itsMS -> pointing();
  MSPointingColumns mspointingCol(mspointing);
  /**
   * Force the direction reference to AZEL
   * This will have to be made more dynamic later !
   */
  string dirref = "AZELGEO";
  mspointingCol.direction().rwKeywordSet().asrwRecord("MEASINFO").define("Ref", dirref);
  mspointingCol.target().rwKeywordSet().asrwRecord("MEASINFO").define("Ref", dirref);
  mspointingCol.pointingOffset().rwKeywordSet().asrwRecord("MEASINFO").define("Ref", dirref);
  mspointingCol.encoder().rwKeywordSet().asrwRecord("MEASINFO").define("Ref", dirref);
  
  itsMS->initRefs();

  itsMS->flush();


  //cout << "\n";
  {
    Path tmpPath(aName);
    Path tmpPath1(tmpPath);
    
    String expanded = tmpPath1.expandedName();
    Path tmpPath2(expanded);
    const String absolute = tmpPath2.absoluteName();
    itsMSPath = absolute;
  }

  return True;
}

const char** ASDM2MSFiller::getPolCombinations(int numCorr) {
  static const char* p1[] = {"RR", 0};
  static const char* p2[] = {"RR", "LL", 0};
  static const char* p4[] = {"RR", "LR", "RL", "LL", 0};

  if (numCorr == 1) {
    return p1;
  }
  else if (numCorr == 2) {
    return p2;
  }
  else {
    return p4;
  } 
}


// Insert records in the table ANTENNA
int ASDM2MSFiller::addAntenna( const string& name_,
			       const string& station_,
			       double lx_,
			       double ly_,
			       double lz_,
			       double offset_x_,
			       double offset_y_,
			       double offset_z_,
			       float  dish_diam_) {

  uInt crow;
  //itsNumAntenna = num_antenna_;
  itsNumAntenna++;

  // cout << "addAntenna : entering \n";

  MSAntenna msant = itsMS -> antenna();
  MSAntennaColumns msantCol(msant);

  // cout << "addAntenna : loop over antennas \n";
  crow = msant.nrow();

  Vector<Double>  antXYZ(3);
  Vector<Double>  antOffset(3);
  msant.addRow();
  msantCol.name().put(crow, String(name_));
  msantCol.station().put(crow, String(station_));
  msantCol.type().put(crow, String("GROUND-BASED"));
  msantCol.mount().put(crow, String("ALT-AZ"));

  antXYZ(0) = lx_;
  antXYZ(1) = ly_;
  antXYZ(2) = lz_;
  msantCol.position().put(crow, antXYZ);
  
  antOffset(0) = offset_x_;
  antOffset(1) = offset_y_;
  antOffset(2) = offset_z_;
  
  msantCol.offset().put(crow, antOffset);
  msantCol.dishDiameter().put(crow, dish_diam_);
  msantCol.flagRow().put(crow, False);
  
  msant.flush(True);
  // cout << "addAntenna : table flushed \n";
  // cout << "addAntenna : exiting \n";
  // cout << "\n";
  return crow;
}

const casa::MeasurementSet* ASDM2MSFiller::ms() { return itsMS; }


void ASDM2MSFiller::addData (bool                      complexData,
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
			     vector<double>            &uvw_){
  
  unsigned int theSize = time_.size();
  Bool *flag_row__  = new Bool[theSize];
  for (unsigned int i = 0; i < theSize; i++) {
    flag_row__[i] = 0;
  }

  
  Vector<Double> time(IPosition(1, theSize), &time_.at(0), SHARE);
  Vector<Int>    antenna1(IPosition(1, theSize), &antennaId1_.at(0), SHARE);
  Vector<Int>    antenna2(IPosition(1, theSize), &antennaId2_.at(0), SHARE);
  Vector<Int>    feed1(IPosition(1, theSize), &feedId1_.at(0) , SHARE);
  Vector<Int>    feed2(IPosition(1, theSize), &feedId2_.at(0), SHARE);
  Vector<Int>    dataDescriptionId(IPosition(1, theSize), &dataDescId_.at(0), SHARE);
  Vector<Int>    processorId(IPosition(1, theSize), processorId_);
  Vector<Int>    fieldId(IPosition(1, theSize), fieldId_);
  Vector<Double> interval(IPosition(1, theSize), &interval_.at(0), SHARE);
  Vector<Double> exposure(IPosition(1, theSize), &exposure_.at(0), SHARE);
  Vector<Double> timeCentroid(IPosition(1, theSize), &timeCentroid_.at(0), SHARE);
  Vector<Int>    scanNumber(IPosition(1, theSize), scanNumber_);
  Vector<Int>    arrayId(IPosition(1, theSize), arrayId_);
  Vector<Int>    observationId(IPosition(1, theSize), observationId_);
  Vector<Int>    stateId(IPosition(1, theSize), &stateId_.at(0), SHARE);
  Matrix<Double> uvw(IPosition(2, 3, theSize), &uvw_.at(0), SHARE);
  Vector<Bool>   flagRow(IPosition(1, theSize), flag_row__, SHARE);

  // UVW takes its contents directly from the argument uvw_.
  // Matrix<Double> uvw(IPosition(2, 3, nBaselines), uvw_, SHARE);
  
  // The data column will be constructed row after row.
  // The sigma               "
  // The weight              "
  // The flag                "
  
    
  // Define  a slicer to write blocks of values in each column of the main table.
  Slicer slicer(IPosition(1,itsMSMainRow),
		IPosition(1, (itsMSMainRow+theSize-1)),
		Slicer::endIsLast);


  // Let's create nBaseLines empty rows into the main table.
  itsMS->addRow(theSize);    

  // Now it's time to write all these Arrays into the relevant columns.
  itsMSCol->time().putColumnRange(slicer, time);   
  itsMSCol->antenna1().putColumnRange(slicer, antenna1); 
  itsMSCol->antenna2().putColumnRange(slicer, antenna2);
  itsMSCol->feed1().putColumnRange(slicer, feed1);
  itsMSCol->feed2().putColumnRange(slicer, feed2);
  itsMSCol->dataDescId().putColumnRange(slicer, dataDescriptionId);
  itsMSCol->processorId().putColumnRange(slicer, processorId);
  itsMSCol->fieldId().putColumnRange(slicer, fieldId);               
  itsMSCol->interval().putColumnRange(slicer, interval); 
  itsMSCol->exposure().putColumnRange(slicer, exposure);
  itsMSCol->timeCentroid().putColumnRange(slicer, timeCentroid);
  itsMSCol->scanNumber().putColumnRange(slicer, scanNumber);
  itsMSCol->arrayId().putColumnRange(slicer, arrayId);
  itsMSCol->observationId().putColumnRange(slicer, observationId);
  itsMSCol->stateId().putColumnRange(slicer, stateId);
  itsMSCol->uvw().putColumnRange(slicer, uvw);
  itsMSCol->flagRow().putColumnRange(slicer, flagRow); 

  int cRow0 = 0;
  for (unsigned int cRow = itsMSMainRow; cRow < itsMSMainRow+theSize; cRow++) {      
    int numChan = nChanNPol_[cRow0].first;
    int numCorr = nChanNPol_[cRow0].second;

    Vector<float>   ones(IPosition(1, numCorr), 1.0);

    // Sigma and Weight set to arrays of 1.0
    itsMSCol->sigma().put(cRow, ones);
    itsMSCol->weight().put(cRow, ones);

    // The flag cell (an array) is put at false.
    itsMSCol->flag().put(cRow, Matrix<Bool>(IPosition(2, numCorr, numChan), false));
    cRow0++;
  }

#if 0  
  // All the columns that could not be written in one shot are now filled row by row.
  //Matrix<Complex> uncorrected_data;
  Matrix<Complex> uncorrected_data;
  Matrix<Complex> corrected_data;
  Matrix<Float>   float_data;
  Matrix<Bool>    flag;

  int cRow0 = 0;
  for (unsigned int cRow = itsMSMainRow; cRow < itsMSMainRow+theSize; cRow++) {      
    int numCorr = dataShape_.at(cRow0).at(0);
    int numChan = dataShape_.at(cRow0).at(1);

    Vector<float>   ones(IPosition(1, numCorr), 1.0);

    //@pragma omp ordered
    {
    if (complexData) {
      // Uncorrected data
      if ( uncorrectedData_.at(cRow0) != 0) {
	uncorrected_data.takeStorage(IPosition(2, numCorr, numChan), (Complex *)(uncorrectedData_.at(cRow0)), COPY);
      }
      else {
	uncorrected_data.resize(numCorr, numChan);
	uncorrected_data = 0.0;
      }
      itsMSCol->data().put(cRow, uncorrected_data);

      // Corrected data
      if ( correctedData_.at(cRow0) != 0) {
	corrected_data.takeStorage(IPosition(2, numCorr, numChan), (Complex *)(correctedData_.at(cRow0)), COPY);
      }
      else {
	corrected_data.resize(numCorr, numChan);
	corrected_data = 0.0;
      }
      itsMSCol->correctedData().put(cRow, corrected_data);
    }
    else {
      // Float data.
      float_data.takeStorage(IPosition(2, numCorr, numChan), uncorrectedData_.at(cRow0), SHARE);
      itsMSCol->floatData().put(cRow, float_data);
    }
    }

    // Sigma and Weight set to arrays of 1.0
    itsMSCol->sigma().put(cRow, ones);
    itsMSCol->weight().put(cRow, ones);
    // The flag cell (an array) is put at false.

    itsMSCol->flag().put(cRow, Matrix<Bool>(IPosition(2, numCorr, numChan), false));
    cRow0++;
  }
#endif
  // Don't forget to increment itsMSMainRow.
  itsMSMainRow += theSize;

  
  // Flush
  // itsMS->flush();   

  delete[] flag_row__;
}

void ASDM2MSFiller::addData (bool                      complexData,
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
			     vector<unsigned int>      &flag_) {
  
  //cout << "Entering addData" << endl;

  unsigned int theSize = time_.size();
  Bool *flag_row__  = new Bool[theSize];
  for (unsigned int i = 0; i < theSize; i++) {
    flag_row__[i]              = flag_.at(i)==0?false:true;
  }

  Vector<Double> time(IPosition(1, theSize), &time_.at(0), SHARE);
  Vector<Int>    antenna1(IPosition(1, theSize), &antennaId1_.at(0), SHARE);
  Vector<Int>    antenna2(IPosition(1, theSize), &antennaId2_.at(0), SHARE);
  Vector<Int>    feed1(IPosition(1, theSize), &feedId1_.at(0) , SHARE);
  Vector<Int>    feed2(IPosition(1, theSize), &feedId2_.at(0), SHARE);
  Vector<Int>    dataDescriptionId(IPosition(1, theSize), &dataDescId_.at(0), SHARE);
  Vector<Int>    processorId(IPosition(1, theSize), processorId_);
  Vector<Int>    fieldId(IPosition(1, theSize), &fieldId_.at(0), SHARE);
  Vector<Double> interval(IPosition(1, theSize), &interval_.at(0), SHARE);
  Vector<Double> exposure(IPosition(1, theSize), &exposure_.at(0), SHARE);
  Vector<Double> timeCentroid(IPosition(1, theSize), &timeCentroid_.at(0), SHARE);
  Vector<Int>    scanNumber(IPosition(1, theSize), scanNumber_);
  Vector<Int>    arrayId(IPosition(1, theSize), arrayId_);
  Vector<Int>    observationId(IPosition(1, theSize), observationId_);
  Vector<Int>    stateId(IPosition(1, theSize), &stateId_.at(0), SHARE);
  Matrix<Double> uvw(IPosition(2, 3, theSize), &uvw_.at(0), SHARE);
  Vector<Bool>   flagRow(IPosition(1, theSize), flag_row__, SHARE);

  /*
    Matrix<Double> uvw(IPosition(2, 3, theSize), uvw__, SHARE);
  */
  
  // UVW takes its contents directly from the argument uvw_.
  // Matrix<Double> uvw(IPosition(2, 3, nBaselines), uvw_, SHARE);
  
  // The data column will be constructed row after row.
  // The sigma               "
  // The weight              "
  // The flag                "
  
    
  // Define  a slicer to write blocks of values in each column of the main table.
  Slicer slicer(IPosition(1,itsMSMainRow),
		IPosition(1, (itsMSMainRow+theSize-1)),
		Slicer::endIsLast);
    
  // Let's create nBaseLines empty rows into the main table.
  itsMS->addRow(theSize);
    
  // Now it's time to write all these Arrays into the relevant columns.

  itsMSCol->time().putColumnRange(slicer, time);
  itsMSCol->antenna1().putColumnRange(slicer, antenna1);
  itsMSCol->antenna2().putColumnRange(slicer, antenna2);
  itsMSCol->feed1().putColumnRange(slicer, feed1);
  itsMSCol->feed2().putColumnRange(slicer, feed2);
  itsMSCol->dataDescId().putColumnRange(slicer, dataDescriptionId);
  itsMSCol->processorId().putColumnRange(slicer, processorId);
  itsMSCol->fieldId().putColumnRange(slicer, fieldId);
  itsMSCol->interval().putColumnRange(slicer, interval);
  itsMSCol->exposure().putColumnRange(slicer, exposure);
  itsMSCol->timeCentroid().putColumnRange(slicer, timeCentroid);
  itsMSCol->scanNumber().putColumnRange(slicer, scanNumber);
  itsMSCol->arrayId().putColumnRange(slicer, arrayId);
  itsMSCol->observationId().putColumnRange(slicer, observationId);
  itsMSCol->stateId().putColumnRange(slicer, stateId);
  itsMSCol->uvw().putColumnRange(slicer, uvw);
  itsMSCol->flagRow().putColumnRange(slicer, flagRow);

#if 1     
  // All the columns that could not be written in one shot are now filled row by row.
  //Matrix<Complex> uncorrected_data;
  Matrix<Complex> uncorrected_data;
  Matrix<Complex> corrected_data;
  Matrix<Float>   float_data;
  Matrix<Bool>    flag;

  int cRow0 = 0;
  for (unsigned int cRow = itsMSMainRow; cRow < itsMSMainRow+theSize; cRow++) {      
    int numCorr = dataShape_.at(cRow0).at(0);
    int numChan = dataShape_.at(cRow0).at(1);

    Vector<float>   ones(IPosition(1, numCorr), 1.0);

    //@pragma omp ordered
    {
    if (complexData) {
      // Uncorrected data
      if ( uncorrectedData_.at(cRow0) != 0) {
	uncorrected_data.takeStorage(IPosition(2, numCorr, numChan), (Complex *)(uncorrectedData_.at(cRow0)), COPY);
      }
      else {
	uncorrected_data.resize(numCorr, numChan);
	uncorrected_data = 0.0;
      }
      itsMSCol->data().put(cRow, uncorrected_data);

      // Corrected data
      if ( correctedData_.at(cRow0) != 0) {
	corrected_data.takeStorage(IPosition(2, numCorr, numChan), (Complex *)(correctedData_.at(cRow0)), COPY);
      }
      else {
	corrected_data.resize(numCorr, numChan);
	corrected_data = 0.0;
      }
      itsMSCol->correctedData().put(cRow, corrected_data);
    }
    else {
      // Float data.
      float_data.takeStorage(IPosition(2, numCorr, numChan), uncorrectedData_.at(cRow0), SHARE);
      itsMSCol->floatData().put(cRow, float_data);
    }
    }

    // Sigma and Weight set to arrays of 1.0
    itsMSCol->sigma().put(cRow, ones);
    itsMSCol->weight().put(cRow, ones);
    // The flag cell (an array) is put at false.

    itsMSCol->flag().put(cRow, Matrix<Bool>(IPosition(2, numCorr, numChan), false));
    cRow0++;
  }
#endif
  // Don't forget to increment itsMSMainRow.
  itsMSMainRow += theSize;

  
  // Flush
  // itsMS->flush();   
  //cout << "Exiting addData" << endl;

  delete[] flag_row__;
}

void ASDM2MSFiller::addData (bool			 complexData,
			     vector<double>		&time_,
			     vector<int>		&antennaId1_,
			     vector<int>		&antennaId2_,
			     vector<int>		&feedId1_,
			     vector<int>		&feedId2_,
			     vector<int>		&dataDescId_,
			     int			 processorId_,
			     vector<int>		&fieldId_,
			     vector<double>		&interval_,
			     vector<double>		&exposure_,
			     vector<double>		&timeCentroid_,
			     int			 scanNumber_,
			     int			 arrayId_,
			     int			 observationId_,
			     vector<int>		&stateId_,
			     vector<double>		&uvw_,
			     vector<vector<unsigned int> >  &dataShape_,
			     vector<float *>		&data_,
			     vector<unsigned int>	&flag_,
			     vector<double> &		 weight_,
			     vector<double> &            sigma_) {
  
  //cout << "Entering addData" << endl;
  //printf("entering addData ...\n");

  unsigned int theSize = time_.size();
  Bool *flag_row__  = new Bool[theSize];
  for (unsigned int i = 0; i < theSize; i++) {
    flag_row__[i]              = flag_.at(i)==0?false:true;
  }

  Vector<Double> time(IPosition(1, theSize), &time_.at(0), SHARE);
  Vector<Int>    antenna1(IPosition(1, theSize), &antennaId1_.at(0), SHARE);
  Vector<Int>    antenna2(IPosition(1, theSize), &antennaId2_.at(0), SHARE);
  Vector<Int>    feed1(IPosition(1, theSize), &feedId1_.at(0) , SHARE);
  Vector<Int>    feed2(IPosition(1, theSize), &feedId2_.at(0), SHARE);
  Vector<Int>    dataDescriptionId(IPosition(1, theSize), &dataDescId_.at(0), SHARE);
  Vector<Int>    processorId(IPosition(1, theSize), processorId_);
  Vector<Int>    fieldId(IPosition(1, theSize), &fieldId_.at(0), SHARE);
  Vector<Double> interval(IPosition(1, theSize), &interval_.at(0), SHARE);
  Vector<Double> exposure(IPosition(1, theSize), &exposure_.at(0), SHARE);
  Vector<Double> timeCentroid(IPosition(1, theSize), &timeCentroid_.at(0), SHARE);
  Vector<Int>    scanNumber(IPosition(1, theSize), scanNumber_);
  Vector<Int>    arrayId(IPosition(1, theSize), arrayId_);
  Vector<Int>    observationId(IPosition(1, theSize), observationId_);
  Vector<Int>    stateId(IPosition(1, theSize), &stateId_.at(0), SHARE);
  //printf("storing uvw\n");
  Matrix<Double> uvw(IPosition(2, 3, theSize), &uvw_.at(0), SHARE);
  Vector<Bool>   flagRow(IPosition(1, theSize), flag_row__, SHARE);

  /*
    Matrix<Double> uvw(IPosition(2, 3, theSize), uvw__, SHARE);
  */
  
  // UVW takes its contents directly from the argument uvw_.
  // Matrix<Double> uvw(IPosition(2, 3, nBaselines), uvw_, SHARE);
  
  // The data column will be constructed row after row.
  // The sigma               "
  // The weight              "
  // The flag                "
  
    
  // Define  a slicer to write blocks of values in each column of the main table.
  Slicer slicer(IPosition(1,itsMSMainRow),
		IPosition(1, (itsMSMainRow+theSize-1)),
		Slicer::endIsLast);
    
  // Let's create nBaseLines empty rows into the main table.
  //printf("addRow \n");
  itsMS->addRow(theSize);
    
  // Now it's time to write all these Arrays into the relevant columns.

  itsMSCol->time().putColumnRange(slicer, time);
  itsMSCol->antenna1().putColumnRange(slicer, antenna1);
  itsMSCol->antenna2().putColumnRange(slicer, antenna2);
  itsMSCol->feed1().putColumnRange(slicer, feed1);
  itsMSCol->feed2().putColumnRange(slicer, feed2);
  itsMSCol->dataDescId().putColumnRange(slicer, dataDescriptionId);
  itsMSCol->processorId().putColumnRange(slicer, processorId);
  itsMSCol->fieldId().putColumnRange(slicer, fieldId);
  itsMSCol->interval().putColumnRange(slicer, interval);
  itsMSCol->exposure().putColumnRange(slicer, exposure);
  itsMSCol->timeCentroid().putColumnRange(slicer, timeCentroid);
  itsMSCol->scanNumber().putColumnRange(slicer, scanNumber);
  itsMSCol->arrayId().putColumnRange(slicer, arrayId);
  itsMSCol->observationId().putColumnRange(slicer, observationId);
  itsMSCol->stateId().putColumnRange(slicer, stateId);
  itsMSCol->uvw().putColumnRange(slicer, uvw);
  itsMSCol->flagRow().putColumnRange(slicer, flagRow);
#if 1     
  // All the columns that could not be written in one shot are now filled row by row.
  //Matrix<Complex> uncorrected_data;
  Matrix<Complex> data;
  Matrix<Float>   float_data;
  Matrix<Bool>    flag;

  int cRow0 = 0;
  //printf("itsMSMainRow+theSize=%d\n", itsMSMainRow+theSize);
  int maxrow = itsMSMainRow+theSize;
  for (unsigned int cRow = itsMSMainRow; cRow < itsMSMainRow+theSize; cRow++) {      
    int numCorr = dataShape_.at(cRow0).at(0);
    int numChan = dataShape_.at(cRow0).at(1);

    Vector<float>   weight(IPosition(1, numCorr), weight_[cRow-itsMSMainRow]);
    Vector<float>   sigma(IPosition(1, numCorr), sigma_[cRow-itsMSMainRow]);

    //@pragma omp ordered
    { 
      if (complexData) {
	data.resize(numCorr,numChan);
	data.takeStorage(IPosition(2, numCorr, numChan), (Complex *)(data_.at(cRow0)), COPY);
	itsMSCol->data().put(cRow, data);
	//printf("DONE writing complex data column %d %d\n", cRow, maxrow);
      }
      else {
	// Float data.
	float_data.takeStorage(IPosition(2, numCorr, numChan), data_.at(cRow0), SHARE);
	itsMSCol->floatData().put(cRow, float_data);
      }
    }
    // Sigma and Weight.
    itsMSCol->sigma().put(cRow, sigma);
    itsMSCol->weight().put(cRow, weight);
    // The flag cell (an array) is put at false.

    itsMSCol->flag().put(cRow, Matrix<Bool>(IPosition(2, numCorr, numChan), false));
    cRow0++;
  }
#endif
  // Don't forget to increment itsMSMainRow.
  itsMSMainRow += theSize;

  
  // Flush
  // itsMS->flush();   
  //cout << "Exiting addData" << endl;

  delete[] flag_row__;
}


// Add a record in the table DataDescription
int  ASDM2MSFiller::addDataDescription(int spectral_window_id_,
				       int polarization_id_) {
  uInt crow;
  MSDataDescription msdd = itsMS -> dataDescription();
  MSDataDescColumns msddCol(msdd);

  crow = msdd.nrow();
  msdd.addRow();
    
  msddCol.spectralWindowId().put(crow, spectral_window_id_);
  msddCol.polarizationId().put(crow, polarization_id_);
  msddCol.flagRow().put(crow, False);
    
  return crow;
}

// Add a record in the table DataDescription
int  ASDM2MSFiller::addUniqueDataDescription( int spectral_window_id_,
					      int polarization_id_ ) {
  uInt crow;
  MSDataDescription msdd = itsMS -> dataDescription();
  MSDataDescColumns msddCol(msdd);

  crow   = msdd.nrow();
  uInt i;
  for ( i = 0;
	i < crow
	  &&  ( msddCol.spectralWindowId()(i) != spectral_window_id_ ||
		msddCol.polarizationId()(i)   != polarization_id_ ) ; 
	i++ );
  
  if (i < crow) return i;

  itsDDMgr.setDD(crow, polarization_id_, spectral_window_id_);
  msdd.addRow();
    
  msddCol.spectralWindowId().put(crow, spectral_window_id_);
  msddCol.polarizationId().put(crow, polarization_id_);
  msddCol.flagRow().put(crow, False);
    
  msdd.flush(True);
  return crow;
}

// Test if the file corresponding to the given path exists.
// Returns : 3 if it exists and is writable.
//           2 if it does not exists and is writable.
//           1 if it exists and is non writable.
//           0 if it does not exist and is non writable.
int ASDM2MSFiller::exists( char *path) {

  int existFlag;
  int writableFlag;

  const String s(path);
  Path tmpPath(s);
  Path tmpPath1(tmpPath);

  String expanded = tmpPath1.expandedName();
  Path tmpPath2(expanded);
  const String absolute = tmpPath2.absoluteName();
  Path tmpPath3(absolute);
  const String dirname = tmpPath3.dirName();

  existFlag = File(absolute).exists()?1:0;
  
  if (existFlag) {
    writableFlag = Table::isWritable(absolute);
  }
  else {
    writableFlag = File(dirname).isWritable();
  }
  return existFlag + 2*writableFlag;
}


// msPath() -
// Returns the absolute name of the measurement set
//
String ASDM2MSFiller::msPath() {
  return itsMSPath;
}

// Add a record in the table FEED
void ASDM2MSFiller::addFeed(int      antenna_id_,
			    int      feed_id_,
			    int      spectral_window_id_,
			    double   time_,
			    double   interval_,
			    int      num_receptors_,
			    int      beam_id_,
			    vector<double>& beam_offset_,
			    vector<string>& pol_type_,
			    vector<std::complex<float> >& polarization_response_,
			    vector<double>&   position_,
			    vector<double>&   feed_angle_) {
  

  int crow;
  MSFeed msfeed = itsMS -> feed();
  MSFeedColumns msfeedCol(msfeed);

  //cout << "\nEntering addFeed";

  // Now we can put the values in a new row.
  Matrix<Double>   beamOffset(IPosition(2, 2, num_receptors_), &beam_offset_[0], SHARE);
  Matrix<Complex>  polResponse(IPosition(2, num_receptors_, num_receptors_), &polarization_response_[0], SHARE);
  Vector<Double>   position(IPosition(1, 3), &position_[0], SHARE);
  Vector<Double>   receptorAngle(IPosition(1, num_receptors_), &feed_angle_[0], SHARE);
  Vector<String>   polarizationType(num_receptors_);

  /*
  for (i=0; i<num_receptors_; i++) {
    beamOffset(0, i) = beam_offset_[2*i];
    beamOffset(1, i) = beam_offset_[2*i+1];
  }
  */
  /*
  int  i, j, k;
  k = 0;
  for (j=0; j<num_receptors_; j++) {
    for(i=0; i<num_receptors_; i++) {
      polResponse(i, j) = Complex(polarization_responseR_[k],
				  polarization_responseI_[k]);
      k++;
    }
  }
  */

  int i;
  /*
  for (i=0; i<3; i++) {
    position(i) = position_[i];
  }
  */
  for (i=0; i<num_receptors_; i++) {
    polarizationType(i) = String(pol_type_[i]);
  }

  crow = msfeed.nrow();

  //cout << "\naddFeed : time="  << time_;
  msfeed.addRow();
  msfeedCol.antennaId().put(crow, antenna_id_);
  msfeedCol.feedId().put(crow, feed_id_);
  msfeedCol.spectralWindowId().put(crow, spectral_window_id_);
  msfeedCol.time().put(crow,time_);
  msfeedCol.interval().put(crow,interval_);
  msfeedCol.numReceptors().put(crow, num_receptors_);
  msfeedCol.beamId().put(crow, -1);
  msfeedCol.beamOffset().put(crow, beamOffset);
  msfeedCol.polarizationType().put(crow, polarizationType);
  msfeedCol.polResponse().put(crow, polResponse);
  msfeedCol.position().put(crow, position);
  msfeedCol.receptorAngle().put(crow, receptorAngle); 

  msfeed.flush();
  // cout << "\n Exiting addFeed";
}
	     

// Adds a field record in the TABLE
void ASDM2MSFiller::addField(const string&		name_,
			     const string&		code_,
			     double			time_,
			     int                        num_poly_,
			     vector<vector<double> >&	delay_dir_,
			     vector<vector<double> >&	phase_dir_,
			     vector<vector<double> >&	reference_dir_,
			     const string&		direction_code_,
			     int                        source_id_) {
  uInt							crow;
  //cout << "\naddField : entering";
  Vector<MDirection>					delayDir(num_poly_);
  Vector<MDirection>					referenceDir(num_poly_);
  Vector<MDirection>					phaseDir(num_poly_);

  MSField msfield = itsMS -> field();
  MSFieldColumns msfieldCol(msfield);

  crow = msfield.nrow();
  msfield.addRow();

  msfieldCol.name().put(crow, name_);
  msfieldCol.code().put(crow, code_);
  msfieldCol.time().put(crow, time_);
  msfieldCol.numPoly().put(crow, 0);

  String s(direction_code_);
  if(s==""){
    // cout << "directionCode doesn't exist or is empty. Will try to determine it based on name." << endl;
    s=name_;
  }

  s.trim(); s.upcase();
  map<string, MDirection::Types>::const_iterator iter = string2MDirection.find(s);

  MDirection::Types directionReference = (iter == string2MDirection.end()) ? MDirection::J2000 : iter->second;

  for (unsigned int i = 0; i < num_poly_; i++) {
    delayDir(i)     = MDirection(Quantity(delay_dir_[i][0], "rad"), Quantity(delay_dir_[i][1], "rad"), directionReference);
    referenceDir(i) = MDirection(Quantity(reference_dir_[i][0], "rad"), Quantity(reference_dir_[i][1], "rad"), directionReference);
    phaseDir(i)     = MDirection(Quantity(phase_dir_[i][0], "rad"), Quantity(phase_dir_[i][1], "rad"), directionReference);
  }
  
  msfieldCol.numPoly().put(crow,num_poly_ - 1);

  msfieldCol.delayDirMeasCol().put(crow, delayDir);
  
  msfieldCol.referenceDirMeasCol().put(crow, referenceDir);
  
  msfieldCol.phaseDirMeasCol().put(crow, phaseDir);
  
  msfieldCol.sourceId().put(crow, source_id_);

  msfieldCol.ephemerisId().put(crow, -1);

  msfieldCol.flagRow().put(crow, False);
  //cout << "\naddField : exiting";
  msfield.flush();
}


void ASDM2MSFiller::updateEphemerisIdInField(vector<pair<int, int> >& idxEphemerisId_v) {
  MSField msfield = itsMS -> field();
  MSFieldColumns msfieldCol(msfield);

  typedef pair<int, int> pairOfInt_t;

  BOOST_FOREACH (pairOfInt_t p, idxEphemerisId_v) {
    msfieldCol.ephemerisId().put(p.first, p.second);
  }

  msfield.flush();
}	       

// Add a record in the table FLAG_CMD;
void ASDM2MSFiller::addFlagCmd(double		time_,
			       double		interval_,
			       const string&	type_,
			       const string&	reason_,
			       int		level_,
			       int		severity_,
			       int		applied_,
			       string&   command_) {
  uInt						crow;
  MSFlagCmd					msflagcmd = itsMS -> flagCmd();
  MSFlagCmdColumns				msflagcmdCol(msflagcmd);

  crow = msflagcmd.nrow();
  
  msflagcmd.addRow();
  msflagcmdCol.time().put(crow, time_);
  msflagcmdCol.interval().put(crow, interval_);
  msflagcmdCol.type().put(crow, "FLAG");
  msflagcmdCol.reason().put(crow, "DUMMY");
  msflagcmdCol.level().put(crow, 0);
  msflagcmdCol.severity().put(crow, 0);
  msflagcmdCol.applied().put(crow, 0);
  msflagcmdCol.command().put(crow, "");
  
  msflagcmd.flush(True);
  // cout << "\n";
}
			  
// Add a record in the table HISTORY
void ASDM2MSFiller::addHistory( double		time_,
				int		observation_id_,
				const string&	message_,
				const string&	priority_,
				const string&	origin_,
				int		object_id_,
				const string&	application_,
				const string&	cli_command_,
				const string& app_parms_ ) {

  uInt			crow;
  MSHistory		mshistory = itsMS -> history();
  MSHistoryColumns	mshistoryCol(mshistory);

  Vector<String> cliCommand(1);
  Vector<String> appParms(1);
  cliCommand = cli_command_;
  appParms = app_parms_;

  crow = mshistory.nrow();

  mshistory.addRow();
  
  mshistoryCol.time().put(crow, time_);
  mshistoryCol.observationId().put(crow, observation_id_-1);
  mshistoryCol.message().put(crow, message_);
  mshistoryCol.priority().put(crow, priority_);
  mshistoryCol.origin().put(crow, origin_);
  mshistoryCol.objectId().put(crow, object_id_);
  mshistoryCol.application().put(crow, String(application_));
  mshistoryCol.cliCommand().put(crow, cliCommand);
  mshistoryCol.appParams().put(crow, appParms);
  
  mshistory.flush(True);
  // cout << "\n";
}
	     
// Adds a single observation record in the table OBSERVATION
void ASDM2MSFiller::addObservation(const string&		telescopeName_,
				   double			startTime_,
				   double			endTime_,
				   const string&		observer_,
				   const vector<string>&	log_,
				   const string&		schedule_type_,
				   const vector<string>&	schedule_,
				   const string&		project_,
				   double			release_date_) {

  uInt crow;
  MSObservation msobs = itsMS -> observation();
  MSObservationColumns msobsCol(msobs);
  
  // cout << "\n addObservation: entering";
  // Build the log vector.
  Vector<String> log(1);
  
  if (log_.size() > 0) {
    log.resize(log_.size());
    for (vector<string>::size_type i = 0; i < log_.size(); i++) log(i) = log_[i];    
  }
  else
    log(0) =  "" ;


  // Build the schedule vector
  Vector<String> schedule(1);

  if (schedule_.size() > 0) {
    schedule.resize(schedule_.size());
    for (vector<string>::size_type  i = 0; i < schedule_.size(); i++) schedule(i) = schedule_[i];
  }
  else
    schedule(0) = "";

  Vector<Double> timeRange(2);
  
  timeRange(0) = startTime_;
  timeRange(1) = endTime_;

  // Fill the columns
  crow = msobs.nrow();
  /*
    itsObservationTimeMgr[0].setIndex(crow);
  */
  msobs.addRow();

  msobsCol.telescopeName().put(crow, String(telescopeName_));
  msobsCol.timeRange().put(crow, timeRange);
  msobsCol.observer().put(crow, String(observer_));
  msobsCol.log().put(crow, log);
  msobsCol.scheduleType().put(crow, schedule_type_);
  msobsCol.schedule().put(crow, schedule);
  msobsCol.project().put(crow, String(project_));
  msobsCol.releaseDate().put(crow, release_date_);
  msobsCol.flagRow().put(crow, False);
  msobs.flush();

  // cout << "\n addObservation: exiting";
  // cout << "\n";
}

void ASDM2MSFiller::addPointingSlice(unsigned int                 n_row_,
				     vector<int>&                 antenna_id_,
				     vector<double>&               time_,
				     vector<double>&              interval_,
				     vector<double>&              direction_,
				     vector<double>&              target_,
				     vector<double>&              pointing_offset_,
				     vector<double>&              encoder_,
				     vector<bool>&                tracking_,
				     bool                         overTheTopExists4All_,
				     vector<bool>&                v_overTheTop_,
				     vector<s_overTheTop>&        v_s_overTheTop_) {
  Vector<Int>    antenna_id(IPosition(1, n_row_), &antenna_id_[0], SHARE);
  Vector<Double> time(IPosition(1, n_row_), &time_[0], SHARE);
  Vector<Double> interval(IPosition(1, n_row_), &interval_[0], SHARE);

  Vector<String> name(IPosition(1, n_row_), "");

  Vector<int>    num_poly(IPosition(1, n_row_), 0);
  Cube<Double>   direction(IPosition(3, 2, 1, n_row_), &direction_[0], SHARE);
  Cube<Double>   target(IPosition(3, 2, 1, n_row_), &target_[0], SHARE);
  Cube<Double>   pointing_offset(IPosition(3, 2, 1, n_row_), &pointing_offset_[0], SHARE);
  Matrix<Double> encoder(IPosition(2, 2, n_row_), &encoder_[0], SHARE);
  Vector<Bool>   tracking(IPosition(1, n_row_));

  MSPointing mspointing = itsMS -> pointing();
  MSPointingColumns mspointingCol(mspointing);
  unsigned int crow = mspointing.nrow();

  // Let's fill tracking
  tracking = Vector<bool>(tracking_);
  // Define a slicer to write blocks of values in each column of the pointing table.
  Slicer slicer(IPosition(1, crow),
		IPosition(1, crow + n_row_ - 1),
		Slicer::endIsLast);

  // Let's create n_row_ empty rows into the pointing table.
  mspointing.addRow(n_row_);

  // Fill the columns
  mspointingCol.antennaId().putColumnRange(slicer, antenna_id);
  mspointingCol.time().putColumnRange(slicer, time);
  mspointingCol.interval().putColumnRange(slicer, interval);
  mspointingCol.name().putColumnRange(slicer, name);
  mspointingCol.numPoly().putColumnRange(slicer, num_poly);
  mspointingCol.direction().putColumnRange(slicer, direction);
  mspointingCol.target().putColumnRange(slicer, target);
  mspointingCol.pointingOffset().putColumnRange(slicer, pointing_offset);
  mspointingCol.encoder().putColumnRange(slicer, encoder);
  mspointingCol.tracking().putColumnRange(slicer, tracking);

  // We can use the slicer only if overTheTop is present on all rows.
  // For the time being, if this is not the case (i.e. overTheTop present only on a subset possibly empty) then we ignore overTheTop.
  if (overTheTopExists4All_) {
    vector<bool>::iterator iiter = v_overTheTop_.begin();
    Vector<Bool> over_the_top(IPosition(1, n_row_));
    Vector<Bool>::iterator oiter = over_the_top.begin();
    for ( ; oiter != over_the_top.end(); ++iiter, ++oiter) 
      *oiter = *iiter;

    mspointingCol.overTheTop().putColumnRange(slicer, over_the_top);
  }
  // Otherwise we fill overTheTop range after range.
  else {
    for (unsigned int i = 0; i < v_overTheTop_.size(); i++) {
      s_overTheTop saux = v_s_overTheTop_.at(i);
      for (unsigned int j = saux.start; j < (saux.start + saux.len) ; j++)
	mspointingCol.overTheTop().put(j, saux.value);
    }
  }

  mspointing.flush();
}


		 
// Adds a record in the table Polarization
int ASDM2MSFiller::addPolarization(int num_corr_,
				   vector<int>& corr_type_,
				   vector<int>& corr_product_) {
  uInt crow;
  int  i;
  Vector<Int>  corrType(num_corr_);
  Matrix<Int>  corrProduct(2, num_corr_);

  MSPolarization mspolar = itsMS -> polarization();
  MSPolarizationColumns mspolarCol(mspolar);

  const char** p=getPolCombinations(num_corr_);


  crow = mspolar.nrow();
  itsDDMgr.setNumCorr(crow, num_corr_);
  mspolar.addRow();

  mspolarCol.numCorr().put(crow, num_corr_);
  for (i=0; i < num_corr_; i++) {
    corrType(i) = Stokes::type(p[i]);
    corrProduct(0, i) = Stokes::receptor1(Stokes::type(p[i]));
    corrProduct(1, i) = Stokes::receptor2(Stokes::type(p[i]));
  }
  mspolarCol.corrType().put(crow,corrType);
  mspolarCol.corrProduct().put(crow, corrProduct);

  mspolar.flush();
  // cout << "\n";
  return crow;
}


int ASDM2MSFiller::addUniquePolarization(int num_corr_,
					 const vector<int>& corr_type_,
					 const vector<int>& corr_product_) {
  uInt crow;
  int  i;
  Vector<Int>  corrType(IPosition(1, num_corr_), (int *)&corr_type_[0], SHARE);
  Matrix<Int>  corrProduct(2, num_corr_);
  MSPolarization mspolar = itsMS -> polarization();
  MSPolarizationColumns mspolarCol(mspolar);

  //const char** p=getPolCombinations(num_corr_);
  int iCorrProduct = 0;
  for (i=0; i < num_corr_; i++) {
    //corrProduct(0, i) = Stokes::receptor1(Stokes::type(p[i]));
    corrProduct(0, i) = corr_product_[iCorrProduct++];
    //corrProduct(1, i) = Stokes::receptor2(Stokes::type(p[i]));
    corrProduct(1, i) = corr_product_[iCorrProduct++];;
  }

  crow = mspolar.nrow();

  /*
   * Look for an existing polarization
   */
  for (uInt i = 0; i < crow; i++) {
    Vector<Int> _corrType = mspolarCol.corrType()(i);
    Matrix<Int> _corrProduct = mspolarCol.corrProduct()(i);
    if ( (mspolarCol.numCorr()(i) == num_corr_)  &&
	 allEQ(_corrType,corrType) &&
	 allEQ(_corrProduct,corrProduct)
	 ) return i;
  }

  /*
   * This polarization configuration has not been found
   * let's create a new row.
   */
  mspolar.addRow();
  mspolarCol.numCorr().put(crow, num_corr_);
  mspolarCol.corrType().put(crow,corrType);
  mspolarCol.corrProduct().put(crow, corrProduct);

  mspolar.flush(True);
  return crow;
}


// Adds a record in the table PROCESSOR
void ASDM2MSFiller::addProcessor(string& type_,
				 string& sub_type_,
				 int  type_id_,
				 int  mode_id_) {
  uInt crow;
  MSProcessor msproc = itsMS -> processor();
  MSProcessorColumns msprocCol(msproc);

  crow = msproc.nrow();
  msproc.addRow();

  msprocCol.type().put(crow, String(type_));
  msprocCol.subType().put(crow, String(sub_type_));
  msprocCol.typeId().put(crow, type_id_);
  msprocCol.modeId().put(crow, mode_id_);
  
  msprocCol.flagRow().put(crow, False);

  msproc.flush();
  // cout << "\n";
}


// Adds a single state record in the table STATE in such a way that there is no repeated row.
// Returns the index of row added or found with these values. 
int ASDM2MSFiller::addUniqueState(bool sig_,
				  bool ref_,
				  double cal_,
				  double load_,
				  unsigned int sub_scan_,
				  string& obs_mode_,
				  bool flag_row_) {
  MSStateColumns msstateCol(itsMS -> state());
  uInt crow = itsMS->state().nrow();
  
  uInt i = 0;
  
  for (i = 0; i < crow; i++) {
    if ((msstateCol.sig()(i) == sig_) &&
	(msstateCol.ref()(i) == ref_) &&
	(msstateCol.cal()(i) == cal_) &&
	(msstateCol.load()(i) == load_) && 
	(msstateCol.subScan()(i) == (int)sub_scan_) &&
	(msstateCol.obsMode()(i).compare(obs_mode_) == 0) &&
	(msstateCol.flagRow()(i) == flag_row_)) {
      return i;
    }
  }
  
  itsMS->state().addRow();
  msstateCol.sig().put(crow, sig_);
  msstateCol.ref().put(crow, ref_);
  msstateCol.cal().put(crow, cal_);
  msstateCol.load().put(crow, load_);
  msstateCol.subScan().put(crow, sub_scan_);
  msstateCol.obsMode().put(crow, String(obs_mode_));
  msstateCol.flagRow().put(crow, flag_row_);
  
  return crow;
}

// Add a record in the table SOURCE
void ASDM2MSFiller::addSource(int             source_id_,
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
			      vector<double>& sysvel_){
  MSSource mssource = itsMS -> source();
  MSSourceColumns mssourceCol(mssource);

  String s(direction_code_);
  if(s==""){
    cout << "SOURCE table: directionCode doesn't exist or is empty. Will assume J2000." << endl;
    s="J2000";
  }
  s.trim(); s.upcase();
  map<string, MDirection::Types>::const_iterator iter = string2MDirection.find(s);

  MDirection directionMD;

  if (iter == string2MDirection.end()) {
    //cout << "Could not determine directionCode. Assuming J2000 ..." << endl;
    directionMD    = MDirection(Quantity(direction_[0], "rad"), Quantity(direction_[1], "rad"), MDirection::J2000);
  }
  else {
    //cout << "directionCode is " << s << " (" << (int)iter->second << ")" << endl;
    directionMD    = MDirection(Quantity(direction_[0], "rad"), Quantity(direction_[1], "rad"), iter->second);
  }

  MDirection::Types theType;
  MDirection::getType(theType, directionMD.getRefString());
  if(mssource.nrow()==0){ // setDescRefCode works only with empty table 
    //cout << "Setting Source table direction reference to " << (int)theType << endl;
    if( ((int)theType) >= MDirection::N_Types ){
      cout << "Solar system object reference frame handling in Source table not yet implemented. Falling back to J2000." << endl;
      mssourceCol.directionMeas().setDescRefCode((int)MDirection::J2000, True); 
    }
    else{
      mssourceCol.directionMeas().setDescRefCode((int)theType, True); 
    }
  }
  else{
    casa::MDirection mD;
    mssourceCol.directionMeas().get(0, mD);
    casa::MDirection::Types theFirstType;
    casa::MDirection::getType(theFirstType, mD.getRefString());
    if(theType != theFirstType){
      cout << "Inconsistent directionCodes in Source table: " << theType << " (" << MDirection::showType(theType) << ") and "
	   << theFirstType << " (" << MDirection::showType(theFirstType) << ")"<< endl;
      cout << "Will convert all directions to type " << theFirstType << " (" << MDirection::showType(theFirstType) << ")" << endl;
      if( ((int)theType) >= MDirection::N_Types ){
	cout << "(Proper conversion not yet implemented for solar system objects.)" << endl;
	directionMD = MDirection(Quantity(direction_[0], "rad"), Quantity(direction_[1], "rad"), theFirstType);
      }
    }
  }
    
  // Add a new row.
  int crow = mssource.nrow();
  mssource.addRow();

  Vector<Double> properMotion(IPosition(1, 2), &proper_motion_[0], SHARE);

  // Fill the new row
  mssourceCol.sourceId().put(crow,source_id_);
  mssourceCol.time().put(crow,time_);
  mssourceCol.interval().put(crow,interval_);
  mssourceCol.spectralWindowId().put(crow,spectral_window_id_);
  mssourceCol.numLines().put(crow,num_lines_);
  mssourceCol.name().put(crow,String(name_));
  mssourceCol.calibrationGroup().put(crow,calibration_group_);
  mssourceCol.code().put(crow,String(code_));
  mssourceCol.directionMeas().put(crow,directionMD);
  if (position_.size() > 0) mssourceCol.position().put(crow,Vector<Double>(IPosition(1, 3), &position_[0], SHARE));
  mssourceCol.properMotion().put(crow,properMotion);
  if ( transition_.size() > 0 ) {
    Vector<String> transition(IPosition(1, transition_.size()));
    for (vector<string>::size_type i = 0; i < transition_.size(); i++)
      transition(i) = transition_[i];
    mssourceCol.transition().put(crow, transition);
  }
  if ( rest_frequency_.size() > 0 ) 
    mssourceCol.restFrequency().put(crow,Vector<Double>(IPosition(1, rest_frequency_.size()), &rest_frequency_[0], SHARE));
  if ( sysvel_.size() > 0)
    mssourceCol.sysvel().put(crow, Vector<Double>(IPosition(1, sysvel_.size()), &sysvel_[0], SHARE));
}

// Add a  record  in the table SPECTRAL_WINDOW
int ASDM2MSFiller::addSpectralWindow(int			num_chan_,
				     const string&		name_,
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
				     const vector<int>&		assoc_sp_id_,
				     const vector<string>&      assoc_nature_) {
 
  MSSpectralWindow msspwin = itsMS -> spectralWindow();
  MSSpWindowColumns msspwinCol(msspwin);

  Vector<Double> chanFreq(IPosition(1, num_chan_), const_cast<double *>(&chan_freq_[0]), SHARE);
  Vector<Double> chanWidth(IPosition(1, num_chan_), const_cast<double *>(&chan_width_[0]), SHARE);
  Vector<Double> effectiveBW(IPosition(1, num_chan_), const_cast<double *>(&effective_bw_[0]), SHARE);
  Vector<Double> resolution(IPosition(1, num_chan_), const_cast<double *>(&resolution_[0]), SHARE);
  uInt crow;
  
  crow = msspwin.nrow();
  itsDDMgr.setNumChan(crow, num_chan_);
  msspwin.addRow();
  
  msspwinCol.numChan().put(crow, num_chan_);
  msspwinCol.name().put(crow, String(name_));
  msspwinCol.refFrequency().put(crow, ref_frequency_);

  msspwinCol.chanFreq().put(crow, chanFreq);
  msspwinCol.chanWidth().put(crow, chanWidth);
  msspwinCol.effectiveBW().put(crow, effectiveBW);
  msspwinCol.resolution().put(crow, resolution);
  msspwinCol.measFreqRef().put(crow, meas_freq_ref_);
  msspwinCol.totalBandwidth().put(crow, total_bandwidth_);
  msspwinCol.netSideband().put(crow, net_sideband_);
  if (bbc_no_ >= 0) msspwinCol.bbcNo().put(crow, bbc_no_);
  msspwinCol.ifConvChain().put(crow, if_conv_chain_);
  msspwinCol.freqGroup().put(crow, freq_group_);
  msspwinCol.freqGroupName().put(crow, String(freq_group_name_));
  /*
   * Put assoc informations if any.
   */
  if ( num_assoc_>0  && assoc_sp_id_.size() > 0 ) {
    msspwinCol.assocSpwId().put(crow, Vector<Int>(IPosition(1, num_assoc_), const_cast<int *>(&assoc_sp_id_[0]), SHARE));
  }

  if ( num_assoc_>0 && assoc_nature_.size() > 0  ) {
    Vector<String> assocNature(assoc_nature_.size());
    for (vector<string>::size_type iAssocNature = 0;
	 iAssocNature < assoc_nature_.size();
	 iAssocNature++) 
      assocNature(iAssocNature) = assoc_nature_[iAssocNature];
    msspwinCol.assocNature().put(crow, assocNature);
  }

  msspwinCol.flagRow().put(crow, False);

  msspwin.flush();
  // cout << "\n";
  return crow;
}


// Adds a single state record in the table STATE.
void ASDM2MSFiller::addState(bool    sig_,
			     bool    ref_,
			     double  cal_,
			     double  load_,
			     int     sub_scan_,
			     string& obs_mode_) {
  uInt crow;
  MSState msstate = itsMS -> state();
  MSStateColumns msstateCol(msstate);

  crow = msstate.nrow();
  msstate.addRow();

  msstateCol.sig().put(crow, sig_);
  msstateCol.ref().put(crow, ref_);
  msstateCol.cal().put(crow, cal_);
  msstateCol.load().put(crow, load_);
  msstateCol.subScan().put(crow, sub_scan_);
  msstateCol.obsMode().put(crow, obs_mode_);
  msstateCol.flagRow().put(crow, False);
  msstate.flush();

  // cout << "\n";
}

// Adds a record in the table SYSCAL.
void ASDM2MSFiller:: addSysCal(int    antenna_id,
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
			       pair<bool, bool>&           tant_tsys_flag_pair) {
  MSSysCal mssyscal = itsMS -> sysCal();
  MSSysCalColumns mssyscalCol(mssyscal);
  
  int crow = mssyscal.nrow();
  mssyscal.addRow();

  mssyscalCol.antennaId().put(crow, antenna_id);
  mssyscalCol.feedId().put(crow, feed_id);
  mssyscalCol.spectralWindowId().put(crow, spectral_window_id);
  mssyscalCol.time().put(crow, time_);
  mssyscalCol.interval().put(crow, interval_);

  if (tcal_spectrum_pair.first) {
    Matrix<Float> tcal_spectrum(IPosition(2, numReceptor_, numChan_), &tcal_spectrum_pair.second[0], SHARE);
    mssyscalCol.tcalSpectrum().put(crow, tcal_spectrum);
    if (tcal_flag_pair.first)
      mssyscalCol.tcalFlag().put(crow, tcal_flag_pair.second);
  }

  if (trx_spectrum_pair.first) {
    Matrix<Float> trx_spectrum(IPosition(2, numReceptor_, numChan_), &trx_spectrum_pair.second[0], SHARE);
    mssyscalCol.trxSpectrum().put(crow, trx_spectrum);
    if (trx_flag_pair.first)
      mssyscalCol.trxFlag().put(crow, trx_flag_pair.second);
  }

  if (tsky_spectrum_pair.first) {
    Matrix<Float> tsky_spectrum(IPosition(2, numReceptor_, numChan_), &tsky_spectrum_pair.second[0], SHARE);
    mssyscalCol.tskySpectrum().put(crow, tsky_spectrum);
    if (tsky_flag_pair.first)
      mssyscalCol.tskyFlag().put(crow, tsky_flag_pair.second);
  }

  if (tsys_spectrum_pair.first) {
    Matrix<Float> tsys_spectrum(IPosition(2, numReceptor_, numChan_), &tsys_spectrum_pair.second[0], SHARE);
    mssyscalCol.tsysSpectrum().put(crow, tsys_spectrum);
    if (tsys_flag_pair.first)
      mssyscalCol.tsysFlag().put(crow, tsys_flag_pair.second);
  }

  if (tant_spectrum_pair.first) {
    Matrix<Float> tant_spectrum(IPosition(2, numReceptor_, numChan_), &tant_spectrum_pair.second[0], SHARE);
    mssyscalCol.tantSpectrum().put(crow, tant_spectrum);
    if (tant_flag_pair.first)
      mssyscalCol.tantFlag().put(crow, tant_flag_pair.second);
  }

  if (tant_tsys_spectrum_pair.first) {
    Matrix<Float> tant_tsys_spectrum(IPosition(2, numReceptor_, numChan_), &tant_tsys_spectrum_pair.second[0], SHARE);
    mssyscalCol.tantTsysSpectrum().put(crow, tant_tsys_spectrum);
    if (tant_tsys_flag_pair.first)
      mssyscalCol.tantTsysFlag().put(crow, tant_tsys_flag_pair.second);
  }

  mssyscal.flush();
}

// Adds a  record weather in the table WEATHER.
  void ASDM2MSFiller::addWeather(int				antenna_id_,
				 double				time_,
				 double				interval_,
				 const pair<bool, float>&	pressure_opt_,
				 const pair<bool, float>&	relHumidity_opt_,
				 const pair<bool, float>&	temperature_opt_,
				 const pair<bool, float>&	windDirection_opt_,
				 const pair<bool, float>&	windSpeed_opt_,
				 const pair<bool, float>&	dewPoint_opt_,
				 int				wx_station_id_,
				 vector<double>&		wx_station_position_) {
  
  MSWeather msweather = itsMS -> weather();
  MSWeatherColumns msweatherCol(msweather);

  int crow;
  
  crow = msweather.nrow();
  msweather.addRow();
  
  msweatherCol.antennaId().put(crow, antenna_id_);
  msweatherCol.interval().put(crow, interval_);
  msweatherCol.time().put(crow, time_);

  if (pressure_opt_.first) msweatherCol.pressure().put(crow, pressure_opt_.second);
  if (relHumidity_opt_.first) msweatherCol.relHumidity().put(crow, relHumidity_opt_.second);
  if (temperature_opt_.first) msweatherCol.temperature().put(crow, temperature_opt_.second);
  if (windDirection_opt_.first) msweatherCol.windDirection().put(crow, windDirection_opt_.second);
  if (windSpeed_opt_.first) msweatherCol.windSpeed().put(crow, windSpeed_opt_.second);
  if (dewPoint_opt_.first) msweatherCol.dewPoint().put(crow, dewPoint_opt_.second);

  ScalarColumn<int> nsWXStationId(msweather, "NS_WX_STATION_ID");
  nsWXStationId.put(crow, wx_station_id_);
  ArrayColumn<double> nsWXStationPosition(msweather, "NS_WX_STATION_POSITION");
  nsWXStationPosition.put(crow, Vector<double>(IPosition(1, 3), &wx_station_position_[0], SHARE));

  msweather.flush();
}


void ASDM2MSFiller::addSysPower(int		antennaId,
				int		feedId,
				int		spectralWindowId,
				double		time,
				double		interval,
				unsigned int    numReceptor,
				vector<float>&	switchedPowerDifference,
				vector<float>&	switchedPowerSum,
				vector<float>&  requantizerGain
				) {
  Table						mssyspower = itsMS->rwKeywordSet().asTable("SYSPOWER");
  int						rowIndex   = mssyspower.nrow();
  mssyspower.addRow(1);

  ScalarColumn<Int>	antennaIdCol(mssyspower, "ANTENNA_ID");
  antennaIdCol.put(rowIndex, antennaId);

  ScalarColumn<Int>	feedIdCol(mssyspower, "FEED_ID");
  feedIdCol.put(rowIndex, feedId);

  ScalarColumn<Int>	spectralWindowIdCol(mssyspower, "SPECTRAL_WINDOW_ID");
  spectralWindowIdCol.put(rowIndex, spectralWindowId);

  ScalarColumn<Double>	timeCol(mssyspower, "TIME");
  timeCol.put(rowIndex, time);

  ScalarColumn<Double>	intervalCol(mssyspower, "INTERVAL");
  intervalCol.put(rowIndex, interval);

  // numReceptor != 0 then consider the optional attributes.
  //
  if (numReceptor) {
    // switchedPowerDifference size != 0 then optional attribute is present.
    // 
    if (switchedPowerDifference.size()) {
      ArrayColumn<Float>	switchedPowerDifferenceCol(mssyspower, "SWITCHED_DIFF");
      Vector<Float> switchedPowerDifference_(IPosition(1, switchedPowerDifference.size()), const_cast<float *>(&switchedPowerDifference[0]));
      switchedPowerDifferenceCol.put(rowIndex, switchedPowerDifference_);
    }
    
    // switchedPowerSum size != 0 then optional attribute is present.
    // 
    if (switchedPowerSum.size()) {
      ArrayColumn<Float>	switchedPowerSumCol(mssyspower, "SWITCHED_SUM");
      Vector<Float> switchedPowerSum_(IPosition(1, switchedPowerSum.size()), const_cast<float *>(&switchedPowerSum[0]));
      switchedPowerSumCol.put(rowIndex, switchedPowerSum_);
    }
    
    // requantizerGain size != 0 then optional attribute is present.
    // 
    if (requantizerGain.size()) {
      ArrayColumn<Float>	requantizerGainCol(mssyspower, "REQUANTIZER_GAIN");
      Vector<Float> requantizerGain_(IPosition(1, requantizerGain.size()), const_cast<float *>(&requantizerGain[0]));
      requantizerGainCol.put(rowIndex, requantizerGain_);
    }
  }
  
  mssyspower.flush();
}

void ASDM2MSFiller::addSysPowerSlice(unsigned int	nRow,
				     vector<int>&       antennaId,
				     vector<int>&	spectralWindowId,
				     vector<int>&	feedId,
				     vector<double>&	time,
				     vector<double>&	interval,
				     unsigned int       numReceptor,
				     vector<float>&	switchedPowerDifference,
				     vector<float>&	switchedPowerSum,
				     vector<float>&     requantizerGain) {

  Table			mssyspower = itsMS->rwKeywordSet().asTable("SYSPOWER");
    
  Vector<Int>		antennaIdMS(IPosition(1, nRow), &antennaId[0], SHARE);
  Vector<Int>		spectralWindowIdMS(IPosition(1, nRow), &spectralWindowId[0], SHARE);
  Vector<Int>		feedIdMS(IPosition(1, nRow), &feedId[0], SHARE);
  Vector<Double>	timeMS(IPosition(1, nRow), &time[0], SHARE);
  Vector<Double>	intervalMS(IPosition(1, nRow), &interval[0], SHARE);

  unsigned int crow = mssyspower.nrow();

  // Define a slicer to write blocks of nRow rows in the columns the SYSPOWER table.
  Slicer slicer(IPosition(1, crow),
		IPosition(1, crow + nRow - 1),
		Slicer::endIsLast);

  mssyspower.addRow(nRow);

  // Fill the obvious columns
  ScalarColumn<Int>	antennaIdCol(mssyspower, "ANTENNA_ID");
  antennaIdCol.putColumnRange(slicer, antennaIdMS);
  
  ScalarColumn<Int>	feedIdCol(mssyspower, "FEED_ID");
  feedIdCol.putColumnRange(slicer, feedIdMS);
  
  ScalarColumn<Int>	spectralWindowIdCol(mssyspower, "SPECTRAL_WINDOW_ID");
  spectralWindowIdCol.putColumnRange(slicer, spectralWindowIdMS);
  
  ScalarColumn<Double>	timeCol(mssyspower, "TIME");
  timeCol.putColumnRange(slicer, timeMS);

  ScalarColumn<Double>	intervalCol(mssyspower, "INTERVAL");
  intervalCol.putColumnRange(slicer, intervalMS);

  ArrayColumn<Float> switchedPowerDifferenceCol(mssyspower, "SWITCHED_DIFF");
  if (switchedPowerDifference.size() != 0) {
    Matrix<Float> switchedPowerDifferenceMS(IPosition(2, numReceptor, nRow), &switchedPowerDifference[0], SHARE);
    switchedPowerDifferenceCol.putColumnRange(slicer, switchedPowerDifferenceMS);
  }

  ArrayColumn<Float> switchedPowerSumCol(mssyspower, "SWITCHED_SUM");
  if (switchedPowerSum.size() != 0) {
    Matrix<Float> switchedPowerSumMS(IPosition(2, numReceptor, nRow), &switchedPowerSum[0], SHARE);
    switchedPowerSumCol.putColumnRange(slicer, switchedPowerSumMS);
  }

  ArrayColumn<Float> requantizerGainCol(mssyspower, "REQUANTIZER_GAIN");
  if (requantizerGain.size() != 0) {
    Matrix<Float> requantizerGainMS(IPosition(2, numReceptor, nRow), &requantizerGain[0], SHARE);
    requantizerGainCol.putColumnRange(slicer, requantizerGainMS);
  }

  mssyspower.flush();
}
				     

void ASDM2MSFiller::addCalDevice(int				antennaId,
				 int				feedId,
				 int				spectralWindowId,
				 double				time,
				 double				interval,
				 unsigned int			numCalLoad,
				 vector<string>			calloadNames,
				 unsigned int			numReceptor,
				 vector<vector<float> >&	calEff,
				 vector<vector<float> >&	noiseCal,
				 vector<double >&                temperatureLoad) {
  Table								mscaldevice = itsMS->rwKeywordSet().asTable("CALDEVICE");
  int								rowIndex    = mscaldevice.nrow();
  mscaldevice.addRow(1);

  ScalarColumn<Int>	antennaIdCol(mscaldevice, "ANTENNA_ID");
  antennaIdCol.put(rowIndex, antennaId);

  ScalarColumn<Int>	feedIdCol(mscaldevice, "FEED_ID");
  feedIdCol.put(rowIndex, feedId);

  ScalarColumn<Int>	spectralWindowIdCol(mscaldevice, "SPECTRAL_WINDOW_ID");
  spectralWindowIdCol.put(rowIndex, spectralWindowId);

  ScalarColumn<Double>	timeCol(mscaldevice, "TIME");
  timeCol.put(rowIndex, time);

  ScalarColumn<Double>	intervalCol(mscaldevice, "INTERVAL");
  intervalCol.put(rowIndex, interval);  

  ScalarColumn<Int> numCalLoadCol(mscaldevice, "NUM_CAL_LOAD");
  numCalLoadCol.put(rowIndex, numCalLoad);

  ArrayColumn<String> calloadNamesCol(mscaldevice, "CAL_LOAD_NAMES");
  Vector<String> calloadNames_(IPosition(1, calloadNames.size()));
  for (unsigned int i = 0; i < calloadNames.size(); i++)
    calloadNames_(i) = calloadNames[i];
  calloadNamesCol.put(rowIndex, calloadNames_);
  
  //
  // numReceptor == 0 will be interpreted as numReceptor 'absent'.
  if (numReceptor) {
    ScalarColumn<Int> numReceptorCol(mscaldevice, "NUM_RECEPTOR");
    numReceptorCol.put(rowIndex, numReceptor);

    //
    // calEff size == 0 will be interpreted as calEff absent. 
    if (calEff.size()) {
      ArrayColumn<Float> calEffCol(mscaldevice, "CAL_EFF");
      Matrix<Float> calEff_(IPosition(2, numCalLoad, numReceptor));
      for (unsigned int iReceptor = 0; iReceptor < numReceptor; iReceptor++)
	for (unsigned int iCalLoad = 0; iCalLoad < numCalLoad; iCalLoad++)
	  calEff_(iCalLoad, iReceptor) = calEff.at(iReceptor).at(iCalLoad);
      calEffCol.put(rowIndex, calEff_);
    }
    
    //
    // noiseCal size == 0 will be interpreted as noiseCal absent.
    if (noiseCal.size()) {
      ArrayColumn<Float> noiseCalCol(mscaldevice, "NOISE_CAL");
      Matrix<Float> noiseCal_(IPosition(2, numCalLoad, numReceptor));
      for (unsigned int iReceptor = 0; iReceptor < numReceptor; iReceptor++)
	for (unsigned int iCalLoad = 0; iCalLoad < numCalLoad; iCalLoad++)
	  noiseCal_(iCalLoad, iReceptor) = (float) noiseCal.at(iReceptor).at(iCalLoad);
      noiseCalCol.put(rowIndex, noiseCal_);
    }
  }
   
  //
  // temperatureLoad == 0 will be interpreted as temperatureLoad absent. 
  if (temperatureLoad.size()) {
    ArrayColumn<Double> temperatureLoadCol(mscaldevice, "TEMPERATURE_LOAD");
    temperatureLoadCol.put(rowIndex, Vector<Double>(IPosition(1, numCalLoad), &temperatureLoad[0], SHARE));
  }    

  mscaldevice.flush();
}

void ASDM2MSFiller::end(double time_) {
  itsMS->flush();
  delete itsMS;
}

