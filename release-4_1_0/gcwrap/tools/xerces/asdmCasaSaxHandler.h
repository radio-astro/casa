#ifndef asdmCASASAXHANDLER_H
#define asdmCASASAXHANDLER _H
/*
 *
 * Spawn from CasaSaxHandler to parse ASDM XML tables
 * Paulo Cortes
 *
 */

#include <xercesc/sax2/DefaultHandler.hpp>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/DataType.h>
#include <vector>
///
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casa/Arrays/Array.h>



using namespace xercesc;
using namespace std;

namespace casa {

class Table;

class asdmCasaSaxHandler : public DefaultHandler {
	public :
		asdmCasaSaxHandler(Table &, const String &tabName);
		void characters (const XMLCh * const chars,
				 const unsigned int length);
		void startElement (const XMLCh * const uri,
				   const XMLCh * const localname,
				   const XMLCh * const qname,
				   const Attributes & attr);
		void endElement (const XMLCh * const uri,
				 const XMLCh * const localname,
				 const XMLCh * const qname);
		void fatalError(const SAXParseException&);
		void setDescriptor (const String tabName);
                int countIt(char *str);
                Array<String> sSplit(char *str);
                Array<Double> sSplit_d(char *str);
                Array<Int>    sSplit_i(char *str);

		void    assignColumn(char *str, Array<Double> &doubleColumn);
                void    assignColumn(char *str, Array<Int> &intColumn);
                void    assignColumn(char *str, Array<String> &stringColumn);
                void    assignColumn(char *str, Array<Bool> &stringColumn);
                void    assignColumn(char *str, Array<Complex> &stringColumn);

	private :
		Bool flagIn;
		Bool isRow;
		Bool itsEntityRef;
                Int  nRow;
		Int nElements;
		String itsElement;
		String currentElement;
		String valueName;
		String tName;
		String xmlTableName;
		//TableDesc itsTableDesc;
		Table &itsTable;
		std::string  itsTag;
		std::string  tagList;
                const Int nMaxBuffer;
                

		//
		//Scalar and Array objects are declared here
                //There is a finite number of datatypes used at each table
                //However, I cannot have repeated names attached to each 
                //column, if I have to Int xml tags I have to assign unique
                //names to the Table columns, i.e. 
                //
                //        IntScalarColumn attach(itsTable,"xmlTag1")
                //        IntScalarColumn attach(itsTable,"xmlTag2")
                //
                //this will compile, but produce a segmentation fault signal
                //For the sake of readability, at considering that (most likely)
                //I won't maintain this code, I decided to use the tag names 
                //for each column. This is inneficient, but will allow the next
                //developer to better understand what I did, and to add, or modify,
                // new tables more easily



                //
	        //Scan table definition
		//
		ScalarColumn<String> execBlockId;
                ScalarColumn<Int>    scanNumber;
                ScalarColumn<String> startTime;
                ScalarColumn<String> endTime;
                ScalarColumn<Int>    numSubScan;
                ScalarColumn<Int>    numIntent;
                ArrayColumn<String>  scanIntent;
                ScalarColumn<Bool>   flagRow;
                ScalarColumn<Int>    numField;
                ScalarColumn<String> sourceName;
                ArrayColumn<String>  fieldName;

                //Main table

                ScalarColumn<String> configDescriptionId;
                ScalarColumn<String> fieldId;
                ScalarColumn<String> time;
                ArrayColumn<String>  stateId;
                ScalarColumn<Int>    subscanNumber;
                ScalarColumn<Int>    integrationNumber;
                ArrayColumn<Double>  uvw;
                ArrayColumn<String>  exposure;
                ArrayColumn<String>  timeCentroid;
                ScalarColumn<String>  dataOid;
                ArrayColumn<Int>     flagAnt;
                ArrayColumn<Int>     flagPol;
                ArrayColumn<Int>     flagBaseband;
                ScalarColumn<String> interval;
                ScalarColumn<Int>    subintegrationNumber;
               
                //AlmaCorrelatorModeId Table

                ScalarColumn<String> almaCorrelatorModeId;
                ScalarColumn<Int>    numBaseband;
                ArrayColumn<Int>     basebandIndex;
                ScalarColumn<String> accumMode;
                ArrayColumn<Int>     basebandConfig;
                ScalarColumn<Int>    binMode;
                ScalarColumn<Bool>   quantization;
                ScalarColumn<String> windowFunction;
                ArrayColumn<Int>     axesOrderArray;

		//Antenna Table

                ScalarColumn<String> antennaId;
                ScalarColumn<String> stationId;
                ScalarColumn<String> name;
                ScalarColumn<String> type;
                ScalarColumn<Double> xPosition;
                ScalarColumn<Double> yPosition;
                ScalarColumn<Double> zPosition;
                ScalarColumn<Double> xOffset;
                ScalarColumn<Double> yOffset;
                ScalarColumn<Double> zOffset;
                ScalarColumn<Double> dishDiameter;
                ScalarColumn<String> assocAntennaId;

                //ConfigDescription Table
                //The AUX suffix in antennaIdAUX stands for
		//auxiliary
                
                ArrayColumn <String>  dataDescriptionId;
                ArrayColumn <String>  antennaIdAUX;
                ArrayColumn <Int>     feedId;
                ScalarColumn<String>  processorId;
                ArrayColumn <String>  switchCycleId;
                ScalarColumn<Int>     numAntenna;
                ScalarColumn<Int>     numFeed;
                ArrayColumn <Int>     numSubBand;
                ScalarColumn<Int>     correlationMode;
                ScalarColumn<Int>     atmPhaseCode;
                ArrayColumn <Int>     phasedArrayList;
                ArrayColumn <Bool>    flagAntAUX;
		
		//DataDescription Table

                ScalarColumn<String>  dataDescriptionIdDDT;
                ScalarColumn<String>  polOrHoloId;
                ScalarColumn<String>  spectralWindowId;

		//ExecBlock Table

                ScalarColumn<String> telescopeName;
                ScalarColumn<String> configName;
                ScalarColumn<Double> baseRangeMin;
                ScalarColumn<Double> baseRangeMax;
                ScalarColumn<Double> baseRangeMinor;
                ScalarColumn<Double> baseRangeMajor;
                ScalarColumn<Double> basePa;
                ScalarColumn<String> timeInterval;
                ScalarColumn<String> observerName;
                ArrayColumn<String>  observingLog;
                ArrayColumn<String>  schedulerMode;
                ScalarColumn<String> projectId;
                ScalarColumn<Double> siteLongitude;
                ScalarColumn<Double> siteLatitude;
                ScalarColumn<Double> siteAltitude;
                ScalarColumn<String> execBlockUID;
                ScalarColumn<Bool>   aborted;

		//Feed Table

                ScalarColumn<Int>    feedIdFT;
                ArrayColumn<Int>     receiverId;
                ScalarColumn<Int>    numReceptors;
                ArrayColumn<Double>  beamOffset;
                ArrayColumn<Double>  focusReference;
                ArrayColumn<String>  polarizationType;
                ArrayColumn<Complex> polResponse;
                ArrayColumn<Double>  receptorAngle;
                ArrayColumn<String>  beamId;
                ScalarColumn<Int>    feedNum;
                ScalarColumn<Float>  illumOffset;
                ScalarColumn<Float>  illumOffsetPa;

	        //Field Table

                ScalarColumn<String> fieldNameAUX;
                ScalarColumn<String> code;
                ScalarColumn<Int>    numPoly;
                ArrayColumn<Double>  delayDir;
                ArrayColumn<Double>  phaseDir;
                ArrayColumn<Double>  referenceDir;
                ArrayColumn<String>  assocFieldId;
                ScalarColumn<String> ephemerisId;
                ScalarColumn<Int>    sourceId;
                ScalarColumn<String> assocNature;

		//Polarization Table

		ScalarColumn<String> polarizationId;
                ScalarColumn<Int>    numCorr;
                ArrayColumn<Int>     corrType;
                ArrayColumn<Int>     corrProduct;

                //Processor Table

		ScalarColumn<String> subType;

		//Reciever Table

                ScalarColumn<Int>    receiverIdAUX;
                ScalarColumn<Int>    numLo;
                ScalarColumn<String> frequencyBand;
                ArrayColumn<Double>  freqLo;
                ScalarColumn<Double> stability;
                ArrayColumn<Int>     sidebandLo;
                ScalarColumn<Double> tDewar;
                ScalarColumn<String> stabilityDuration;
                ScalarColumn<String> dewarName;
                ScalarColumn<Bool>   stabilityflag;

		//SBSummary Table

                ScalarColumn<String> sbId;
                ScalarColumn<String> obsUnitSetId;
                ScalarColumn<String> sbIntent;
                ScalarColumn<String> sbType;
                ScalarColumn<String> sbDuration;
                ScalarColumn<Int>    numScan;
                ScalarColumn<Int>    numberRepeats;
                ArrayColumn<String>  weatherConstraint;
                ArrayColumn<String>  scienceGoal;
                ScalarColumn<Double> raCenter;
                ScalarColumn<Double> decCenter;
                ScalarColumn<Double> frequency;
                ArrayColumn<String>  observingMode;

		//Source Table

                ScalarColumn<Int>    numLines;
                ArrayColumn<Double>  direction;
                ArrayColumn<Double>  properMotion;
                ScalarColumn<Int>    sourceParameterId;
                ScalarColumn<String> catalog;
                ScalarColumn<Int>    calibrationGroup;
                ArrayColumn<Double>  position;
                ArrayColumn<String>  transition;
                ArrayColumn<Double>  restFrequency;
                ArrayColumn<Double>  sysVel;
                ScalarColumn<String> sourceModel;
                ScalarColumn<Double> deltaVel;
                ArrayColumn<Double>  rangeVel;

		//SpectralWindow Table

                ScalarColumn<Int>    numChan;
                ScalarColumn<Double> refFreq;
                ArrayColumn<Double>  chanFreq;
                ArrayColumn<Double>  chanWidth;
                ArrayColumn<Double>  effectiveBw;
                ArrayColumn<Double>  resolution;
                ScalarColumn<Double> totBandwidth;
                ScalarColumn<Int>    netSideband;
                ArrayColumn<String>  assocSpectralWindowId;
                ScalarColumn<Int>    dopplerId;
                ScalarColumn<Int>    measFreqRef;
                ScalarColumn<Int>    bbcNo;
                ScalarColumn<Int>    bbcSideband;
                ScalarColumn<Int>    ifConvChain;
                ScalarColumn<Int>    freqGroup;
                ScalarColumn<String> freqGroupName;
                ArrayColumn<String>  assocNatureAUX;

		//State Table

                ScalarColumn<String> stateIdAUX;
                ScalarColumn<Bool>   sig;
                ScalarColumn<Bool>   ref;
                ScalarColumn<Int>    calloadNum;
                ScalarColumn<String> obsMode;
                ScalarColumn<String> obsIntent;
                ScalarColumn<Float>  weight;

		//Subscan Table

                ScalarColumn<String> subscanIntent;
                ScalarColumn<Int>    numberIntegration;
                ArrayColumn<Int>     numberSubintegration;
                ScalarColumn<String> subscanMode;

		//SwitchCycle Table

                ScalarColumn<String> switchCycleIdAUX;
                ScalarColumn<Int>    numStep;
                ArrayColumn<Double>   weightArray;
                ArrayColumn<Double>  offsetArray;
                ArrayColumn<Double>  freqOffsetArray;

		//CalCurve Table

                ScalarColumn<String> calDataId;
                ScalarColumn<String> calReductionId;
                ScalarColumn<String> antennaName;
                ScalarColumn<String> startValidTime;
                ScalarColumn<String> endValidTime;
                ScalarColumn<String> typeCurve;
                ArrayColumn<Double>  curve;
                ArrayColumn<Double>  frequencyRange;
                ScalarColumn<String> refAntennaName;
                ScalarColumn<String> receiverBand;
                ScalarColumn<String> timeOrigin;


		//CalData Table

                ArrayColumn<Int>     scanSet;
                ScalarColumn<String> calType;
                ScalarColumn<String> startTimeObserved;
                ScalarColumn<String> endTimeObserved;
                ScalarColumn<String> calDataType;
                ScalarColumn<Int>    frequencyGroup;
                ArrayColumn<String>  fieldCode;
                ArrayColumn<String>  sourceNameAUX;
                ArrayColumn<String>  sourceCode;
                ScalarColumn<String> assocCalDataId;
                ScalarColumn<String> assocCalNature;

		//CalPhase Table

                ScalarColumn<String> basebandName;
                ScalarColumn<Int>    numBaseline;
                ArrayColumn<String>  antennaNames;
                ArrayColumn<Double>  decorrelationFactor;
                ArrayColumn<Double>  uncorrPhaseRms;
                ArrayColumn<Double>  corrPhaseRms;
                ArrayColumn<Double>  statPhaseRms;
                ArrayColumn<Bool>    corrValidity;

		//CalReduction Table

                ScalarColumn<Int>    numApplied;
                ScalarColumn<Int>    numParam;
                ScalarColumn<String> timeReduced;
                ArrayColumn<String>  calAppliedArray;
                ArrayColumn<String>  paramSet;
                ScalarColumn<String> messages;
                ScalarColumn<String> software;
                ScalarColumn<String> softwareVersion;
                ScalarColumn<String> invalidConditions;

		//CalSeeingTable

                ScalarColumn<Int>    numBaseLength;
                ArrayColumn<Double>  baseLength;
                ScalarColumn<Double> seeing;
                ScalarColumn<Double> seeingFrequency;
                ScalarColumn<Double> seeingFreqBandwidth;
                ScalarColumn<Double> exponent;

		//CalAtmosphere Table

                ScalarColumn<Int>    numFreq;
                ArrayColumn<Double>  frequencySpectrum;
                ScalarColumn<String> syscalType;
                ArrayColumn<Double>  tSysSpectrum;
                ArrayColumn<Double>  tRecSpectrum;
                ArrayColumn<Double>  tAtmSpectrum;
                ArrayColumn<Double>  tauSpectrum;
                ArrayColumn<Double>  sbGainSpectrum;
                ArrayColumn<Double>  forwardEffSpectrum;
                ScalarColumn<Double> groundPressure;
                ScalarColumn<Double> groundTemperature;
                ScalarColumn<Double> groundRelHumidity;
                ArrayColumn<Double>  tSys;
                ArrayColumn<Double>  tRec;
                ArrayColumn<Double>  tAtm;
                ArrayColumn<Double>  sbGain;
                ArrayColumn<Double>  water;
                ArrayColumn<Double>  forwardEfficiency;
                ArrayColumn<Double>  tau;

		//CalAmpli Table

                ArrayColumn<Double>  apertureEfficiencyError;
                ArrayColumn<Double>  uncorrectedApertureEfficiency;
                ArrayColumn<Double>  correctedApertureEfficiency;
                ArrayColumn<Bool>    correctionValidity;

                //CalBandpass Table

                ArrayColumn<Double>  freqLimits;
                ScalarColumn<Int>    numPhasePoly;
                ScalarColumn<Int>    numAmpliPoly;
                ArrayColumn<Double>  phaseCurve;
                ArrayColumn<Double>  ampliCurve;
        
                //AlmaRadiometer Table

                ScalarColumn<String> modeId;
                ScalarColumn<Int>    numBand;

		//CalDelay Table

                ArrayColumn<Double>  delayOffset;
                ArrayColumn<Double>  delayError;
                ScalarColumn<Double> crossDelayOffset;
                ScalarColumn<Double> crossDelayOffsetError;

		//CalDevice Table

                ScalarColumn<Int>    numCalload;
                ArrayColumn<Double>  noiseCal;
                ArrayColumn<Double>  temperatureLoad;
                ArrayColumn<Double>  calEff;

                //CalFlux Table

                ScalarColumn<Int>    numStokes;
                ArrayColumn<Int>     Stokes;
                ArrayColumn<Double>  frequencyAUX;
                ArrayColumn<Double>  frequencyWidth;
                ArrayColumn<Double>  flux;
                ArrayColumn<Double>  fluxError;
                ScalarColumn<String> fluxMethod;
                ArrayColumn<Double>  size;
                ArrayColumn<Double>  sizeError;
                ArrayColumn<Double>  PA;
                ArrayColumn<Double>  PAError;


                //CalFocus Table

                ArrayColumn<Double>  offset;
                ArrayColumn<Double>  error;
                ScalarColumn<String> method;
                ArrayColumn<Double>  pointingDirection;
                ArrayColumn<Bool>    wasFixed;


                //CalFocusModel Table

                ScalarColumn<Int>    numCoeff;
                ArrayColumn<Double>  focusRMS;
                ArrayColumn<String>  coeffName;
                ArrayColumn<String>  coeffFormula;
                ArrayColumn<Double>  coeffValue;
                ArrayColumn<Double>  coeffError;
                ArrayColumn<Bool>    coeffFixed;
                ScalarColumn<String> focusModel;
                ScalarColumn<Int>    numSourceObs;


                //CalGain Table

                ArrayColumn<Double>  gain;
                ArrayColumn<Bool>    gainValid;
                ArrayColumn<Double>  fit;
                ArrayColumn<Double>  fitWeight;
                ScalarColumn<Bool>   totalGainValid;
                ScalarColumn<Double> totalFit;
                ScalarColumn<Double> totalFitWeight;

                //CalHolography Table
 
                ScalarColumn<Int>    numScrew;
                ArrayColumn<Double>  focusPosition;
                ScalarColumn<Double> rawRms;
                ScalarColumn<Double> weightedRms;
                ArrayColumn<String>  screwName;
                ArrayColumn<Double>  screwMotion;
                ArrayColumn<Double>  screwMotionError;
                ScalarColumn<Int>    panelModes;
                ScalarColumn<String> beamMapUID;
                ScalarColumn<String> surfaceMapUID;


                //CalPointing Table

                ArrayColumn<Double>  collOffset;
                ArrayColumn<Double>  collError;
                ScalarColumn<String> pointingMethod;
                ScalarColumn<String> mode;
                ArrayColumn<Double>  beamWidth;
                ArrayColumn<Double>  beamWidthError;
                ScalarColumn<Double> beamPA;
                ScalarColumn<Double> beamPAError;
                ScalarColumn<Double> peakIntensity;
                ScalarColumn<Double> peakIntensityError;

                //CalPointingModel Table


                ScalarColumn<Int>    numObs;
                ScalarColumn<Int>    numFormula;
                ScalarColumn<Double> azimuthRms;
                ScalarColumn<Double> elevationRms;
                ScalarColumn<Double> skyRms;
                ArrayColumn<Double>  coeffVal;
                ScalarColumn<String> pointingModel;

		//CalPosition Table

                ArrayColumn<Double>  positionOffset;
                ArrayColumn<Double>  positionErr;
                ScalarColumn<String> delayRms;
                ScalarColumn<Double> phaseRms;
                ScalarColumn<Double> axesOffset;
                ScalarColumn<Bool>   axesOffsetFixed;
                ScalarColumn<Double> axesOffsetErr;
                ScalarColumn<String> positionMethod;
                ArrayColumn<String>  refAntennaNames;

                //CalPrimaryBeam Table

                ScalarColumn<Int>    numPixelX;
                ScalarColumn<Int>    numPixelY;
                ScalarColumn<Double> refX;
                ScalarColumn<Double> refY;
                ScalarColumn<Double> valX;
                ScalarColumn<Double> valY;
                ScalarColumn<Double> incX;
                ScalarColumn<Double> incY;
                ArrayColumn<Double>  amplitude;
                ArrayColumn<Double>  phase;

	        //CalWVR Table
       
                ScalarColumn<String> WVRMethod;
                ArrayColumn<Double>  pathCoeff;

		//Doppler Table

                ScalarColumn<Int>    transitionIndex;
                ScalarColumn<Double> velDef;
		
		//Observation Table

                ScalarColumn<String> observationId;

                //FlagCmd Table

                ScalarColumn<String> reason;
                ScalarColumn<Int>    level;
                ScalarColumn<Int>    severity;
                ScalarColumn<Bool>   applied;
                ScalarColumn<String> command;

		//Focus Table

                ScalarColumn<String> focusModelId;
                ScalarColumn<Double> xFocusPosition;
                ScalarColumn<Double> yFocusPosition;
                ScalarColumn<Double> zFocusPosition;
                ScalarColumn<Double> focusTracking;
                ScalarColumn<Double> xFocusOffset;
                ScalarColumn<Double> yFocusOffset;
                ScalarColumn<Double> zFocusOffset;

		//GainTracking Table

                ScalarColumn<Double> attenuator;
                ScalarColumn<String> delayoff1;
                ScalarColumn<String> delayoff2;
                ScalarColumn<Double> phaseoff1;
                ScalarColumn<Double> phaseoff2;
                ScalarColumn<Double> rateoff1;
                ScalarColumn<Double> rateoff2;
                ScalarColumn<Double> samplingLevel;
                ScalarColumn<Double> phaseRefOffset;

		//History Table

                ScalarColumn<String> message;
                ScalarColumn<String> priority;
                ScalarColumn<String> origin;
                ScalarColumn<String> objectId;
                ScalarColumn<String> application;
                ScalarColumn<String> cliCommand;
                ScalarColumn<String> appParms;

		//Holography Table

                ScalarColumn<String> holographyId;
                ScalarColumn<Double> distance;
                ScalarColumn<Double> focus;

		//Pointing Table

                ScalarColumn<Int>    pointingModelId;
                ArrayColumn<Double>  target;
                ArrayColumn<Double>  encoder;
                ScalarColumn<Bool>   pointingTracking;
                ArrayColumn<Double>  sourceOffset;
                ScalarColumn<Bool>   phaseTracking;
                ScalarColumn<Bool>   overTheTop;


		//Pointing Table

                ArrayColumn<Int>     phaseRmsAUX;

		//SourceParameter Table

                ScalarColumn<Int>    numDep;
                ArrayColumn<Int>     stokeParameter;
                ArrayColumn<Double>  frequencyInterval;
                ArrayColumn<Double>  fluxErr;
                ArrayColumn<Double>  positionAngle;
                ArrayColumn<Double>  sizeErr;
                ArrayColumn<Double>  positionAngleErr;
                ArrayColumn<Int>     depSourceParameterId;


                //SquareLawDetector Table

                ScalarColumn<String> squareLawDetectorId;
                ScalarColumn<String> bandType;

		//SysCal Table
                ScalarColumn<Int>    numLoad;
                ArrayColumn<Int>     calLoad;
                ArrayColumn<Double>  feff;
                ArrayColumn<Double>  aeff;
                ScalarColumn<Double> phaseDiff;
                ScalarColumn<Double> sbgain;
                ArrayColumn<Double>  tcal;
                ArrayColumn<Double>  trx;
                ArrayColumn<Double>  tsys;
                ArrayColumn<Double>  tsky;
                ArrayColumn<Double>  tant;
                ArrayColumn<Double>  tantTsys;
                ArrayColumn<Double>  pwvPath;
                ArrayColumn<Double>  dpwvPath;
                ArrayColumn<Double>  feffSpectrum;
                ArrayColumn<Double>  tcalSpectrum;
                ArrayColumn<Double>  sbgainSpectrum;
                ArrayColumn<Double>  trxSpectrum;
                ArrayColumn<Double>  tskySpectrum;
                ArrayColumn<Double>  tsysSpectrum;
                ArrayColumn<Double>  tantSpectrum;
                ArrayColumn<Double>  tantTsysSpectrum;
                ArrayColumn<Double>  pwvPathSpectrum;
                ArrayColumn<Double>  dpwvPathSpectrum;
                ScalarColumn<Int>    numPolyFreq;
                ScalarColumn<Double> freqOrigin;
                ArrayColumn<Double>  delayCurve;
                ArrayColumn<Double>  bandpassCurve;
                ScalarColumn<Bool>   phasediffFlag;
                ScalarColumn<Bool>   sbgainFlag;
                ScalarColumn<Bool>   tauFlag;
                ScalarColumn<Bool>   tcalFlag;
                ScalarColumn<Bool>   trxFlag;
                ScalarColumn<Bool>   tskyFlag;
                ScalarColumn<Bool>   tsysFlag;
                ScalarColumn<Bool>   tantFlag;
                ScalarColumn<Bool>   tantTsysFlag;
                ScalarColumn<Bool>   pwvPathFlag;

		//TotalPower Table

                ArrayColumn<Double>  floatData;

		//WVMCal Table

                ScalarColumn<String> calibrationMode;
                ScalarColumn<String> operationMode;
                ScalarColumn<Double> wvrefModel;

		//Weather Table

                ScalarColumn<Double> pressure;
                ScalarColumn<Double> relHumidity;
                ScalarColumn<Double> temperature;
                ScalarColumn<Double> windDirection;
                ScalarColumn<Double> windSpeed;
                ScalarColumn<Double> windMax;
                ScalarColumn<Bool>   pressureFlag;
                ScalarColumn<Bool>   relHumidityFlag;
                ScalarColumn<Bool>   temperatureFlag;
                ScalarColumn<Bool>   windDirectionFlag;
                ScalarColumn<Bool>   windSpeedFlag;
                ScalarColumn<Bool>   windMaxFlag;
                ScalarColumn<Double> dewPoint;
                ScalarColumn<Bool>   dewPointFlag;

		//ASDM Table

                ScalarColumn<String> Name;
                ScalarColumn<Int>    NumberRows;








 

};

#endif

}
