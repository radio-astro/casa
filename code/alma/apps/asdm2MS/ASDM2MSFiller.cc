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

// Methods of ASDM2MSFiller classe.
// The constructor
ASDM2MSFiller::ASDM2MSFiller(const string& name_,
			     double        creation_time_,
			     bool          withRadioMeters_,
			     bool          complexData,
			     bool          withCompression,
			     bool          withCorrectedData):
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
  status = createMS(name_, complexData, withCompression, withCorrectedData);
  //cout << "Back from call createMS" << endl;

}

// The destructor
ASDM2MSFiller::~ASDM2MSFiller() {
  ;
}

int ASDM2MSFiller::createMS(const string& msName, bool complexData, bool withCompression, bool withCorrectedData) {

  String aName(msName);

  // FLAG CATEGORY stuff.
  Vector<String>  cat(itsNCat);
  cat(0) = "FLAG_CMD";
  cat(1) = "ORIGINAL";
  cat(2) = "USER";


  //cout << "Entering createMS : measurement set = "<< aName <<"\n";

  // Get the MS main default table description
  TableDesc td = MS::requiredTableDesc();
  //cout << "createMS TableDesc\n";

  if (complexData) {
    // Add the DATA column
    MS::addColumnToDesc(td, MS::DATA,2);
      
    // Do we want a CORRECTED_DATA column ?
    if (withCorrectedData) 
      MS::addColumnToDesc(td, MS::CORRECTED_DATA,2);
      
    // Do we want a compressed DATA column ?
    if (withCompression) MS::addColumnCompression(td, MS::DATA, true);
  }
  else {
    // Add the FLOAT_DATA column
    MS::addColumnToDesc(td, MS::FLOAT_DATA,2);

    // Do we want a compressed DATA column ?
    if (withCompression) MS::addColumnCompression(td, MS::FLOAT_DATA, true);
  }

  /*  
  // Add MODEL_DATA, CORRECTED_DATA and IMAGING_WEIGHT columns
  MS::addColumnToDesc(td, MS::MODEL_DATA,2);
  MS::addColumnToDesc(td, MS::CORRECTED_DATA,2);
  MS::addColumnToDesc(td, MS::IMAGING_WEIGHT,1);
  */

  // Setup hypercolumns for the data/flag/flag_category/sigma & weight columns, model_data, corrected_data and imaging_weight.
  const Vector<String> coordCols(0);
  const Vector<String> idCols(0);
    
  String colData;
  String colFloatData;

  if (complexData) {
    colData = MS::columnName(MS::DATA);
    td.defineHypercolumn("TiledData", 3, stringToVector(colData),
			 coordCols, idCols);
  }
  else {
    colFloatData = MS::columnName(MS::FLOAT_DATA);
    td.defineHypercolumn("TiledFloatData", 3, stringToVector(colFloatData),
			 coordCols, idCols);
  }

  //cout << "defined float data Hypercolumn" << endl;

  /*
    String colModelData = MS::columnName(MS::MODEL_DATA);
    td.defineHypercolumn("TiledModelData", 3, stringToVector(colModelData),
    coordCols, idCols);
    //cout << "defined model data Hypercolumn" << endl;

    String colCorrData = MS::columnName(MS::CORRECTED_DATA);
    td.defineHypercolumn("TiledCorrectedData", 3,
    stringToVector(colCorrData),
    coordCols, idCols);
    //cout << "defined corrected data Hypercolumn" << endl;

    String colImWgt = MS::columnName(MS::IMAGING_WEIGHT);
    td.defineHypercolumn("TiledImWgt", 2,
    stringToVector(colImWgt),
    coordCols, idCols);
    //cout << "defined imaging weight Hypercolumn" << endl;
    */
    

  td.defineHypercolumn("TiledFlag", 3,
		       stringToVector(MS::columnName(MS::FLAG)));
  //cout << "defined flag Hypercolumn" << endl;


  if ( withCompression ) MS::addColumnCompression(td, MS::WEIGHT, true);
  td.defineHypercolumn("TiledWeight", 2,
		       stringToVector(MS::columnName(MS::WEIGHT)));
  //cout << "defined Weight Hypercolumn" << endl;

  if ( withCompression ) MS::addColumnCompression(td, MS::SIGMA, true);
  td.defineHypercolumn("TiledSigma", 2,
		       stringToVector(MS::columnName(MS::SIGMA)));
  //cout << "defined Sigma Hypercolumn" << endl;


  td.defineHypercolumn("TiledUVW", 2,
		       stringToVector(MS::columnName(MS::UVW)));
  //cout << "defined UVW Hypercolumn" << endl;

    
  td.defineHypercolumn("TiledFlagCategory", 4,
		       stringToVector(MS::columnName(MS::FLAG_CATEGORY)));
  //cout << "defined Flag Category Hypercolumn" << endl;

  if (itsWithRadioMeters) {
    // ALMA_PHAS_COOR hypercolumn
    String colPhaseCorr = "ALMA_PHAS_CORR";
    td.addColumn(ArrayColumnDesc<Complex>(colPhaseCorr,
					  "Phase-corrected data",
					  2));
    td.defineHypercolumn("TiledPhaseCorr",
			 3,
			 stringToVector(colPhaseCorr),
			 coordCols,
			 idCols);

    // ALMA_NO_PHAS_COOR hypercolumn
    String colNoPhaseCorr = "ALMA_NO_PHAS_CORR";
    td.addColumn(ArrayColumnDesc<Complex>(colNoPhaseCorr,
					  "Phase-uncorrected data",
					  2));
    td.defineHypercolumn("TiledNoPhaseCorr",
			 3,
			 stringToVector(colNoPhaseCorr),
			 coordCols,
			 idCols);

    // ALMA_PHAS_CORR_FLAG_ROW
    String colPhaseCorrFlagRow = "ALMA_PHAS_CORR_FLAG_ROW";
    td.addColumn(ScalarColumnDesc<Bool>(colPhaseCorrFlagRow,
					"Phase-corrected data present?"));
  }
    

  SetupNewTable newTab(aName, td, Table::New);

  //cout << "createMS SetupNewTable\n";
    
  // Choose the Tile size per column to be ~ 4096K
  const Int nTileCorr = 1;
  const Int nTileChan = 1024;
  const Int tileSizeKBytes = 16;
  Int nTileRow;

  // Create an incremental storage manager
  IncrementalStMan      incrStMan;

  // By default all the columns are bound to the Inc Stman
  newTab.bindAll(incrStMan);    

  // Define standard storage managers for scalar columns

  // ANTENNA1
  StandardStMan antenna1StMan("Antenna1 Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::ANTENNA1), antenna1StMan);

  // ANTENNA2 
  StandardStMan antenna2StMan("Antenna2 Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::ANTENNA2), antenna2StMan);

  // FEED1
  StandardStMan feed1StMan("Feed1 Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::FEED1), feed1StMan);

  // FEED2 
  StandardStMan feed2StMan("Feed2 Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::FEED2), feed2StMan);


  // DATA_DESC_ID 
  StandardStMan dataDescriptionStMan("DataDescID Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::DATA_DESC_ID), dataDescriptionStMan);

  // PROCESSOR_ID 
  StandardStMan processorStMan("ProcessorID Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::PROCESSOR_ID), processorStMan);

  // FIELD_ID 
  StandardStMan fieldStMan("FieldID Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::FIELD_ID), fieldStMan);


  // INTERVAL 
  StandardStMan intervalStMan("Interval Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::INTERVAL), intervalStMan);

  // EXPOSURE 
  StandardStMan exposureStMan("Exposure Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::EXPOSURE), exposureStMan);

  // TIME_CENTROID 
  StandardStMan timeCentroidStMan("TimeCentroid Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::TIME_CENTROID), timeCentroidStMan);

  // SCAN_NUMBER 
  StandardStMan scanNumberStMan("ScanNumber Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::SCAN_NUMBER), scanNumberStMan);

  // ARRAY_ID 
  StandardStMan arrayIdStMan("ArrayId Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::ARRAY_ID), arrayIdStMan);
    
  // OBSERVATION_ID 
  StandardStMan observationIdStMan("ObservationId Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::OBSERVATION_ID), observationIdStMan);

  // STATE_ID 
  StandardStMan stateIdStMan("StateId Standard Storage Manager", 32768);
  newTab.bindColumn(MS::columnName(MS::STATE_ID), stateIdStMan);



    
  if (complexData) {
    // DATA hypercolumn
    nTileRow = (tileSizeKBytes * 1024 / (2 * 4 * nTileCorr * nTileChan));
    IPosition dataTileShape(3, nTileCorr, nTileChan, nTileRow);

    TiledShapeStMan dataStMan("TiledData", dataTileShape);
    newTab.bindColumn(colData, dataStMan);
      
    if (withCorrectedData) {
      // CORRECTED DATA hypercolumn
      nTileRow = (tileSizeKBytes * 1024 / (2 * 4 * nTileCorr * nTileChan));
      IPosition corrDataTileShape(3, nTileCorr, nTileChan, nTileRow);
	
      TiledShapeStMan corrDataStMan("TiledCorrectedData", corrDataTileShape);
      newTab.bindColumn(MS::columnName(MS::CORRECTED_DATA), corrDataStMan);
    }
  }
  else {
    // FLOAT_DATA hypercolumn
    nTileRow = (tileSizeKBytes * 1024 / (2 * 4 * nTileCorr * nTileChan));
    IPosition floatDataTileShape(3, nTileCorr, nTileChan, nTileRow);

    TiledShapeStMan floatDataStMan("TiledFloatData", floatDataTileShape);
    newTab.bindColumn(colFloatData, floatDataStMan);
  }
  /*
  // MODEL DATA hypercolumn
  nTileRow = (tileSizeKBytes * 1024 / (2 * 4 * nTileCorr * nTileChan));
  IPosition modelDataTileShape(3, nTileCorr, nTileChan, nTileRow);

  TiledShapeStMan modelDataStMan("TiledModelData", modelDataTileShape);
  newTab.bindColumn(MS::columnName(MS::MODEL_DATA), modelDataStMan);

  // CORRECTED DATA hypercolumn
  nTileRow = (tileSizeKBytes * 1024 / (2 * 4 * nTileCorr * nTileChan));
  IPosition corrDataTileShape(3, nTileCorr, nTileChan, nTileRow);

  TiledShapeStMan corrDataStMan("TiledCorrectedData", corrDataTileShape);
  newTab.bindColumn(MS::columnName(MS::CORRECTED_DATA), corrDataStMan);

  // IMAGING WEIGHT hypercolumn
  nTileRow = (tileSizeKBytes * 1024 / (4 * nTileChan));
  IPosition imWgtTileShape(2, nTileChan, nTileRow);

  TiledShapeStMan imWgtStMan("TiledImWgt", imWgtTileShape);
  newTab.bindColumn(MS::columnName(MS::IMAGING_WEIGHT), imWgtStMan);
  */
    
  // WEIGHT and SIGMA hypercolumn
#if 0
  nTileRow = (tileSizeKBytes * 1024 / (4 * nTileCorr));
  IPosition weightTileShape(2, nTileCorr, nTileRow);
  TiledShapeStMan weightStMan("TiledWeight", weightTileShape);
  newTab.bindColumn(MS::columnName(MS::WEIGHT), weightStMan);
  TiledShapeStMan sigmaStMan("TiledSigma", weightTileShape);
  newTab.bindColumn(MS::columnName(MS::SIGMA), sigmaStMan);
#else
  StandardStMan weightStMan("StandardStManWeight");
  newTab.bindColumn(MS::columnName(MS::WEIGHT), weightStMan);
  StandardStMan sigmaStMan("StandardStManSigma");
  newTab.bindColumn(MS::columnName(MS::SIGMA), sigmaStMan);

#endif
  // UVW hyperColumn
  nTileRow = (tileSizeKBytes * 1024 / (8 * 3));
  IPosition uvwTileShape(2, 3, nTileRow);
  TiledColumnStMan uvwStMan("TiledUVW", uvwTileShape);
  newTab.bindColumn(MS::columnName(MS::UVW), uvwStMan);

  // FLAG hyperColumn
  nTileRow = (tileSizeKBytes * 1024 / (nTileCorr * nTileChan));
  IPosition flagTileShape(3, nTileCorr, nTileChan, nTileRow);
  TiledShapeStMan flagStMan("TiledFlag", flagTileShape);
  newTab.bindColumn(MS::columnName(MS::FLAG), flagStMan);

  // FLAG CATEGORY hypercolumn
  nTileRow = (tileSizeKBytes * 1024 / (nTileCorr * nTileChan * itsNCat));
  IPosition flagCategoryTileShape(4, nTileCorr, nTileChan,
				  itsNCat, nTileRow);
  TiledShapeStMan flagCategoryStMan("TiledFlagCategory",
				    flagCategoryTileShape);
  newTab.bindColumn(MS::columnName(MS::FLAG_CATEGORY),
		    flagCategoryStMan);

  //cout << "createMS bindAll\n";

  //cout << "createMS DATA, FLAG, UVW, WEIGHT and SIGMA bound to their storage managers\n";

  

  // And finally create the Measurement Set and get access
  // to its columns
  //Table::TableOption openOption = Table::New;
  //itsMS = new MeasurementSet(newTab, openOption);
  //cout << "About to call the Measurement set constructor" << endl;
  itsMS = new casa::MeasurementSet(newTab);
  //cout << "createMS MeasurementSet, adress=" << (int) itsMS << endl;
  if (! itsMS) {
    return False;
  }
  //cout << "Measurement Set just created, main table nrow=" << itsMS->nrow()<< endl;
  

  itsMSCol = new casa::MSMainColumns(*itsMS);


  // Create all subtables and their possible extra columns.
  // Antenna
  {
    SetupNewTable tabSetup(itsMS->antennaTableName(),
			   MSAntenna::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::ANTENNA),
				      Table(tabSetup));
  }
  // Data description
  {
    SetupNewTable tabSetup(itsMS->dataDescriptionTableName(),
			   MSDataDescription::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::DATA_DESCRIPTION),
				      Table(tabSetup));
  }
  // Feed
  {
    TableDesc td = MSFeed::requiredTableDesc();
 
    SetupNewTable tabSetup(itsMS->feedTableName(),
			   MSFeed::requiredTableDesc(), Table::New);
    MSFeed::addColumnToDesc (td, MSFeed::FOCUS_LENGTH);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::FEED),
				      Table(tabSetup));
  }
  // Flag
  {
    SetupNewTable tabSetup(itsMS->flagCmdTableName(),
			   MSFlagCmd::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::FLAG_CMD),
				      Table(tabSetup));
  }
  // Field
  {
    SetupNewTable tabSetup(itsMS->fieldTableName(),
			   MSField::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::FIELD),
				      Table(tabSetup));
  }
  // History
  {
    SetupNewTable tabSetup(itsMS->historyTableName(),
			   MSHistory::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::HISTORY),
				      Table(tabSetup));
  }
  // Observation
  {
    SetupNewTable tabSetup(itsMS->observationTableName(),
			   MSObservation::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::OBSERVATION),
				      Table(tabSetup));
  }
  // Pointing
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

  // Polarization
  {
    TableDesc td = MSPolarization::requiredTableDesc();
    SetupNewTable tabSetup(itsMS->polarizationTableName(),
			   td, Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::POLARIZATION),
				      Table(tabSetup));
  }
  // Processor
  {
    SetupNewTable tabSetup(itsMS->processorTableName(),
			   MSProcessor::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::PROCESSOR),
				      Table(tabSetup));
  }
  // Source
  {
    TableDesc td = MSSource::requiredTableDesc();
    MSSource::addColumnToDesc (td, MSSource::POSITION);
    MSSource::addColumnToDesc (td, MSSource::TRANSITION);
    MSSource::addColumnToDesc (td, MSSource::REST_FREQUENCY);
    MSSource::addColumnToDesc (td, MSSource::SYSVEL);
    SetupNewTable tabSetup(itsMS->sourceTableName(),
			   td, Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::SOURCE),
				      Table(tabSetup));
  }
  // Spectral Window
  {
    TableDesc td = MSSpectralWindow::requiredTableDesc();
    MSSpectralWindow::addColumnToDesc (td, MSSpectralWindow::BBC_NO);
    MSSpectralWindow::addColumnToDesc (td, MSSpectralWindow::ASSOC_SPW_ID);
    MSSpectralWindow::addColumnToDesc (td, MSSpectralWindow::ASSOC_NATURE);
    SetupNewTable tabSetup(itsMS->spectralWindowTableName(),
			   td, Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::SPECTRAL_WINDOW),
				      Table(tabSetup));

  }

  // State
  {
    SetupNewTable tabSetup(itsMS->stateTableName(),
			   MSState::requiredTableDesc(),
			   Table::New);
    itsMS->rwKeywordSet().defineTable(MS::keywordName(MS::STATE),
				      Table(tabSetup));
  }
  // Syscal
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

void ASDM2MSFiller::addData (bool                complexData,
			     vector<double>     &time_,
			     vector<int>        &antennaId1_,
			     vector<int>        &antennaId2_,
			     vector<int>        &feedId1_,
			     vector<int>        &feedId2_,
			     vector<int>        &dataDescId_,
			     int                 processorId_,
			     vector<int>        &fieldId_,
			     vector<double>     &interval_,
			     vector<double>     &exposure_,
			     vector<double>     &timeCentroid_,
			     int                 scanNumber_,
			     int                 arrayId_,
			     int                 observationId_,
			     vector<int>        &stateId_,
			     vector<double>     &uvw_,
			     vector<vector<unsigned int> >  &dataShape_,
			     vector<float *>    &data_,
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
  Matrix<Complex> data;
  Matrix<Float>   float_data;
  Matrix<Bool>    flag;

  int cRow0 = 0;
  for (unsigned int cRow = itsMSMainRow; cRow < itsMSMainRow+theSize; cRow++) {      
    int numCorr = dataShape_.at(cRow0).at(0);
    int numChan = dataShape_.at(cRow0).at(1);

    Vector<float>   ones(IPosition(1, numCorr), 1.0);

    if (complexData) {
      data.takeStorage(IPosition(2, numCorr, numChan), (Complex *)(data_.at(cRow0)), COPY);
      itsMSCol->data().put(cRow, data);
    }
    else {
      // Float data.
      float_data.takeStorage(IPosition(2, numCorr, numChan), data_.at(cRow0), SHARE);
      itsMSCol->floatData().put(cRow, float_data);
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
void ASDM2MSFiller::addField(const string&      name_,
			     const string&      code_,
			     double		time_,
			     vector<double>&	delay_dir_,
			     vector<double>&	phase_dir_,
			     vector<double>&	reference_dir_,
			     int               source_id_) {
  uInt						crow;
  // cout << "\naddField : entering";
  Matrix<Double>				delayDir(IPosition(2, 2, 1), &delay_dir_[0], SHARE);
  Matrix<Double>				referenceDir(IPosition(2, 2, 1), &reference_dir_[0], SHARE);
  Matrix<Double>				phaseDir(IPosition(2, 2,1), &phase_dir_[0], SHARE);

  MSField msfield = itsMS -> field();
  MSFieldColumns msfieldCol(msfield);

  crow = msfield.nrow();
  msfield.addRow();

  msfieldCol.name().put(crow, name_);
  msfieldCol.code().put(crow, code_);
  msfieldCol.time().put(crow, time_);
  msfieldCol.numPoly().put(crow, 0);
  msfieldCol.delayDir().put(crow, delayDir);
  msfieldCol.phaseDir().put(crow, phaseDir);
  msfieldCol.referenceDir().put(crow, referenceDir);
  msfieldCol.sourceId().put(crow, source_id_);
  /*
    msfieldCol.sourceId().put(crow, -1);
  */
  msfieldCol.flagRow().put(crow, False);
  // cout << "\naddField : exiting";
  msfield.flush();
  // cout << "\n";
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
					 //					 const vector<Stokes::StokesTypes>& corr_type_,
					 const vector<int>& corr_product_) {
  uInt crow;
  int  i;
  Vector<Int>  corrType(IPosition(1, num_corr_), (int *)&corr_type_[0], SHARE);
  Matrix<Int>  corrProduct(2, num_corr_);
  MSPolarization mspolar = itsMS -> polarization();
  MSPolarizationColumns mspolarCol(mspolar);

  const char** p=getPolCombinations(num_corr_);
  for (i=0; i < num_corr_; i++) {
    corrProduct(0, i) = Stokes::receptor1(Stokes::type(p[i]));
    corrProduct(1, i) = Stokes::receptor2(Stokes::type(p[i]));
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
  MSState msstate = itsMS -> state();
  MSStateColumns msstateCol(msstate);
  uInt crow = msstate.nrow();
  
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
  
  msstate.addRow();
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
			      vector<double>& position_,
			      vector<double>& proper_motion_,
			      vector<string>& transition_,
			      vector<double>& rest_frequency_,
			      vector<double>& sysvel_){
  MSSource mssource = itsMS -> source();
  MSSourceColumns mssourceCol(mssource);

      
  // Add a new row.
  int crow = mssource.nrow();
  mssource.addRow();

  Vector<Double> direction(IPosition(1, 2), &direction_[0], SHARE);
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
  mssourceCol.direction().put(crow,direction);
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
void ASDM2MSFiller::addWeather(int             antenna_id_,
			       double          time_,
			       double          interval_,
			       float           pressure_,
			       bool            pressure_flag_,
			       float           rel_humidity_,
			       bool            rel_humidity_flag_,
			       float           temperature_,
			       bool            temperature_flag_,
			       float           wind_direction_,
			       bool            wind_direction_flag_,
			       float           wind_speed_,
			       bool            wind_speed_flag_,
			       bool            has_dew_point_,
			       float           dew_point_,
			       bool            dew_point_flag_,
			       int             wx_station_id_,
			       vector<double>& wx_station_position_) {
  
  MSWeather msweather = itsMS -> weather();
  MSWeatherColumns msweatherCol(msweather);

  int crow;
  
  crow = msweather.nrow();
  msweather.addRow();
  
  msweatherCol.antennaId().put(crow, antenna_id_);
  msweatherCol.interval().put(crow, interval_);
  msweatherCol.time().put(crow, time_);

  msweatherCol.pressure().put(crow, pressure_);
  msweatherCol.pressureFlag().put(crow, pressure_flag_);
  msweatherCol.relHumidity().put(crow, rel_humidity_);
  msweatherCol.relHumidityFlag().put(crow, rel_humidity_flag_);
  msweatherCol.temperature().put(crow, temperature_);
  msweatherCol.temperatureFlag().put(crow, temperature_flag_);
  msweatherCol.windDirection().put(crow, wind_direction_);
  msweatherCol.windDirectionFlag().put(crow, wind_direction_flag_);
  msweatherCol.windSpeed().put(crow, wind_speed_);
  msweatherCol.windSpeedFlag().put(crow, wind_speed_flag_);
  if (has_dew_point_) {
    msweatherCol.dewPoint().put(crow, dew_point_);
    msweatherCol.dewPointFlag().put(crow, dew_point_flag_);
  }

  ScalarColumn<int> nsWXStationId(msweather, "NS_WX_STATION_ID");
  nsWXStationId.put(crow, wx_station_id_);
  ArrayColumn<double> nsWXStationPosition(msweather, "NS_WX_STATION_POSITION");
  nsWXStationPosition.put(crow, Vector<double>(IPosition(1, 3), &wx_station_position_[0], SHARE));

  msweather.flush();
}

void ASDM2MSFiller::end(double time_) {
  itsMS->flush();
  delete itsMS;
}

