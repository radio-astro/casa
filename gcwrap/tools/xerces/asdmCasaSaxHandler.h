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

namespace casacore{

class Table;
}

namespace casa {


class asdmCasaSaxHandler : public DefaultHandler {
	public :
		asdmCasaSaxHandler(casacore::Table &, const casacore::String &tabName);
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
		void setDescriptor (const casacore::String tabName);
                int countIt(char *str);
                casacore::Array<casacore::String> sSplit(char *str);
                casacore::Array<casacore::Double> sSplit_d(char *str);
                casacore::Array<casacore::Int>    sSplit_i(char *str);

		void    assignColumn(char *str, casacore::Array<casacore::Double> &doubleColumn);
                void    assignColumn(char *str, casacore::Array<casacore::Int> &intColumn);
                void    assignColumn(char *str, casacore::Array<casacore::String> &stringColumn);
                void    assignColumn(char *str, casacore::Array<casacore::Bool> &stringColumn);
                void    assignColumn(char *str, casacore::Array<casacore::Complex> &stringColumn);

	private :
		casacore::Bool flagIn;
		casacore::Bool isRow;
		casacore::Bool itsEntityRef;
                casacore::Int  nRow;
		casacore::Int nElements;
		casacore::String itsElement;
		casacore::String currentElement;
		casacore::String valueName;
		casacore::String tName;
		casacore::String xmlTableName;
		//casacore::TableDesc itsTableDesc;
		casacore::Table &itsTable;
		std::string  itsTag;
		std::string  tagList;
                const casacore::Int nMaxBuffer;
                

		//
		//Scalar and casacore::Array objects are declared here
                //There is a finite number of datatypes used at each table
                //However, I cannot have repeated names attached to each 
                //column, if I have to casacore::Int xml tags I have to assign unique
                //names to the casacore::Table columns, i.e. 
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
		casacore::ScalarColumn<casacore::String> execBlockId;
                casacore::ScalarColumn<casacore::Int>    scanNumber;
                casacore::ScalarColumn<casacore::String> startTime;
                casacore::ScalarColumn<casacore::String> endTime;
                casacore::ScalarColumn<casacore::Int>    numSubScan;
                casacore::ScalarColumn<casacore::Int>    numIntent;
                casacore::ArrayColumn<casacore::String>  scanIntent;
                casacore::ScalarColumn<casacore::Bool>   flagRow;
                casacore::ScalarColumn<casacore::Int>    numField;
                casacore::ScalarColumn<casacore::String> sourceName;
                casacore::ArrayColumn<casacore::String>  fieldName;

                //Main table

                casacore::ScalarColumn<casacore::String> configDescriptionId;
                casacore::ScalarColumn<casacore::String> fieldId;
                casacore::ScalarColumn<casacore::String> time;
                casacore::ArrayColumn<casacore::String>  stateId;
                casacore::ScalarColumn<casacore::Int>    subscanNumber;
                casacore::ScalarColumn<casacore::Int>    integrationNumber;
                casacore::ArrayColumn<casacore::Double>  uvw;
                casacore::ArrayColumn<casacore::String>  exposure;
                casacore::ArrayColumn<casacore::String>  timeCentroid;
                casacore::ScalarColumn<casacore::String>  dataOid;
                casacore::ArrayColumn<casacore::Int>     flagAnt;
                casacore::ArrayColumn<casacore::Int>     flagPol;
                casacore::ArrayColumn<casacore::Int>     flagBaseband;
                casacore::ScalarColumn<casacore::String> interval;
                casacore::ScalarColumn<casacore::Int>    subintegrationNumber;
               
                //AlmaCorrelatorModeId Table

                casacore::ScalarColumn<casacore::String> almaCorrelatorModeId;
                casacore::ScalarColumn<casacore::Int>    numBaseband;
                casacore::ArrayColumn<casacore::Int>     basebandIndex;
                casacore::ScalarColumn<casacore::String> accumMode;
                casacore::ArrayColumn<casacore::Int>     basebandConfig;
                casacore::ScalarColumn<casacore::Int>    binMode;
                casacore::ScalarColumn<casacore::Bool>   quantization;
                casacore::ScalarColumn<casacore::String> windowFunction;
                casacore::ArrayColumn<casacore::Int>     axesOrderArray;

		//Antenna Table

                casacore::ScalarColumn<casacore::String> antennaId;
                casacore::ScalarColumn<casacore::String> stationId;
                casacore::ScalarColumn<casacore::String> name;
                casacore::ScalarColumn<casacore::String> type;
                casacore::ScalarColumn<casacore::Double> xPosition;
                casacore::ScalarColumn<casacore::Double> yPosition;
                casacore::ScalarColumn<casacore::Double> zPosition;
                casacore::ScalarColumn<casacore::Double> xOffset;
                casacore::ScalarColumn<casacore::Double> yOffset;
                casacore::ScalarColumn<casacore::Double> zOffset;
                casacore::ScalarColumn<casacore::Double> dishDiameter;
                casacore::ScalarColumn<casacore::String> assocAntennaId;

                //ConfigDescription Table
                //The AUX suffix in antennaIdAUX stands for
		//auxiliary
                
                casacore::ArrayColumn <casacore::String>  dataDescriptionId;
                casacore::ArrayColumn <casacore::String>  antennaIdAUX;
                casacore::ArrayColumn <casacore::Int>     feedId;
                casacore::ScalarColumn<casacore::String>  processorId;
                casacore::ArrayColumn <casacore::String>  switchCycleId;
                casacore::ScalarColumn<casacore::Int>     numAntenna;
                casacore::ScalarColumn<casacore::Int>     numFeed;
                casacore::ArrayColumn <casacore::Int>     numSubBand;
                casacore::ScalarColumn<casacore::Int>     correlationMode;
                casacore::ScalarColumn<casacore::Int>     atmPhaseCode;
                casacore::ArrayColumn <casacore::Int>     phasedArrayList;
                casacore::ArrayColumn <casacore::Bool>    flagAntAUX;
		
		//DataDescription Table

                casacore::ScalarColumn<casacore::String>  dataDescriptionIdDDT;
                casacore::ScalarColumn<casacore::String>  polOrHoloId;
                casacore::ScalarColumn<casacore::String>  spectralWindowId;

		//ExecBlock Table

                casacore::ScalarColumn<casacore::String> telescopeName;
                casacore::ScalarColumn<casacore::String> configName;
                casacore::ScalarColumn<casacore::Double> baseRangeMin;
                casacore::ScalarColumn<casacore::Double> baseRangeMax;
                casacore::ScalarColumn<casacore::Double> baseRangeMinor;
                casacore::ScalarColumn<casacore::Double> baseRangeMajor;
                casacore::ScalarColumn<casacore::Double> basePa;
                casacore::ScalarColumn<casacore::String> timeInterval;
                casacore::ScalarColumn<casacore::String> observerName;
                casacore::ArrayColumn<casacore::String>  observingLog;
                casacore::ArrayColumn<casacore::String>  schedulerMode;
                casacore::ScalarColumn<casacore::String> projectId;
                casacore::ScalarColumn<casacore::Double> siteLongitude;
                casacore::ScalarColumn<casacore::Double> siteLatitude;
                casacore::ScalarColumn<casacore::Double> siteAltitude;
                casacore::ScalarColumn<casacore::String> execBlockUID;
                casacore::ScalarColumn<casacore::Bool>   aborted;

		//Feed Table

                casacore::ScalarColumn<casacore::Int>    feedIdFT;
                casacore::ArrayColumn<casacore::Int>     receiverId;
                casacore::ScalarColumn<casacore::Int>    numReceptors;
                casacore::ArrayColumn<casacore::Double>  beamOffset;
                casacore::ArrayColumn<casacore::Double>  focusReference;
                casacore::ArrayColumn<casacore::String>  polarizationType;
                casacore::ArrayColumn<casacore::Complex> polResponse;
                casacore::ArrayColumn<casacore::Double>  receptorAngle;
                casacore::ArrayColumn<casacore::String>  beamId;
                casacore::ScalarColumn<casacore::Int>    feedNum;
                casacore::ScalarColumn<casacore::Float>  illumOffset;
                casacore::ScalarColumn<casacore::Float>  illumOffsetPa;

	        //Field Table

                casacore::ScalarColumn<casacore::String> fieldNameAUX;
                casacore::ScalarColumn<casacore::String> code;
                casacore::ScalarColumn<casacore::Int>    numPoly;
                casacore::ArrayColumn<casacore::Double>  delayDir;
                casacore::ArrayColumn<casacore::Double>  phaseDir;
                casacore::ArrayColumn<casacore::Double>  referenceDir;
                casacore::ArrayColumn<casacore::String>  assocFieldId;
                casacore::ScalarColumn<casacore::String> ephemerisId;
                casacore::ScalarColumn<casacore::Int>    sourceId;
                casacore::ScalarColumn<casacore::String> assocNature;

		//Polarization Table

		casacore::ScalarColumn<casacore::String> polarizationId;
                casacore::ScalarColumn<casacore::Int>    numCorr;
                casacore::ArrayColumn<casacore::Int>     corrType;
                casacore::ArrayColumn<casacore::Int>     corrProduct;

                //Processor Table

		casacore::ScalarColumn<casacore::String> subType;

		//Reciever Table

                casacore::ScalarColumn<casacore::Int>    receiverIdAUX;
                casacore::ScalarColumn<casacore::Int>    numLo;
                casacore::ScalarColumn<casacore::String> frequencyBand;
                casacore::ArrayColumn<casacore::Double>  freqLo;
                casacore::ScalarColumn<casacore::Double> stability;
                casacore::ArrayColumn<casacore::Int>     sidebandLo;
                casacore::ScalarColumn<casacore::Double> tDewar;
                casacore::ScalarColumn<casacore::String> stabilityDuration;
                casacore::ScalarColumn<casacore::String> dewarName;
                casacore::ScalarColumn<casacore::Bool>   stabilityflag;

		//SBSummary Table

                casacore::ScalarColumn<casacore::String> sbId;
                casacore::ScalarColumn<casacore::String> obsUnitSetId;
                casacore::ScalarColumn<casacore::String> sbIntent;
                casacore::ScalarColumn<casacore::String> sbType;
                casacore::ScalarColumn<casacore::String> sbDuration;
                casacore::ScalarColumn<casacore::Int>    numScan;
                casacore::ScalarColumn<casacore::Int>    numberRepeats;
                casacore::ArrayColumn<casacore::String>  weatherConstraint;
                casacore::ArrayColumn<casacore::String>  scienceGoal;
                casacore::ScalarColumn<casacore::Double> raCenter;
                casacore::ScalarColumn<casacore::Double> decCenter;
                casacore::ScalarColumn<casacore::Double> frequency;
                casacore::ArrayColumn<casacore::String>  observingMode;

		//Source Table

                casacore::ScalarColumn<casacore::Int>    numLines;
                casacore::ArrayColumn<casacore::Double>  direction;
                casacore::ArrayColumn<casacore::Double>  properMotion;
                casacore::ScalarColumn<casacore::Int>    sourceParameterId;
                casacore::ScalarColumn<casacore::String> catalog;
                casacore::ScalarColumn<casacore::Int>    calibrationGroup;
                casacore::ArrayColumn<casacore::Double>  position;
                casacore::ArrayColumn<casacore::String>  transition;
                casacore::ArrayColumn<casacore::Double>  restFrequency;
                casacore::ArrayColumn<casacore::Double>  sysVel;
                casacore::ScalarColumn<casacore::String> sourceModel;
                casacore::ScalarColumn<casacore::Double> deltaVel;
                casacore::ArrayColumn<casacore::Double>  rangeVel;

		//SpectralWindow Table

                casacore::ScalarColumn<casacore::Int>    numChan;
                casacore::ScalarColumn<casacore::Double> refFreq;
                casacore::ArrayColumn<casacore::Double>  chanFreq;
                casacore::ArrayColumn<casacore::Double>  chanWidth;
                casacore::ArrayColumn<casacore::Double>  effectiveBw;
                casacore::ArrayColumn<casacore::Double>  resolution;
                casacore::ScalarColumn<casacore::Double> totBandwidth;
                casacore::ScalarColumn<casacore::Int>    netSideband;
                casacore::ArrayColumn<casacore::String>  assocSpectralWindowId;
                casacore::ScalarColumn<casacore::Int>    dopplerId;
                casacore::ScalarColumn<casacore::Int>    measFreqRef;
                casacore::ScalarColumn<casacore::Int>    bbcNo;
                casacore::ScalarColumn<casacore::Int>    bbcSideband;
                casacore::ScalarColumn<casacore::Int>    ifConvChain;
                casacore::ScalarColumn<casacore::Int>    freqGroup;
                casacore::ScalarColumn<casacore::String> freqGroupName;
                casacore::ArrayColumn<casacore::String>  assocNatureAUX;

		//State Table

                casacore::ScalarColumn<casacore::String> stateIdAUX;
                casacore::ScalarColumn<casacore::Bool>   sig;
                casacore::ScalarColumn<casacore::Bool>   ref;
                casacore::ScalarColumn<casacore::Int>    calloadNum;
                casacore::ScalarColumn<casacore::String> obsMode;
                casacore::ScalarColumn<casacore::String> obsIntent;
                casacore::ScalarColumn<casacore::Float>  weight;

		//Subscan Table

                casacore::ScalarColumn<casacore::String> subscanIntent;
                casacore::ScalarColumn<casacore::Int>    numberIntegration;
                casacore::ArrayColumn<casacore::Int>     numberSubintegration;
                casacore::ScalarColumn<casacore::String> subscanMode;

		//SwitchCycle Table

                casacore::ScalarColumn<casacore::String> switchCycleIdAUX;
                casacore::ScalarColumn<casacore::Int>    numStep;
                casacore::ArrayColumn<casacore::Double>   weightArray;
                casacore::ArrayColumn<casacore::Double>  offsetArray;
                casacore::ArrayColumn<casacore::Double>  freqOffsetArray;

		//CalCurve Table

                casacore::ScalarColumn<casacore::String> calDataId;
                casacore::ScalarColumn<casacore::String> calReductionId;
                casacore::ScalarColumn<casacore::String> antennaName;
                casacore::ScalarColumn<casacore::String> startValidTime;
                casacore::ScalarColumn<casacore::String> endValidTime;
                casacore::ScalarColumn<casacore::String> typeCurve;
                casacore::ArrayColumn<casacore::Double>  curve;
                casacore::ArrayColumn<casacore::Double>  frequencyRange;
                casacore::ScalarColumn<casacore::String> refAntennaName;
                casacore::ScalarColumn<casacore::String> receiverBand;
                casacore::ScalarColumn<casacore::String> timeOrigin;


		//CalData Table

                casacore::ArrayColumn<casacore::Int>     scanSet;
                casacore::ScalarColumn<casacore::String> calType;
                casacore::ScalarColumn<casacore::String> startTimeObserved;
                casacore::ScalarColumn<casacore::String> endTimeObserved;
                casacore::ScalarColumn<casacore::String> calDataType;
                casacore::ScalarColumn<casacore::Int>    frequencyGroup;
                casacore::ArrayColumn<casacore::String>  fieldCode;
                casacore::ArrayColumn<casacore::String>  sourceNameAUX;
                casacore::ArrayColumn<casacore::String>  sourceCode;
                casacore::ScalarColumn<casacore::String> assocCalDataId;
                casacore::ScalarColumn<casacore::String> assocCalNature;

		//CalPhase Table

                casacore::ScalarColumn<casacore::String> basebandName;
                casacore::ScalarColumn<casacore::Int>    numBaseline;
                casacore::ArrayColumn<casacore::String>  antennaNames;
                casacore::ArrayColumn<casacore::Double>  decorrelationFactor;
                casacore::ArrayColumn<casacore::Double>  uncorrPhaseRms;
                casacore::ArrayColumn<casacore::Double>  corrPhaseRms;
                casacore::ArrayColumn<casacore::Double>  statPhaseRms;
                casacore::ArrayColumn<casacore::Bool>    corrValidity;

		//CalReduction Table

                casacore::ScalarColumn<casacore::Int>    numApplied;
                casacore::ScalarColumn<casacore::Int>    numParam;
                casacore::ScalarColumn<casacore::String> timeReduced;
                casacore::ArrayColumn<casacore::String>  calAppliedArray;
                casacore::ArrayColumn<casacore::String>  paramSet;
                casacore::ScalarColumn<casacore::String> messages;
                casacore::ScalarColumn<casacore::String> software;
                casacore::ScalarColumn<casacore::String> softwareVersion;
                casacore::ScalarColumn<casacore::String> invalidConditions;

		//CalSeeingTable

                casacore::ScalarColumn<casacore::Int>    numBaseLength;
                casacore::ArrayColumn<casacore::Double>  baseLength;
                casacore::ScalarColumn<casacore::Double> seeing;
                casacore::ScalarColumn<casacore::Double> seeingFrequency;
                casacore::ScalarColumn<casacore::Double> seeingFreqBandwidth;
                casacore::ScalarColumn<casacore::Double> exponent;

		//CalAtmosphere Table

                casacore::ScalarColumn<casacore::Int>    numFreq;
                casacore::ArrayColumn<casacore::Double>  frequencySpectrum;
                casacore::ScalarColumn<casacore::String> syscalType;
                casacore::ArrayColumn<casacore::Double>  tSysSpectrum;
                casacore::ArrayColumn<casacore::Double>  tRecSpectrum;
                casacore::ArrayColumn<casacore::Double>  tAtmSpectrum;
                casacore::ArrayColumn<casacore::Double>  tauSpectrum;
                casacore::ArrayColumn<casacore::Double>  sbGainSpectrum;
                casacore::ArrayColumn<casacore::Double>  forwardEffSpectrum;
                casacore::ScalarColumn<casacore::Double> groundPressure;
                casacore::ScalarColumn<casacore::Double> groundTemperature;
                casacore::ScalarColumn<casacore::Double> groundRelHumidity;
                casacore::ArrayColumn<casacore::Double>  tSys;
                casacore::ArrayColumn<casacore::Double>  tRec;
                casacore::ArrayColumn<casacore::Double>  tAtm;
                casacore::ArrayColumn<casacore::Double>  sbGain;
                casacore::ArrayColumn<casacore::Double>  water;
                casacore::ArrayColumn<casacore::Double>  forwardEfficiency;
                casacore::ArrayColumn<casacore::Double>  tau;

		//CalAmpli Table

                casacore::ArrayColumn<casacore::Double>  apertureEfficiencyError;
                casacore::ArrayColumn<casacore::Double>  uncorrectedApertureEfficiency;
                casacore::ArrayColumn<casacore::Double>  correctedApertureEfficiency;
                casacore::ArrayColumn<casacore::Bool>    correctionValidity;

                //CalBandpass Table

                casacore::ArrayColumn<casacore::Double>  freqLimits;
                casacore::ScalarColumn<casacore::Int>    numPhasePoly;
                casacore::ScalarColumn<casacore::Int>    numAmpliPoly;
                casacore::ArrayColumn<casacore::Double>  phaseCurve;
                casacore::ArrayColumn<casacore::Double>  ampliCurve;
        
                //AlmaRadiometer Table

                casacore::ScalarColumn<casacore::String> modeId;
                casacore::ScalarColumn<casacore::Int>    numBand;

		//CalDelay Table

                casacore::ArrayColumn<casacore::Double>  delayOffset;
                casacore::ArrayColumn<casacore::Double>  delayError;
                casacore::ScalarColumn<casacore::Double> crossDelayOffset;
                casacore::ScalarColumn<casacore::Double> crossDelayOffsetError;

		//CalDevice Table

                casacore::ScalarColumn<casacore::Int>    numCalload;
                casacore::ArrayColumn<casacore::Double>  noiseCal;
                casacore::ArrayColumn<casacore::Double>  temperatureLoad;
                casacore::ArrayColumn<casacore::Double>  calEff;

                //CalFlux Table

                casacore::ScalarColumn<casacore::Int>    numStokes;
                casacore::ArrayColumn<casacore::Int>     Stokes;
                casacore::ArrayColumn<casacore::Double>  frequencyAUX;
                casacore::ArrayColumn<casacore::Double>  frequencyWidth;
                casacore::ArrayColumn<casacore::Double>  flux;
                casacore::ArrayColumn<casacore::Double>  fluxError;
                casacore::ScalarColumn<casacore::String> fluxMethod;
                casacore::ArrayColumn<casacore::Double>  size;
                casacore::ArrayColumn<casacore::Double>  sizeError;
                casacore::ArrayColumn<casacore::Double>  PA;
                casacore::ArrayColumn<casacore::Double>  PAError;


                //CalFocus Table

                casacore::ArrayColumn<casacore::Double>  offset;
                casacore::ArrayColumn<casacore::Double>  error;
                casacore::ScalarColumn<casacore::String> method;
                casacore::ArrayColumn<casacore::Double>  pointingDirection;
                casacore::ArrayColumn<casacore::Bool>    wasFixed;


                //CalFocusModel Table

                casacore::ScalarColumn<casacore::Int>    numCoeff;
                casacore::ArrayColumn<casacore::Double>  focusRMS;
                casacore::ArrayColumn<casacore::String>  coeffName;
                casacore::ArrayColumn<casacore::String>  coeffFormula;
                casacore::ArrayColumn<casacore::Double>  coeffValue;
                casacore::ArrayColumn<casacore::Double>  coeffError;
                casacore::ArrayColumn<casacore::Bool>    coeffFixed;
                casacore::ScalarColumn<casacore::String> focusModel;
                casacore::ScalarColumn<casacore::Int>    numSourceObs;


                //CalGain Table

                casacore::ArrayColumn<casacore::Double>  gain;
                casacore::ArrayColumn<casacore::Bool>    gainValid;
                casacore::ArrayColumn<casacore::Double>  fit;
                casacore::ArrayColumn<casacore::Double>  fitWeight;
                casacore::ScalarColumn<casacore::Bool>   totalGainValid;
                casacore::ScalarColumn<casacore::Double> totalFit;
                casacore::ScalarColumn<casacore::Double> totalFitWeight;

                //CalHolography Table
 
                casacore::ScalarColumn<casacore::Int>    numScrew;
                casacore::ArrayColumn<casacore::Double>  focusPosition;
                casacore::ScalarColumn<casacore::Double> rawRms;
                casacore::ScalarColumn<casacore::Double> weightedRms;
                casacore::ArrayColumn<casacore::String>  screwName;
                casacore::ArrayColumn<casacore::Double>  screwMotion;
                casacore::ArrayColumn<casacore::Double>  screwMotionError;
                casacore::ScalarColumn<casacore::Int>    panelModes;
                casacore::ScalarColumn<casacore::String> beamMapUID;
                casacore::ScalarColumn<casacore::String> surfaceMapUID;


                //CalPointing Table

                casacore::ArrayColumn<casacore::Double>  collOffset;
                casacore::ArrayColumn<casacore::Double>  collError;
                casacore::ScalarColumn<casacore::String> pointingMethod;
                casacore::ScalarColumn<casacore::String> mode;
                casacore::ArrayColumn<casacore::Double>  beamWidth;
                casacore::ArrayColumn<casacore::Double>  beamWidthError;
                casacore::ScalarColumn<casacore::Double> beamPA;
                casacore::ScalarColumn<casacore::Double> beamPAError;
                casacore::ScalarColumn<casacore::Double> peakIntensity;
                casacore::ScalarColumn<casacore::Double> peakIntensityError;

                //CalPointingModel Table


                casacore::ScalarColumn<casacore::Int>    numObs;
                casacore::ScalarColumn<casacore::Int>    numFormula;
                casacore::ScalarColumn<casacore::Double> azimuthRms;
                casacore::ScalarColumn<casacore::Double> elevationRms;
                casacore::ScalarColumn<casacore::Double> skyRms;
                casacore::ArrayColumn<casacore::Double>  coeffVal;
                casacore::ScalarColumn<casacore::String> pointingModel;

		//CalPosition Table

                casacore::ArrayColumn<casacore::Double>  positionOffset;
                casacore::ArrayColumn<casacore::Double>  positionErr;
                casacore::ScalarColumn<casacore::String> delayRms;
                casacore::ScalarColumn<casacore::Double> phaseRms;
                casacore::ScalarColumn<casacore::Double> axesOffset;
                casacore::ScalarColumn<casacore::Bool>   axesOffsetFixed;
                casacore::ScalarColumn<casacore::Double> axesOffsetErr;
                casacore::ScalarColumn<casacore::String> positionMethod;
                casacore::ArrayColumn<casacore::String>  refAntennaNames;

                //CalPrimaryBeam Table

                casacore::ScalarColumn<casacore::Int>    numPixelX;
                casacore::ScalarColumn<casacore::Int>    numPixelY;
                casacore::ScalarColumn<casacore::Double> refX;
                casacore::ScalarColumn<casacore::Double> refY;
                casacore::ScalarColumn<casacore::Double> valX;
                casacore::ScalarColumn<casacore::Double> valY;
                casacore::ScalarColumn<casacore::Double> incX;
                casacore::ScalarColumn<casacore::Double> incY;
                casacore::ArrayColumn<casacore::Double>  amplitude;
                casacore::ArrayColumn<casacore::Double>  phase;

	        //CalWVR Table
       
                casacore::ScalarColumn<casacore::String> WVRMethod;
                casacore::ArrayColumn<casacore::Double>  pathCoeff;

		//Doppler Table

                casacore::ScalarColumn<casacore::Int>    transitionIndex;
                casacore::ScalarColumn<casacore::Double> velDef;
		
		//Observation Table

                casacore::ScalarColumn<casacore::String> observationId;

                //FlagCmd Table

                casacore::ScalarColumn<casacore::String> reason;
                casacore::ScalarColumn<casacore::Int>    level;
                casacore::ScalarColumn<casacore::Int>    severity;
                casacore::ScalarColumn<casacore::Bool>   applied;
                casacore::ScalarColumn<casacore::String> command;

		//Focus Table

                casacore::ScalarColumn<casacore::String> focusModelId;
                casacore::ScalarColumn<casacore::Double> xFocusPosition;
                casacore::ScalarColumn<casacore::Double> yFocusPosition;
                casacore::ScalarColumn<casacore::Double> zFocusPosition;
                casacore::ScalarColumn<casacore::Double> focusTracking;
                casacore::ScalarColumn<casacore::Double> xFocusOffset;
                casacore::ScalarColumn<casacore::Double> yFocusOffset;
                casacore::ScalarColumn<casacore::Double> zFocusOffset;

		//GainTracking Table

                casacore::ScalarColumn<casacore::Double> attenuator;
                casacore::ScalarColumn<casacore::String> delayoff1;
                casacore::ScalarColumn<casacore::String> delayoff2;
                casacore::ScalarColumn<casacore::Double> phaseoff1;
                casacore::ScalarColumn<casacore::Double> phaseoff2;
                casacore::ScalarColumn<casacore::Double> rateoff1;
                casacore::ScalarColumn<casacore::Double> rateoff2;
                casacore::ScalarColumn<casacore::Double> samplingLevel;
                casacore::ScalarColumn<casacore::Double> phaseRefOffset;

		//History Table

                casacore::ScalarColumn<casacore::String> message;
                casacore::ScalarColumn<casacore::String> priority;
                casacore::ScalarColumn<casacore::String> origin;
                casacore::ScalarColumn<casacore::String> objectId;
                casacore::ScalarColumn<casacore::String> application;
                casacore::ScalarColumn<casacore::String> cliCommand;
                casacore::ScalarColumn<casacore::String> appParms;

		//Holography Table

                casacore::ScalarColumn<casacore::String> holographyId;
                casacore::ScalarColumn<casacore::Double> distance;
                casacore::ScalarColumn<casacore::Double> focus;

		//Pointing Table

                casacore::ScalarColumn<casacore::Int>    pointingModelId;
                casacore::ArrayColumn<casacore::Double>  target;
                casacore::ArrayColumn<casacore::Double>  encoder;
                casacore::ScalarColumn<casacore::Bool>   pointingTracking;
                casacore::ArrayColumn<casacore::Double>  sourceOffset;
                casacore::ScalarColumn<casacore::Bool>   phaseTracking;
                casacore::ScalarColumn<casacore::Bool>   overTheTop;


		//Pointing Table

                casacore::ArrayColumn<casacore::Int>     phaseRmsAUX;

		//SourceParameter Table

                casacore::ScalarColumn<casacore::Int>    numDep;
                casacore::ArrayColumn<casacore::Int>     stokeParameter;
                casacore::ArrayColumn<casacore::Double>  frequencyInterval;
                casacore::ArrayColumn<casacore::Double>  fluxErr;
                casacore::ArrayColumn<casacore::Double>  positionAngle;
                casacore::ArrayColumn<casacore::Double>  sizeErr;
                casacore::ArrayColumn<casacore::Double>  positionAngleErr;
                casacore::ArrayColumn<casacore::Int>     depSourceParameterId;


                //SquareLawDetector Table

                casacore::ScalarColumn<casacore::String> squareLawDetectorId;
                casacore::ScalarColumn<casacore::String> bandType;

		//SysCal Table
                casacore::ScalarColumn<casacore::Int>    numLoad;
                casacore::ArrayColumn<casacore::Int>     calLoad;
                casacore::ArrayColumn<casacore::Double>  feff;
                casacore::ArrayColumn<casacore::Double>  aeff;
                casacore::ScalarColumn<casacore::Double> phaseDiff;
                casacore::ScalarColumn<casacore::Double> sbgain;
                casacore::ArrayColumn<casacore::Double>  tcal;
                casacore::ArrayColumn<casacore::Double>  trx;
                casacore::ArrayColumn<casacore::Double>  tsys;
                casacore::ArrayColumn<casacore::Double>  tsky;
                casacore::ArrayColumn<casacore::Double>  tant;
                casacore::ArrayColumn<casacore::Double>  tantTsys;
                casacore::ArrayColumn<casacore::Double>  pwvPath;
                casacore::ArrayColumn<casacore::Double>  dpwvPath;
                casacore::ArrayColumn<casacore::Double>  feffSpectrum;
                casacore::ArrayColumn<casacore::Double>  tcalSpectrum;
                casacore::ArrayColumn<casacore::Double>  sbgainSpectrum;
                casacore::ArrayColumn<casacore::Double>  trxSpectrum;
                casacore::ArrayColumn<casacore::Double>  tskySpectrum;
                casacore::ArrayColumn<casacore::Double>  tsysSpectrum;
                casacore::ArrayColumn<casacore::Double>  tantSpectrum;
                casacore::ArrayColumn<casacore::Double>  tantTsysSpectrum;
                casacore::ArrayColumn<casacore::Double>  pwvPathSpectrum;
                casacore::ArrayColumn<casacore::Double>  dpwvPathSpectrum;
                casacore::ScalarColumn<casacore::Int>    numPolyFreq;
                casacore::ScalarColumn<casacore::Double> freqOrigin;
                casacore::ArrayColumn<casacore::Double>  delayCurve;
                casacore::ArrayColumn<casacore::Double>  bandpassCurve;
                casacore::ScalarColumn<casacore::Bool>   phasediffFlag;
                casacore::ScalarColumn<casacore::Bool>   sbgainFlag;
                casacore::ScalarColumn<casacore::Bool>   tauFlag;
                casacore::ScalarColumn<casacore::Bool>   tcalFlag;
                casacore::ScalarColumn<casacore::Bool>   trxFlag;
                casacore::ScalarColumn<casacore::Bool>   tskyFlag;
                casacore::ScalarColumn<casacore::Bool>   tsysFlag;
                casacore::ScalarColumn<casacore::Bool>   tantFlag;
                casacore::ScalarColumn<casacore::Bool>   tantTsysFlag;
                casacore::ScalarColumn<casacore::Bool>   pwvPathFlag;

		//TotalPower Table

                casacore::ArrayColumn<casacore::Double>  floatData;

		//WVMCal Table

                casacore::ScalarColumn<casacore::String> calibrationMode;
                casacore::ScalarColumn<casacore::String> operationMode;
                casacore::ScalarColumn<casacore::Double> wvrefModel;

		//Weather Table

                casacore::ScalarColumn<casacore::Double> pressure;
                casacore::ScalarColumn<casacore::Double> relHumidity;
                casacore::ScalarColumn<casacore::Double> temperature;
                casacore::ScalarColumn<casacore::Double> windDirection;
                casacore::ScalarColumn<casacore::Double> windSpeed;
                casacore::ScalarColumn<casacore::Double> windMax;
                casacore::ScalarColumn<casacore::Bool>   pressureFlag;
                casacore::ScalarColumn<casacore::Bool>   relHumidityFlag;
                casacore::ScalarColumn<casacore::Bool>   temperatureFlag;
                casacore::ScalarColumn<casacore::Bool>   windDirectionFlag;
                casacore::ScalarColumn<casacore::Bool>   windSpeedFlag;
                casacore::ScalarColumn<casacore::Bool>   windMaxFlag;
                casacore::ScalarColumn<casacore::Double> dewPoint;
                casacore::ScalarColumn<casacore::Bool>   dewPointFlag;

		//ASDM Table

                casacore::ScalarColumn<casacore::String> Name;
                casacore::ScalarColumn<casacore::Int>    NumberRows;








 

};

#endif

}
