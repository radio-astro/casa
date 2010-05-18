
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * File ASDM.h
 */
 
#ifndef ASDM_CLASS
#define ASDM_CLASS

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <Representable.h>
#include <Entity.h>
#include <EntityId.h>
#include <ArrayTime.h>
#include <IllegalAccessException.h>
#include <InvalidArgumentException.h>

using asdm::Entity;
using asdm::EntityId;
using asdm::ArrayTime;
using asdm::IllegalAccessException;
using asdm::InvalidArgumentException;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h> /// <-------------------
using namespace asdmIDL;   /// <-------------------
#endif

/*\file ASDM.h
    \brief Generated from model's revision "1.54", branch "HEAD"
*/

namespace asdm {


//class asdm::MainTable;
class MainTable;

//class asdm::AlmaRadiometerTable;
class AlmaRadiometerTable;

//class asdm::AnnotationTable;
class AnnotationTable;

//class asdm::AntennaTable;
class AntennaTable;

//class asdm::BeamTable;
class BeamTable;

//class asdm::CalAmpliTable;
class CalAmpliTable;

//class asdm::CalAtmosphereTable;
class CalAtmosphereTable;

//class asdm::CalBandpassTable;
class CalBandpassTable;

//class asdm::CalCurveTable;
class CalCurveTable;

//class asdm::CalDataTable;
class CalDataTable;

//class asdm::CalDelayTable;
class CalDelayTable;

//class asdm::CalDeviceTable;
class CalDeviceTable;

//class asdm::CalFluxTable;
class CalFluxTable;

//class asdm::CalFocusTable;
class CalFocusTable;

//class asdm::CalFocusModelTable;
class CalFocusModelTable;

//class asdm::CalGainTable;
class CalGainTable;

//class asdm::CalHolographyTable;
class CalHolographyTable;

//class asdm::CalPhaseTable;
class CalPhaseTable;

//class asdm::CalPointingTable;
class CalPointingTable;

//class asdm::CalPointingModelTable;
class CalPointingModelTable;

//class asdm::CalPositionTable;
class CalPositionTable;

//class asdm::CalPrimaryBeamTable;
class CalPrimaryBeamTable;

//class asdm::CalReductionTable;
class CalReductionTable;

//class asdm::CalSeeingTable;
class CalSeeingTable;

//class asdm::CalWVRTable;
class CalWVRTable;

//class asdm::ConfigDescriptionTable;
class ConfigDescriptionTable;

//class asdm::CorrelatorModeTable;
class CorrelatorModeTable;

//class asdm::DataDescriptionTable;
class DataDescriptionTable;

//class asdm::DelayModelTable;
class DelayModelTable;

//class asdm::DopplerTable;
class DopplerTable;

//class asdm::EphemerisTable;
class EphemerisTable;

//class asdm::ExecBlockTable;
class ExecBlockTable;

//class asdm::FeedTable;
class FeedTable;

//class asdm::FieldTable;
class FieldTable;

//class asdm::FlagCmdTable;
class FlagCmdTable;

//class asdm::FocusTable;
class FocusTable;

//class asdm::FocusModelTable;
class FocusModelTable;

//class asdm::FreqOffsetTable;
class FreqOffsetTable;

//class asdm::GainTrackingTable;
class GainTrackingTable;

//class asdm::HistoryTable;
class HistoryTable;

//class asdm::HolographyTable;
class HolographyTable;

//class asdm::ObservationTable;
class ObservationTable;

//class asdm::PointingTable;
class PointingTable;

//class asdm::PointingModelTable;
class PointingModelTable;

//class asdm::PolarizationTable;
class PolarizationTable;

//class asdm::ProcessorTable;
class ProcessorTable;

//class asdm::ReceiverTable;
class ReceiverTable;

//class asdm::SBSummaryTable;
class SBSummaryTable;

//class asdm::ScanTable;
class ScanTable;

//class asdm::SeeingTable;
class SeeingTable;

//class asdm::SourceTable;
class SourceTable;

//class asdm::SpectralWindowTable;
class SpectralWindowTable;

//class asdm::SquareLawDetectorTable;
class SquareLawDetectorTable;

//class asdm::StateTable;
class StateTable;

//class asdm::StationTable;
class StationTable;

//class asdm::SubscanTable;
class SubscanTable;

//class asdm::SwitchCycleTable;
class SwitchCycleTable;

//class asdm::SysCalTable;
class SysCalTable;

//class asdm::TotalPowerTable;
class TotalPowerTable;

//class asdm::WVMCalTable;
class WVMCalTable;

//class asdm::WeatherTable;
class WeatherTable;


/**
 * The ASDM class is the container for all tables.  Its instantation
 * creates a complete set of tables.
 *
 * 
 * Generated from model's revision "1.54", branch "HEAD"
 */
//class ASDM : public Representable {
class ASDM {

public:
	/**
	 * Create an instance of the tables that belong to this model.
	 */
	ASDM ();
	
	virtual ~ASDM ();
	

	/**
	 * Get the table Main.
	 * @return The table Main as a MainTable.
	 */
	MainTable & getMain () const;

	/**
	 * Get the table AlmaRadiometer.
	 * @return The table AlmaRadiometer as a AlmaRadiometerTable.
	 */
	AlmaRadiometerTable & getAlmaRadiometer () const;

	/**
	 * Get the table Annotation.
	 * @return The table Annotation as a AnnotationTable.
	 */
	AnnotationTable & getAnnotation () const;

	/**
	 * Get the table Antenna.
	 * @return The table Antenna as a AntennaTable.
	 */
	AntennaTable & getAntenna () const;

	/**
	 * Get the table Beam.
	 * @return The table Beam as a BeamTable.
	 */
	BeamTable & getBeam () const;

	/**
	 * Get the table CalAmpli.
	 * @return The table CalAmpli as a CalAmpliTable.
	 */
	CalAmpliTable & getCalAmpli () const;

	/**
	 * Get the table CalAtmosphere.
	 * @return The table CalAtmosphere as a CalAtmosphereTable.
	 */
	CalAtmosphereTable & getCalAtmosphere () const;

	/**
	 * Get the table CalBandpass.
	 * @return The table CalBandpass as a CalBandpassTable.
	 */
	CalBandpassTable & getCalBandpass () const;

	/**
	 * Get the table CalCurve.
	 * @return The table CalCurve as a CalCurveTable.
	 */
	CalCurveTable & getCalCurve () const;

	/**
	 * Get the table CalData.
	 * @return The table CalData as a CalDataTable.
	 */
	CalDataTable & getCalData () const;

	/**
	 * Get the table CalDelay.
	 * @return The table CalDelay as a CalDelayTable.
	 */
	CalDelayTable & getCalDelay () const;

	/**
	 * Get the table CalDevice.
	 * @return The table CalDevice as a CalDeviceTable.
	 */
	CalDeviceTable & getCalDevice () const;

	/**
	 * Get the table CalFlux.
	 * @return The table CalFlux as a CalFluxTable.
	 */
	CalFluxTable & getCalFlux () const;

	/**
	 * Get the table CalFocus.
	 * @return The table CalFocus as a CalFocusTable.
	 */
	CalFocusTable & getCalFocus () const;

	/**
	 * Get the table CalFocusModel.
	 * @return The table CalFocusModel as a CalFocusModelTable.
	 */
	CalFocusModelTable & getCalFocusModel () const;

	/**
	 * Get the table CalGain.
	 * @return The table CalGain as a CalGainTable.
	 */
	CalGainTable & getCalGain () const;

	/**
	 * Get the table CalHolography.
	 * @return The table CalHolography as a CalHolographyTable.
	 */
	CalHolographyTable & getCalHolography () const;

	/**
	 * Get the table CalPhase.
	 * @return The table CalPhase as a CalPhaseTable.
	 */
	CalPhaseTable & getCalPhase () const;

	/**
	 * Get the table CalPointing.
	 * @return The table CalPointing as a CalPointingTable.
	 */
	CalPointingTable & getCalPointing () const;

	/**
	 * Get the table CalPointingModel.
	 * @return The table CalPointingModel as a CalPointingModelTable.
	 */
	CalPointingModelTable & getCalPointingModel () const;

	/**
	 * Get the table CalPosition.
	 * @return The table CalPosition as a CalPositionTable.
	 */
	CalPositionTable & getCalPosition () const;

	/**
	 * Get the table CalPrimaryBeam.
	 * @return The table CalPrimaryBeam as a CalPrimaryBeamTable.
	 */
	CalPrimaryBeamTable & getCalPrimaryBeam () const;

	/**
	 * Get the table CalReduction.
	 * @return The table CalReduction as a CalReductionTable.
	 */
	CalReductionTable & getCalReduction () const;

	/**
	 * Get the table CalSeeing.
	 * @return The table CalSeeing as a CalSeeingTable.
	 */
	CalSeeingTable & getCalSeeing () const;

	/**
	 * Get the table CalWVR.
	 * @return The table CalWVR as a CalWVRTable.
	 */
	CalWVRTable & getCalWVR () const;

	/**
	 * Get the table ConfigDescription.
	 * @return The table ConfigDescription as a ConfigDescriptionTable.
	 */
	ConfigDescriptionTable & getConfigDescription () const;

	/**
	 * Get the table CorrelatorMode.
	 * @return The table CorrelatorMode as a CorrelatorModeTable.
	 */
	CorrelatorModeTable & getCorrelatorMode () const;

	/**
	 * Get the table DataDescription.
	 * @return The table DataDescription as a DataDescriptionTable.
	 */
	DataDescriptionTable & getDataDescription () const;

	/**
	 * Get the table DelayModel.
	 * @return The table DelayModel as a DelayModelTable.
	 */
	DelayModelTable & getDelayModel () const;

	/**
	 * Get the table Doppler.
	 * @return The table Doppler as a DopplerTable.
	 */
	DopplerTable & getDoppler () const;

	/**
	 * Get the table Ephemeris.
	 * @return The table Ephemeris as a EphemerisTable.
	 */
	EphemerisTable & getEphemeris () const;

	/**
	 * Get the table ExecBlock.
	 * @return The table ExecBlock as a ExecBlockTable.
	 */
	ExecBlockTable & getExecBlock () const;

	/**
	 * Get the table Feed.
	 * @return The table Feed as a FeedTable.
	 */
	FeedTable & getFeed () const;

	/**
	 * Get the table Field.
	 * @return The table Field as a FieldTable.
	 */
	FieldTable & getField () const;

	/**
	 * Get the table FlagCmd.
	 * @return The table FlagCmd as a FlagCmdTable.
	 */
	FlagCmdTable & getFlagCmd () const;

	/**
	 * Get the table Focus.
	 * @return The table Focus as a FocusTable.
	 */
	FocusTable & getFocus () const;

	/**
	 * Get the table FocusModel.
	 * @return The table FocusModel as a FocusModelTable.
	 */
	FocusModelTable & getFocusModel () const;

	/**
	 * Get the table FreqOffset.
	 * @return The table FreqOffset as a FreqOffsetTable.
	 */
	FreqOffsetTable & getFreqOffset () const;

	/**
	 * Get the table GainTracking.
	 * @return The table GainTracking as a GainTrackingTable.
	 */
	GainTrackingTable & getGainTracking () const;

	/**
	 * Get the table History.
	 * @return The table History as a HistoryTable.
	 */
	HistoryTable & getHistory () const;

	/**
	 * Get the table Holography.
	 * @return The table Holography as a HolographyTable.
	 */
	HolographyTable & getHolography () const;

	/**
	 * Get the table Observation.
	 * @return The table Observation as a ObservationTable.
	 */
	ObservationTable & getObservation () const;

	/**
	 * Get the table Pointing.
	 * @return The table Pointing as a PointingTable.
	 */
	PointingTable & getPointing () const;

	/**
	 * Get the table PointingModel.
	 * @return The table PointingModel as a PointingModelTable.
	 */
	PointingModelTable & getPointingModel () const;

	/**
	 * Get the table Polarization.
	 * @return The table Polarization as a PolarizationTable.
	 */
	PolarizationTable & getPolarization () const;

	/**
	 * Get the table Processor.
	 * @return The table Processor as a ProcessorTable.
	 */
	ProcessorTable & getProcessor () const;

	/**
	 * Get the table Receiver.
	 * @return The table Receiver as a ReceiverTable.
	 */
	ReceiverTable & getReceiver () const;

	/**
	 * Get the table SBSummary.
	 * @return The table SBSummary as a SBSummaryTable.
	 */
	SBSummaryTable & getSBSummary () const;

	/**
	 * Get the table Scan.
	 * @return The table Scan as a ScanTable.
	 */
	ScanTable & getScan () const;

	/**
	 * Get the table Seeing.
	 * @return The table Seeing as a SeeingTable.
	 */
	SeeingTable & getSeeing () const;

	/**
	 * Get the table Source.
	 * @return The table Source as a SourceTable.
	 */
	SourceTable & getSource () const;

	/**
	 * Get the table SpectralWindow.
	 * @return The table SpectralWindow as a SpectralWindowTable.
	 */
	SpectralWindowTable & getSpectralWindow () const;

	/**
	 * Get the table SquareLawDetector.
	 * @return The table SquareLawDetector as a SquareLawDetectorTable.
	 */
	SquareLawDetectorTable & getSquareLawDetector () const;

	/**
	 * Get the table State.
	 * @return The table State as a StateTable.
	 */
	StateTable & getState () const;

	/**
	 * Get the table Station.
	 * @return The table Station as a StationTable.
	 */
	StationTable & getStation () const;

	/**
	 * Get the table Subscan.
	 * @return The table Subscan as a SubscanTable.
	 */
	SubscanTable & getSubscan () const;

	/**
	 * Get the table SwitchCycle.
	 * @return The table SwitchCycle as a SwitchCycleTable.
	 */
	SwitchCycleTable & getSwitchCycle () const;

	/**
	 * Get the table SysCal.
	 * @return The table SysCal as a SysCalTable.
	 */
	SysCalTable & getSysCal () const;

	/**
	 * Get the table TotalPower.
	 * @return The table TotalPower as a TotalPowerTable.
	 */
	TotalPowerTable & getTotalPower () const;

	/**
	 * Get the table WVMCal.
	 * @return The table WVMCal as a WVMCalTable.
	 */
	WVMCalTable & getWVMCal () const;

	/**
	 * Get the table Weather.
	 * @return The table Weather as a WeatherTable.
	 */
	WeatherTable & getWeather () const;

	/**
	  * Produces the XML representation of * this.
	  * @return a string containing the XML representation of this.
	  * @throws ConversionException.
	  */
	virtual string toXML();
	
	/**
	 * Write this ASDM dataset to the specified directory
	 * as a collection of XML documents. 
	 * @param directory The directory to which this dataset is written.
	 * @throws ConversionException If any error occurs in converting the
	 * table to XML and writing it to the directory.  This method will
	 * not overwrite any existing file; a ConversionException is also
	 * thrown in this case.
	 */
	void toXML(string directory) ;
	
	/**
	 * Get an ASDM dataset, given the full path name of the 
	 * directory containing the XML version of the dataset.
	 * @param xmlDirectory The full path name of the directory
	 * containing this dataset.
	 * @return The complete dataset that belongs to the container
	 * in this directory.
	 * @throws ConversionException If any error occurs reading the 
	 * files in the directory or in converting the tables from XML.
	 */
	virtual void fromXML(string xml) ;
		
	/**
	 * Get an ASDM dataset, given the full path name of the 
	 * directory containing the XML version of the dataset.
	 * @param xmlDirectory The full path name of the directory
	 * containing this dataset.
	 * @return The complete dataset that belongs to the container
	 * in this directory.
	 * @throws ConversionException If any error occurs reading the 
	 * files in the directory or in converting the tables from XML.
	 *
	 * @deprecated
	 */
	static ASDM *getFromXML(string xmlDirectory) ;
	
   /**
	 * Serialize this into a stream of bytes and encapsulates that stream into a MIME message.
	 * @returns a string containing the MIME message.
	 * 
	 */
	string toMIME();
	
   /** 
     * Extracts the binary part of a MIME message and deserialize its content
	 * to fill this with the result of the deserialization. 
	 * @param mimeMsg the string containing the MIME message.
	 * @throws ConversionException
	 */
	 void setFromMIME(const string & mimeMsg);	

	/**
	 * Write this ASDM dataset to the specified directory
	 * as a collection of files.
	 *
	 * The container itself is written into an XML file. Each table of the container
	 * having at least one row is written into a binary or an XML file depending on
	 * the value of its "fileAsBin" private field.
	 * 
	 * @param directory The directory to which this dataset is written.
	 * @throws ConversionException If any error occurs in converting the
	 * container or any of its table.  This method will
	 * not overwrite any existing file; a ConversionException is also
	 * thrown in this case.
	 */
	void toFile(string directory);

	/**
	 * Reads and parses a collection of files as those produced by the toFile method.
	 * This dataset is populated with the result of the parsing.
	 * @param directory The name of the directory containing the files.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */	
	 void setFromFile(string directory);
	
	#ifndef WITHOUT_ACS
	/**
	  * Converts this ASDM into an ASDMDataSetIDL CORBA structure
	  * @return a pointer to a ASDMDataSetIDL.
	  */
	virtual ASDMDataSetIDL* toIDL();  
	
	 /**
	   * Builds an ASDM out of its IDL representation.
	   * @param x the IDL representation of the ASDM
	   *
	   * @throws DuplicateKey 
	   * @throws ConversionException
	   * @throws UniquenessViolationException
	   */
	virtual void fromIDL(ASDMDataSetIDL* x); 
	#endif
		
	virtual Entity getEntity() const;

	virtual void setEntity(Entity e);
	
	/**
	 * Meaningless, but required for the Representable interface.
	 */
	virtual string getName() const;
	
	/**
	 * Meaningless, but required for the Representable interface.
	 */
	virtual unsigned int size() ;
	
#ifndef WITHOUT_ACS
	/**
	 * Create an ASDM dataset from the ALMA archive, given the
	 * entityId of its container.
	 * @param datasetId The entityId of the container of the dataset.
	 * @throws ConversionException
	 */
	static ASDM *fromArchive(EntityId datasetId) ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Update an ASDM dataset that already exists in the ALMA archive.
	 * @throws ConversionException
	 */
	void updateArchive() const ;
#endif
	
	/**
	 * Return the table, as a Representable object, with the
	 * specified name.
	 * @throws InvalidArgumentException
	 */
	Representable &getTable(string tableName) ;

	
	
	// ===> Attribute timeOfCreation
	
	
	

	
 	/**
 	 * Get timeOfCreation.
 	 * @return timeOfCreation as ArrayTime
 	 */
 	ArrayTime getTimeOfCreation() const;
	
 
 	
 	
 	/**
 	 * Set timeOfCreation with the specified ArrayTime.
 	 * @param timeOfCreation The ArrayTime value to which timeOfCreation is to be set.
 	 
 		
 			
 	 */
 	void setTimeOfCreation (ArrayTime timeOfCreation);
  		
	
	
	


	
	// ===> Attribute version
	
	
	

	
 	/**
 	 * Get version.
 	 * @return version as int
 	 */
 	int getVersion() const;
	
 
 	
 	
 	/**
 	 * Set version with the specified int.
 	 * @param version The int value to which version is to be set.
 	 
 		
 			
 	 */
 	void setVersion (int version);
  		
	
	
	


	
	// ===> Attribute xmlnsPrefix
	
	
	

	
 	/**
 	 * Get xmlnsPrefix.
 	 * @return xmlnsPrefix as string
 	 */
 	string getXmlnsPrefix() const;
	
 
 	
 	
 	/**
 	 * Set xmlnsPrefix with the specified string.
 	 * @param xmlnsPrefix The string value to which xmlnsPrefix is to be set.
 	 
 		
 			
 	 */
 	void setXmlnsPrefix (string xmlnsPrefix);
  		
	
	
	



private:

	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML		
	bool hasBeenAdded;
	

	/**
	 * The table Main
	 */
	MainTable * main;

	/**
	 * The table AlmaRadiometer
	 */
	AlmaRadiometerTable * almaRadiometer;

	/**
	 * The table Annotation
	 */
	AnnotationTable * annotation;

	/**
	 * The table Antenna
	 */
	AntennaTable * antenna;

	/**
	 * The table Beam
	 */
	BeamTable * beam;

	/**
	 * The table CalAmpli
	 */
	CalAmpliTable * calAmpli;

	/**
	 * The table CalAtmosphere
	 */
	CalAtmosphereTable * calAtmosphere;

	/**
	 * The table CalBandpass
	 */
	CalBandpassTable * calBandpass;

	/**
	 * The table CalCurve
	 */
	CalCurveTable * calCurve;

	/**
	 * The table CalData
	 */
	CalDataTable * calData;

	/**
	 * The table CalDelay
	 */
	CalDelayTable * calDelay;

	/**
	 * The table CalDevice
	 */
	CalDeviceTable * calDevice;

	/**
	 * The table CalFlux
	 */
	CalFluxTable * calFlux;

	/**
	 * The table CalFocus
	 */
	CalFocusTable * calFocus;

	/**
	 * The table CalFocusModel
	 */
	CalFocusModelTable * calFocusModel;

	/**
	 * The table CalGain
	 */
	CalGainTable * calGain;

	/**
	 * The table CalHolography
	 */
	CalHolographyTable * calHolography;

	/**
	 * The table CalPhase
	 */
	CalPhaseTable * calPhase;

	/**
	 * The table CalPointing
	 */
	CalPointingTable * calPointing;

	/**
	 * The table CalPointingModel
	 */
	CalPointingModelTable * calPointingModel;

	/**
	 * The table CalPosition
	 */
	CalPositionTable * calPosition;

	/**
	 * The table CalPrimaryBeam
	 */
	CalPrimaryBeamTable * calPrimaryBeam;

	/**
	 * The table CalReduction
	 */
	CalReductionTable * calReduction;

	/**
	 * The table CalSeeing
	 */
	CalSeeingTable * calSeeing;

	/**
	 * The table CalWVR
	 */
	CalWVRTable * calWVR;

	/**
	 * The table ConfigDescription
	 */
	ConfigDescriptionTable * configDescription;

	/**
	 * The table CorrelatorMode
	 */
	CorrelatorModeTable * correlatorMode;

	/**
	 * The table DataDescription
	 */
	DataDescriptionTable * dataDescription;

	/**
	 * The table DelayModel
	 */
	DelayModelTable * delayModel;

	/**
	 * The table Doppler
	 */
	DopplerTable * doppler;

	/**
	 * The table Ephemeris
	 */
	EphemerisTable * ephemeris;

	/**
	 * The table ExecBlock
	 */
	ExecBlockTable * execBlock;

	/**
	 * The table Feed
	 */
	FeedTable * feed;

	/**
	 * The table Field
	 */
	FieldTable * field;

	/**
	 * The table FlagCmd
	 */
	FlagCmdTable * flagCmd;

	/**
	 * The table Focus
	 */
	FocusTable * focus;

	/**
	 * The table FocusModel
	 */
	FocusModelTable * focusModel;

	/**
	 * The table FreqOffset
	 */
	FreqOffsetTable * freqOffset;

	/**
	 * The table GainTracking
	 */
	GainTrackingTable * gainTracking;

	/**
	 * The table History
	 */
	HistoryTable * history;

	/**
	 * The table Holography
	 */
	HolographyTable * holography;

	/**
	 * The table Observation
	 */
	ObservationTable * observation;

	/**
	 * The table Pointing
	 */
	PointingTable * pointing;

	/**
	 * The table PointingModel
	 */
	PointingModelTable * pointingModel;

	/**
	 * The table Polarization
	 */
	PolarizationTable * polarization;

	/**
	 * The table Processor
	 */
	ProcessorTable * processor;

	/**
	 * The table Receiver
	 */
	ReceiverTable * receiver;

	/**
	 * The table SBSummary
	 */
	SBSummaryTable * sBSummary;

	/**
	 * The table Scan
	 */
	ScanTable * scan;

	/**
	 * The table Seeing
	 */
	SeeingTable * seeing;

	/**
	 * The table Source
	 */
	SourceTable * source;

	/**
	 * The table SpectralWindow
	 */
	SpectralWindowTable * spectralWindow;

	/**
	 * The table SquareLawDetector
	 */
	SquareLawDetectorTable * squareLawDetector;

	/**
	 * The table State
	 */
	StateTable * state;

	/**
	 * The table Station
	 */
	StationTable * station;

	/**
	 * The table Subscan
	 */
	SubscanTable * subscan;

	/**
	 * The table SwitchCycle
	 */
	SwitchCycleTable * switchCycle;

	/**
	 * The table SysCal
	 */
	SysCalTable * sysCal;

	/**
	 * The table TotalPower
	 */
	TotalPowerTable * totalPower;

	/**
	 * The table WVMCal
	 */
	WVMCalTable * wVMCal;

	/**
	 * The table Weather
	 */
	WeatherTable * weather;

	/**
	 * The list of tables as Representable.
	 */
	vector<Representable *> table;
	
	/**
	 * The list of Entity objects representing the tables.
	 */
	//vector<Entity *> tableEntity;
	map<string, Entity> tableEntity;
	
	/**
	 * This Container's entity.
	 */
	Entity entity;

	
	
	// ===> Attribute timeOfCreation
	
	

	ArrayTime timeOfCreation;

	
	
 	

	
	// ===> Attribute version
	
	

	int version;

	
	
 	

	
	// ===> Attribute xmlnsPrefix
	
	

	string xmlnsPrefix;

	
	
 	

	
	void error() ; // throw(ConversionException);
	static string getXMLEntity(EntityId id); // throw(ConversionException);
	static void putXMLEntity(string xml); // throw(ConversionException);
	
};

} // End namespace asdm

#endif /* ASDM_CLASS */
