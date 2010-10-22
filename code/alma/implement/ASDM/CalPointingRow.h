
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
 * File CalPointingRow.h
 */
 
#ifndef CalPointingRow_CLASS
#define CalPointingRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalPointingRowIDL;
#endif






#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Temperature.h>
using  asdm::Temperature;

#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;




	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	

	

	
#include "CAntennaMake.h"
using namespace AntennaMakeMod;
	

	
#include "CAtmPhaseCorrection.h"
using namespace AtmPhaseCorrectionMod;
	

	

	

	
#include "CPointingModelMode.h"
using namespace PointingModelModeMod;
	

	
#include "CPointingMethod.h"
using namespace PointingMethodMod;
	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file CalPointing.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::CalPointingTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

class CalPointingRow;
typedef void (CalPointingRow::*CalPointingAttributeFromBin) (EndianISStream& eiss);

/**
 * The CalPointingRow class is a row of a CalPointingTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class CalPointingRow {
friend class asdm::CalPointingTable;

public:

	virtual ~CalPointingRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalPointingTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string getAntennaName() const;
	
 
 	
 	
 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setAntennaName (string antennaName);
  		
	
	
	


	
	// ===> Attribute receiverBand
	
	
	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand getReceiverBand() const;
	
 
 	
 	
 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand);
  		
	
	
	


	
	// ===> Attribute startValidTime
	
	
	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime getStartValidTime() const;
	
 
 	
 	
 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 		
 			
 	 */
 	void setStartValidTime (ArrayTime startValidTime);
  		
	
	
	


	
	// ===> Attribute endValidTime
	
	
	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime getEndValidTime() const;
	
 
 	
 	
 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 		
 			
 	 */
 	void setEndValidTime (ArrayTime endValidTime);
  		
	
	
	


	
	// ===> Attribute ambientTemperature
	
	
	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature getAmbientTemperature() const;
	
 
 	
 	
 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 		
 			
 	 */
 	void setAmbientTemperature (Temperature ambientTemperature);
  		
	
	
	


	
	// ===> Attribute antennaMake
	
	
	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake getAntennaMake() const;
	
 
 	
 	
 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 		
 			
 	 */
 	void setAntennaMake (AntennaMakeMod::AntennaMake antennaMake);
  		
	
	
	


	
	// ===> Attribute atmPhaseCorrection
	
	
	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection getAtmPhaseCorrection() const;
	
 
 	
 	
 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 		
 			
 	 */
 	void setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection);
  		
	
	
	


	
	// ===> Attribute direction
	
	
	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > getDirection() const;
	
 
 	
 	
 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 		
 			
 	 */
 	void setDirection (vector<Angle > direction);
  		
	
	
	


	
	// ===> Attribute frequencyRange
	
	
	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > getFrequencyRange() const;
	
 
 	
 	
 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyRange (vector<Frequency > frequencyRange);
  		
	
	
	


	
	// ===> Attribute pointingModelMode
	
	
	

	
 	/**
 	 * Get pointingModelMode.
 	 * @return pointingModelMode as PointingModelModeMod::PointingModelMode
 	 */
 	PointingModelModeMod::PointingModelMode getPointingModelMode() const;
	
 
 	
 	
 	/**
 	 * Set pointingModelMode with the specified PointingModelModeMod::PointingModelMode.
 	 * @param pointingModelMode The PointingModelModeMod::PointingModelMode value to which pointingModelMode is to be set.
 	 
 		
 			
 	 */
 	void setPointingModelMode (PointingModelModeMod::PointingModelMode pointingModelMode);
  		
	
	
	


	
	// ===> Attribute pointingMethod
	
	
	

	
 	/**
 	 * Get pointingMethod.
 	 * @return pointingMethod as PointingMethodMod::PointingMethod
 	 */
 	PointingMethodMod::PointingMethod getPointingMethod() const;
	
 
 	
 	
 	/**
 	 * Set pointingMethod with the specified PointingMethodMod::PointingMethod.
 	 * @param pointingMethod The PointingMethodMod::PointingMethod value to which pointingMethod is to be set.
 	 
 		
 			
 	 */
 	void setPointingMethod (PointingMethodMod::PointingMethod pointingMethod);
  		
	
	
	


	
	// ===> Attribute numReceptor
	
	
	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int getNumReceptor() const;
	
 
 	
 	
 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 		
 			
 	 */
 	void setNumReceptor (int numReceptor);
  		
	
	
	


	
	// ===> Attribute polarizationTypes
	
	
	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > getPolarizationTypes() const;
	
 
 	
 	
 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 		
 			
 	 */
 	void setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes);
  		
	
	
	


	
	// ===> Attribute collOffsetRelative
	
	
	

	
 	/**
 	 * Get collOffsetRelative.
 	 * @return collOffsetRelative as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getCollOffsetRelative() const;
	
 
 	
 	
 	/**
 	 * Set collOffsetRelative with the specified vector<vector<Angle > >.
 	 * @param collOffsetRelative The vector<vector<Angle > > value to which collOffsetRelative is to be set.
 	 
 		
 			
 	 */
 	void setCollOffsetRelative (vector<vector<Angle > > collOffsetRelative);
  		
	
	
	


	
	// ===> Attribute collOffsetAbsolute
	
	
	

	
 	/**
 	 * Get collOffsetAbsolute.
 	 * @return collOffsetAbsolute as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getCollOffsetAbsolute() const;
	
 
 	
 	
 	/**
 	 * Set collOffsetAbsolute with the specified vector<vector<Angle > >.
 	 * @param collOffsetAbsolute The vector<vector<Angle > > value to which collOffsetAbsolute is to be set.
 	 
 		
 			
 	 */
 	void setCollOffsetAbsolute (vector<vector<Angle > > collOffsetAbsolute);
  		
	
	
	


	
	// ===> Attribute collError
	
	
	

	
 	/**
 	 * Get collError.
 	 * @return collError as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getCollError() const;
	
 
 	
 	
 	/**
 	 * Set collError with the specified vector<vector<Angle > >.
 	 * @param collError The vector<vector<Angle > > value to which collError is to be set.
 	 
 		
 			
 	 */
 	void setCollError (vector<vector<Angle > > collError);
  		
	
	
	


	
	// ===> Attribute collOffsetTied
	
	
	

	
 	/**
 	 * Get collOffsetTied.
 	 * @return collOffsetTied as vector<vector<bool > >
 	 */
 	vector<vector<bool > > getCollOffsetTied() const;
	
 
 	
 	
 	/**
 	 * Set collOffsetTied with the specified vector<vector<bool > >.
 	 * @param collOffsetTied The vector<vector<bool > > value to which collOffsetTied is to be set.
 	 
 		
 			
 	 */
 	void setCollOffsetTied (vector<vector<bool > > collOffsetTied);
  		
	
	
	


	
	// ===> Attribute reducedChiSquared
	
	
	

	
 	/**
 	 * Get reducedChiSquared.
 	 * @return reducedChiSquared as vector<double >
 	 */
 	vector<double > getReducedChiSquared() const;
	
 
 	
 	
 	/**
 	 * Set reducedChiSquared with the specified vector<double >.
 	 * @param reducedChiSquared The vector<double > value to which reducedChiSquared is to be set.
 	 
 		
 			
 	 */
 	void setReducedChiSquared (vector<double > reducedChiSquared);
  		
	
	
	


	
	// ===> Attribute averagedPolarizations, which is optional
	
	
	
	/**
	 * The attribute averagedPolarizations is optional. Return true if this attribute exists.
	 * @return true if and only if the averagedPolarizations attribute exists. 
	 */
	bool isAveragedPolarizationsExists() const;
	

	
 	/**
 	 * Get averagedPolarizations, which is optional.
 	 * @return averagedPolarizations as bool
 	 * @throws IllegalAccessException If averagedPolarizations does not exist.
 	 */
 	bool getAveragedPolarizations() const;
	
 
 	
 	
 	/**
 	 * Set averagedPolarizations with the specified bool.
 	 * @param averagedPolarizations The bool value to which averagedPolarizations is to be set.
 	 
 		
 	 */
 	void setAveragedPolarizations (bool averagedPolarizations);
		
	
	
	
	/**
	 * Mark averagedPolarizations, which is an optional field, as non-existent.
	 */
	void clearAveragedPolarizations ();
	


	
	// ===> Attribute beamPA, which is optional
	
	
	
	/**
	 * The attribute beamPA is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPA attribute exists. 
	 */
	bool isBeamPAExists() const;
	

	
 	/**
 	 * Get beamPA, which is optional.
 	 * @return beamPA as vector<Angle >
 	 * @throws IllegalAccessException If beamPA does not exist.
 	 */
 	vector<Angle > getBeamPA() const;
	
 
 	
 	
 	/**
 	 * Set beamPA with the specified vector<Angle >.
 	 * @param beamPA The vector<Angle > value to which beamPA is to be set.
 	 
 		
 	 */
 	void setBeamPA (vector<Angle > beamPA);
		
	
	
	
	/**
	 * Mark beamPA, which is an optional field, as non-existent.
	 */
	void clearBeamPA ();
	


	
	// ===> Attribute beamPAError, which is optional
	
	
	
	/**
	 * The attribute beamPAError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAError attribute exists. 
	 */
	bool isBeamPAErrorExists() const;
	

	
 	/**
 	 * Get beamPAError, which is optional.
 	 * @return beamPAError as vector<Angle >
 	 * @throws IllegalAccessException If beamPAError does not exist.
 	 */
 	vector<Angle > getBeamPAError() const;
	
 
 	
 	
 	/**
 	 * Set beamPAError with the specified vector<Angle >.
 	 * @param beamPAError The vector<Angle > value to which beamPAError is to be set.
 	 
 		
 	 */
 	void setBeamPAError (vector<Angle > beamPAError);
		
	
	
	
	/**
	 * Mark beamPAError, which is an optional field, as non-existent.
	 */
	void clearBeamPAError ();
	


	
	// ===> Attribute beamPAWasFixed, which is optional
	
	
	
	/**
	 * The attribute beamPAWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAWasFixed attribute exists. 
	 */
	bool isBeamPAWasFixedExists() const;
	

	
 	/**
 	 * Get beamPAWasFixed, which is optional.
 	 * @return beamPAWasFixed as bool
 	 * @throws IllegalAccessException If beamPAWasFixed does not exist.
 	 */
 	bool getBeamPAWasFixed() const;
	
 
 	
 	
 	/**
 	 * Set beamPAWasFixed with the specified bool.
 	 * @param beamPAWasFixed The bool value to which beamPAWasFixed is to be set.
 	 
 		
 	 */
 	void setBeamPAWasFixed (bool beamPAWasFixed);
		
	
	
	
	/**
	 * Mark beamPAWasFixed, which is an optional field, as non-existent.
	 */
	void clearBeamPAWasFixed ();
	


	
	// ===> Attribute beamWidth, which is optional
	
	
	
	/**
	 * The attribute beamWidth is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidth attribute exists. 
	 */
	bool isBeamWidthExists() const;
	

	
 	/**
 	 * Get beamWidth, which is optional.
 	 * @return beamWidth as vector<vector<Angle > >
 	 * @throws IllegalAccessException If beamWidth does not exist.
 	 */
 	vector<vector<Angle > > getBeamWidth() const;
	
 
 	
 	
 	/**
 	 * Set beamWidth with the specified vector<vector<Angle > >.
 	 * @param beamWidth The vector<vector<Angle > > value to which beamWidth is to be set.
 	 
 		
 	 */
 	void setBeamWidth (vector<vector<Angle > > beamWidth);
		
	
	
	
	/**
	 * Mark beamWidth, which is an optional field, as non-existent.
	 */
	void clearBeamWidth ();
	


	
	// ===> Attribute beamWidthError, which is optional
	
	
	
	/**
	 * The attribute beamWidthError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthError attribute exists. 
	 */
	bool isBeamWidthErrorExists() const;
	

	
 	/**
 	 * Get beamWidthError, which is optional.
 	 * @return beamWidthError as vector<vector<Angle > >
 	 * @throws IllegalAccessException If beamWidthError does not exist.
 	 */
 	vector<vector<Angle > > getBeamWidthError() const;
	
 
 	
 	
 	/**
 	 * Set beamWidthError with the specified vector<vector<Angle > >.
 	 * @param beamWidthError The vector<vector<Angle > > value to which beamWidthError is to be set.
 	 
 		
 	 */
 	void setBeamWidthError (vector<vector<Angle > > beamWidthError);
		
	
	
	
	/**
	 * Mark beamWidthError, which is an optional field, as non-existent.
	 */
	void clearBeamWidthError ();
	


	
	// ===> Attribute beamWidthWasFixed, which is optional
	
	
	
	/**
	 * The attribute beamWidthWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthWasFixed attribute exists. 
	 */
	bool isBeamWidthWasFixedExists() const;
	

	
 	/**
 	 * Get beamWidthWasFixed, which is optional.
 	 * @return beamWidthWasFixed as vector<bool >
 	 * @throws IllegalAccessException If beamWidthWasFixed does not exist.
 	 */
 	vector<bool > getBeamWidthWasFixed() const;
	
 
 	
 	
 	/**
 	 * Set beamWidthWasFixed with the specified vector<bool >.
 	 * @param beamWidthWasFixed The vector<bool > value to which beamWidthWasFixed is to be set.
 	 
 		
 	 */
 	void setBeamWidthWasFixed (vector<bool > beamWidthWasFixed);
		
	
	
	
	/**
	 * Mark beamWidthWasFixed, which is an optional field, as non-existent.
	 */
	void clearBeamWidthWasFixed ();
	


	
	// ===> Attribute offIntensity, which is optional
	
	
	
	/**
	 * The attribute offIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensity attribute exists. 
	 */
	bool isOffIntensityExists() const;
	

	
 	/**
 	 * Get offIntensity, which is optional.
 	 * @return offIntensity as vector<Temperature >
 	 * @throws IllegalAccessException If offIntensity does not exist.
 	 */
 	vector<Temperature > getOffIntensity() const;
	
 
 	
 	
 	/**
 	 * Set offIntensity with the specified vector<Temperature >.
 	 * @param offIntensity The vector<Temperature > value to which offIntensity is to be set.
 	 
 		
 	 */
 	void setOffIntensity (vector<Temperature > offIntensity);
		
	
	
	
	/**
	 * Mark offIntensity, which is an optional field, as non-existent.
	 */
	void clearOffIntensity ();
	


	
	// ===> Attribute offIntensityError, which is optional
	
	
	
	/**
	 * The attribute offIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensityError attribute exists. 
	 */
	bool isOffIntensityErrorExists() const;
	

	
 	/**
 	 * Get offIntensityError, which is optional.
 	 * @return offIntensityError as vector<Temperature >
 	 * @throws IllegalAccessException If offIntensityError does not exist.
 	 */
 	vector<Temperature > getOffIntensityError() const;
	
 
 	
 	
 	/**
 	 * Set offIntensityError with the specified vector<Temperature >.
 	 * @param offIntensityError The vector<Temperature > value to which offIntensityError is to be set.
 	 
 		
 	 */
 	void setOffIntensityError (vector<Temperature > offIntensityError);
		
	
	
	
	/**
	 * Mark offIntensityError, which is an optional field, as non-existent.
	 */
	void clearOffIntensityError ();
	


	
	// ===> Attribute offIntensityWasFixed, which is optional
	
	
	
	/**
	 * The attribute offIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensityWasFixed attribute exists. 
	 */
	bool isOffIntensityWasFixedExists() const;
	

	
 	/**
 	 * Get offIntensityWasFixed, which is optional.
 	 * @return offIntensityWasFixed as bool
 	 * @throws IllegalAccessException If offIntensityWasFixed does not exist.
 	 */
 	bool getOffIntensityWasFixed() const;
	
 
 	
 	
 	/**
 	 * Set offIntensityWasFixed with the specified bool.
 	 * @param offIntensityWasFixed The bool value to which offIntensityWasFixed is to be set.
 	 
 		
 	 */
 	void setOffIntensityWasFixed (bool offIntensityWasFixed);
		
	
	
	
	/**
	 * Mark offIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void clearOffIntensityWasFixed ();
	


	
	// ===> Attribute peakIntensity, which is optional
	
	
	
	/**
	 * The attribute peakIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensity attribute exists. 
	 */
	bool isPeakIntensityExists() const;
	

	
 	/**
 	 * Get peakIntensity, which is optional.
 	 * @return peakIntensity as vector<Temperature >
 	 * @throws IllegalAccessException If peakIntensity does not exist.
 	 */
 	vector<Temperature > getPeakIntensity() const;
	
 
 	
 	
 	/**
 	 * Set peakIntensity with the specified vector<Temperature >.
 	 * @param peakIntensity The vector<Temperature > value to which peakIntensity is to be set.
 	 
 		
 	 */
 	void setPeakIntensity (vector<Temperature > peakIntensity);
		
	
	
	
	/**
	 * Mark peakIntensity, which is an optional field, as non-existent.
	 */
	void clearPeakIntensity ();
	


	
	// ===> Attribute peakIntensityError, which is optional
	
	
	
	/**
	 * The attribute peakIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityError attribute exists. 
	 */
	bool isPeakIntensityErrorExists() const;
	

	
 	/**
 	 * Get peakIntensityError, which is optional.
 	 * @return peakIntensityError as vector<Temperature >
 	 * @throws IllegalAccessException If peakIntensityError does not exist.
 	 */
 	vector<Temperature > getPeakIntensityError() const;
	
 
 	
 	
 	/**
 	 * Set peakIntensityError with the specified vector<Temperature >.
 	 * @param peakIntensityError The vector<Temperature > value to which peakIntensityError is to be set.
 	 
 		
 	 */
 	void setPeakIntensityError (vector<Temperature > peakIntensityError);
		
	
	
	
	/**
	 * Mark peakIntensityError, which is an optional field, as non-existent.
	 */
	void clearPeakIntensityError ();
	


	
	// ===> Attribute peakIntensityWasFixed, which is optional
	
	
	
	/**
	 * The attribute peakIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityWasFixed attribute exists. 
	 */
	bool isPeakIntensityWasFixedExists() const;
	

	
 	/**
 	 * Get peakIntensityWasFixed, which is optional.
 	 * @return peakIntensityWasFixed as bool
 	 * @throws IllegalAccessException If peakIntensityWasFixed does not exist.
 	 */
 	bool getPeakIntensityWasFixed() const;
	
 
 	
 	
 	/**
 	 * Set peakIntensityWasFixed with the specified bool.
 	 * @param peakIntensityWasFixed The bool value to which peakIntensityWasFixed is to be set.
 	 
 		
 	 */
 	void setPeakIntensityWasFixed (bool peakIntensityWasFixed);
		
	
	
	
	/**
	 * Mark peakIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void clearPeakIntensityWasFixed ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag getCalDataId() const;
	
 
 	
 	
 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setCalDataId (Tag calDataId);
  		
	
	
	


	
	// ===> Attribute calReductionId
	
	
	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag getCalReductionId() const;
	
 
 	
 	
 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setCalReductionId (Tag calReductionId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * calDataId pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* getCalDataUsingCalDataId();
	 

	

	

	
		
	/**
	 * calReductionId pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* getCalReductionUsingCalReductionId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalPointingRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaName
	    
	 * @param receiverBand
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param ambientTemperature
	    
	 * @param antennaMake
	    
	 * @param atmPhaseCorrection
	    
	 * @param direction
	    
	 * @param frequencyRange
	    
	 * @param pointingModelMode
	    
	 * @param pointingMethod
	    
	 * @param numReceptor
	    
	 * @param polarizationTypes
	    
	 * @param collOffsetRelative
	    
	 * @param collOffsetAbsolute
	    
	 * @param collError
	    
	 * @param collOffsetTied
	    
	 * @param reducedChiSquared
	    
	 */ 
	bool compareNoAutoInc(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AntennaMakeMod::AntennaMake antennaMake, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, vector<Angle > direction, vector<Frequency > frequencyRange, PointingModelModeMod::PointingModelMode pointingModelMode, PointingMethodMod::PointingMethod pointingMethod, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Angle > > collOffsetRelative, vector<vector<Angle > > collOffsetAbsolute, vector<vector<Angle > > collError, vector<vector<bool > > collOffsetTied, vector<double > reducedChiSquared);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param ambientTemperature
	    
	 * @param antennaMake
	    
	 * @param atmPhaseCorrection
	    
	 * @param direction
	    
	 * @param frequencyRange
	    
	 * @param pointingModelMode
	    
	 * @param pointingMethod
	    
	 * @param numReceptor
	    
	 * @param polarizationTypes
	    
	 * @param collOffsetRelative
	    
	 * @param collOffsetAbsolute
	    
	 * @param collError
	    
	 * @param collOffsetTied
	    
	 * @param reducedChiSquared
	    
	 */ 
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AntennaMakeMod::AntennaMake antennaMake, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, vector<Angle > direction, vector<Frequency > frequencyRange, PointingModelModeMod::PointingModelMode pointingModelMode, PointingMethodMod::PointingMethod pointingMethod, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Angle > > collOffsetRelative, vector<vector<Angle > > collOffsetAbsolute, vector<vector<Angle > > collError, vector<vector<bool > > collOffsetTied, vector<double > reducedChiSquared); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPointingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalPointingRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPointingRowIDL struct.
	 */
	CalPointingRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPointingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (CalPointingRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (string rowDoc) ;	

private:
	/**
	 * The table to which this row belongs.
	 */
	CalPointingTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a CalPointingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPointingRow (CalPointingTable &table);

	/**
	 * Create a CalPointingRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalPointingRow row and a CalPointingTable table, the method creates a new
	 * CalPointingRow owned by table. Each attribute of the created row is a copy (deep)
	 * of the corresponding attribute of row. The method does not add the created
	 * row to its table, its simply parents it to table, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a row with default values for its attributes. 
	 *
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 * @param row  The row which is to be copied.
	 */
	 CalPointingRow (CalPointingTable &table, CalPointingRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute ambientTemperature
	
	

	Temperature ambientTemperature;

	
	
 	

	
	// ===> Attribute antennaMake
	
	

	AntennaMakeMod::AntennaMake antennaMake;

	
	
 	

	
	// ===> Attribute atmPhaseCorrection
	
	

	AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection;

	
	
 	

	
	// ===> Attribute direction
	
	

	vector<Angle > direction;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute pointingModelMode
	
	

	PointingModelModeMod::PointingModelMode pointingModelMode;

	
	
 	

	
	// ===> Attribute pointingMethod
	
	

	PointingMethodMod::PointingMethod pointingMethod;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute collOffsetRelative
	
	

	vector<vector<Angle > > collOffsetRelative;

	
	
 	

	
	// ===> Attribute collOffsetAbsolute
	
	

	vector<vector<Angle > > collOffsetAbsolute;

	
	
 	

	
	// ===> Attribute collError
	
	

	vector<vector<Angle > > collError;

	
	
 	

	
	// ===> Attribute collOffsetTied
	
	

	vector<vector<bool > > collOffsetTied;

	
	
 	

	
	// ===> Attribute reducedChiSquared
	
	

	vector<double > reducedChiSquared;

	
	
 	

	
	// ===> Attribute averagedPolarizations, which is optional
	
	
	bool averagedPolarizationsExists;
	

	bool averagedPolarizations;

	
	
 	

	
	// ===> Attribute beamPA, which is optional
	
	
	bool beamPAExists;
	

	vector<Angle > beamPA;

	
	
 	

	
	// ===> Attribute beamPAError, which is optional
	
	
	bool beamPAErrorExists;
	

	vector<Angle > beamPAError;

	
	
 	

	
	// ===> Attribute beamPAWasFixed, which is optional
	
	
	bool beamPAWasFixedExists;
	

	bool beamPAWasFixed;

	
	
 	

	
	// ===> Attribute beamWidth, which is optional
	
	
	bool beamWidthExists;
	

	vector<vector<Angle > > beamWidth;

	
	
 	

	
	// ===> Attribute beamWidthError, which is optional
	
	
	bool beamWidthErrorExists;
	

	vector<vector<Angle > > beamWidthError;

	
	
 	

	
	// ===> Attribute beamWidthWasFixed, which is optional
	
	
	bool beamWidthWasFixedExists;
	

	vector<bool > beamWidthWasFixed;

	
	
 	

	
	// ===> Attribute offIntensity, which is optional
	
	
	bool offIntensityExists;
	

	vector<Temperature > offIntensity;

	
	
 	

	
	// ===> Attribute offIntensityError, which is optional
	
	
	bool offIntensityErrorExists;
	

	vector<Temperature > offIntensityError;

	
	
 	

	
	// ===> Attribute offIntensityWasFixed, which is optional
	
	
	bool offIntensityWasFixedExists;
	

	bool offIntensityWasFixed;

	
	
 	

	
	// ===> Attribute peakIntensity, which is optional
	
	
	bool peakIntensityExists;
	

	vector<Temperature > peakIntensity;

	
	
 	

	
	// ===> Attribute peakIntensityError, which is optional
	
	
	bool peakIntensityErrorExists;
	

	vector<Temperature > peakIntensityError;

	
	
 	

	
	// ===> Attribute peakIntensityWasFixed, which is optional
	
	
	bool peakIntensityWasFixedExists;
	

	bool peakIntensityWasFixed;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	

	Tag calDataId;

	
	
 	

	
	// ===> Attribute calReductionId
	
	

	Tag calReductionId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, CalPointingAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianISStream& eiss);
void receiverBandFromBin( EndianISStream& eiss);
void calDataIdFromBin( EndianISStream& eiss);
void calReductionIdFromBin( EndianISStream& eiss);
void startValidTimeFromBin( EndianISStream& eiss);
void endValidTimeFromBin( EndianISStream& eiss);
void ambientTemperatureFromBin( EndianISStream& eiss);
void antennaMakeFromBin( EndianISStream& eiss);
void atmPhaseCorrectionFromBin( EndianISStream& eiss);
void directionFromBin( EndianISStream& eiss);
void frequencyRangeFromBin( EndianISStream& eiss);
void pointingModelModeFromBin( EndianISStream& eiss);
void pointingMethodFromBin( EndianISStream& eiss);
void numReceptorFromBin( EndianISStream& eiss);
void polarizationTypesFromBin( EndianISStream& eiss);
void collOffsetRelativeFromBin( EndianISStream& eiss);
void collOffsetAbsoluteFromBin( EndianISStream& eiss);
void collErrorFromBin( EndianISStream& eiss);
void collOffsetTiedFromBin( EndianISStream& eiss);
void reducedChiSquaredFromBin( EndianISStream& eiss);

void averagedPolarizationsFromBin( EndianISStream& eiss);
void beamPAFromBin( EndianISStream& eiss);
void beamPAErrorFromBin( EndianISStream& eiss);
void beamPAWasFixedFromBin( EndianISStream& eiss);
void beamWidthFromBin( EndianISStream& eiss);
void beamWidthErrorFromBin( EndianISStream& eiss);
void beamWidthWasFixedFromBin( EndianISStream& eiss);
void offIntensityFromBin( EndianISStream& eiss);
void offIntensityErrorFromBin( EndianISStream& eiss);
void offIntensityWasFixedFromBin( EndianISStream& eiss);
void peakIntensityFromBin( EndianISStream& eiss);
void peakIntensityErrorFromBin( EndianISStream& eiss);
void peakIntensityWasFixedFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the CalPointingTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalPointingRow* fromBin(EndianISStream& eiss, CalPointingTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* CalPointing_CLASS */
