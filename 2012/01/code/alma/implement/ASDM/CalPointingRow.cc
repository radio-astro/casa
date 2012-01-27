
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
 * File CalPointingRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalPointingRow.h>
#include <CalPointingTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalPointingRow;
using asdm::CalPointingTable;

using asdm::CalDataTable;
using asdm::CalDataRow;

using asdm::CalReductionTable;
using asdm::CalReductionRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	CalPointingRow::~CalPointingRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalPointingTable &CalPointingRow::getTable() const {
		return table;
	}

	bool CalPointingRow::isAdded() const {
		return hasBeenAdded;
	}	

	void CalPointingRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPointingRowIDL struct.
	 */
	CalPointingRowIDL *CalPointingRow::toIDL() const {
		CalPointingRowIDL *x = new CalPointingRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->ambientTemperature = ambientTemperature.toIDLTemperature();
			
		
	

	
  		
		
		
			
				
		x->antennaMake = antennaMake;
 				
 			
		
	

	
  		
		
		
			
				
		x->atmPhaseCorrection = atmPhaseCorrection;
 				
 			
		
	

	
  		
		
		
			
		x->direction.length(direction.size());
		for (unsigned int i = 0; i < direction.size(); ++i) {
			
			x->direction[i] = direction.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->pointingModelMode = pointingModelMode;
 				
 			
		
	

	
  		
		
		
			
				
		x->pointingMethod = pointingMethod;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->collOffsetRelative.length(collOffsetRelative.size());
		for (unsigned int i = 0; i < collOffsetRelative.size(); i++) {
			x->collOffsetRelative[i].length(collOffsetRelative.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < collOffsetRelative.size() ; i++)
			for (unsigned int j = 0; j < collOffsetRelative.at(i).size(); j++)
					
				x->collOffsetRelative[i][j]= collOffsetRelative.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
		x->collOffsetAbsolute.length(collOffsetAbsolute.size());
		for (unsigned int i = 0; i < collOffsetAbsolute.size(); i++) {
			x->collOffsetAbsolute[i].length(collOffsetAbsolute.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < collOffsetAbsolute.size() ; i++)
			for (unsigned int j = 0; j < collOffsetAbsolute.at(i).size(); j++)
					
				x->collOffsetAbsolute[i][j]= collOffsetAbsolute.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
		x->collError.length(collError.size());
		for (unsigned int i = 0; i < collError.size(); i++) {
			x->collError[i].length(collError.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < collError.size() ; i++)
			for (unsigned int j = 0; j < collError.at(i).size(); j++)
					
				x->collError[i][j]= collError.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
		x->collOffsetTied.length(collOffsetTied.size());
		for (unsigned int i = 0; i < collOffsetTied.size(); i++) {
			x->collOffsetTied[i].length(collOffsetTied.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < collOffsetTied.size() ; i++)
			for (unsigned int j = 0; j < collOffsetTied.at(i).size(); j++)
					
						
				x->collOffsetTied[i][j] = collOffsetTied.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->reducedChiSquared.length(reducedChiSquared.size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); ++i) {
			
				
			x->reducedChiSquared[i] = reducedChiSquared.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->averagedPolarizationsExists = averagedPolarizationsExists;
		
		
			
				
		x->averagedPolarizations = averagedPolarizations;
 				
 			
		
	

	
  		
		
		x->beamPAExists = beamPAExists;
		
		
			
		x->beamPA.length(beamPA.size());
		for (unsigned int i = 0; i < beamPA.size(); ++i) {
			
			x->beamPA[i] = beamPA.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->beamPAErrorExists = beamPAErrorExists;
		
		
			
		x->beamPAError.length(beamPAError.size());
		for (unsigned int i = 0; i < beamPAError.size(); ++i) {
			
			x->beamPAError[i] = beamPAError.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->beamPAWasFixedExists = beamPAWasFixedExists;
		
		
			
				
		x->beamPAWasFixed = beamPAWasFixed;
 				
 			
		
	

	
  		
		
		x->beamWidthExists = beamWidthExists;
		
		
			
		x->beamWidth.length(beamWidth.size());
		for (unsigned int i = 0; i < beamWidth.size(); i++) {
			x->beamWidth[i].length(beamWidth.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < beamWidth.size() ; i++)
			for (unsigned int j = 0; j < beamWidth.at(i).size(); j++)
					
				x->beamWidth[i][j]= beamWidth.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->beamWidthErrorExists = beamWidthErrorExists;
		
		
			
		x->beamWidthError.length(beamWidthError.size());
		for (unsigned int i = 0; i < beamWidthError.size(); i++) {
			x->beamWidthError[i].length(beamWidthError.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < beamWidthError.size() ; i++)
			for (unsigned int j = 0; j < beamWidthError.at(i).size(); j++)
					
				x->beamWidthError[i][j]= beamWidthError.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->beamWidthWasFixedExists = beamWidthWasFixedExists;
		
		
			
		x->beamWidthWasFixed.length(beamWidthWasFixed.size());
		for (unsigned int i = 0; i < beamWidthWasFixed.size(); ++i) {
			
				
			x->beamWidthWasFixed[i] = beamWidthWasFixed.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->offIntensityExists = offIntensityExists;
		
		
			
		x->offIntensity.length(offIntensity.size());
		for (unsigned int i = 0; i < offIntensity.size(); ++i) {
			
			x->offIntensity[i] = offIntensity.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->offIntensityErrorExists = offIntensityErrorExists;
		
		
			
		x->offIntensityError.length(offIntensityError.size());
		for (unsigned int i = 0; i < offIntensityError.size(); ++i) {
			
			x->offIntensityError[i] = offIntensityError.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->offIntensityWasFixedExists = offIntensityWasFixedExists;
		
		
			
				
		x->offIntensityWasFixed = offIntensityWasFixed;
 				
 			
		
	

	
  		
		
		x->peakIntensityExists = peakIntensityExists;
		
		
			
		x->peakIntensity.length(peakIntensity.size());
		for (unsigned int i = 0; i < peakIntensity.size(); ++i) {
			
			x->peakIntensity[i] = peakIntensity.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->peakIntensityErrorExists = peakIntensityErrorExists;
		
		
			
		x->peakIntensityError.length(peakIntensityError.size());
		for (unsigned int i = 0; i < peakIntensityError.size(); ++i) {
			
			x->peakIntensityError[i] = peakIntensityError.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->peakIntensityWasFixedExists = peakIntensityWasFixedExists;
		
		
			
				
		x->peakIntensityWasFixed = peakIntensityWasFixed;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPointingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalPointingRow::setFromIDL (CalPointingRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setAmbientTemperature(Temperature (x.ambientTemperature));
			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
		
		
			
		setAtmPhaseCorrection(x.atmPhaseCorrection);
  			
 		
		
	

	
		
		
			
		direction .clear();
		for (unsigned int i = 0; i <x.direction.length(); ++i) {
			
			direction.push_back(Angle (x.direction[i]));
			
		}
			
  		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		setPointingModelMode(x.pointingModelMode);
  			
 		
		
	

	
		
		
			
		setPointingMethod(x.pointingMethod);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		collOffsetRelative .clear();
		vector<Angle> v_aux_collOffsetRelative;
		for (unsigned int i = 0; i < x.collOffsetRelative.length(); ++i) {
			v_aux_collOffsetRelative.clear();
			for (unsigned int j = 0; j < x.collOffsetRelative[0].length(); ++j) {
				
				v_aux_collOffsetRelative.push_back(Angle (x.collOffsetRelative[i][j]));
				
  			}
  			collOffsetRelative.push_back(v_aux_collOffsetRelative);			
		}
			
  		
		
	

	
		
		
			
		collOffsetAbsolute .clear();
		vector<Angle> v_aux_collOffsetAbsolute;
		for (unsigned int i = 0; i < x.collOffsetAbsolute.length(); ++i) {
			v_aux_collOffsetAbsolute.clear();
			for (unsigned int j = 0; j < x.collOffsetAbsolute[0].length(); ++j) {
				
				v_aux_collOffsetAbsolute.push_back(Angle (x.collOffsetAbsolute[i][j]));
				
  			}
  			collOffsetAbsolute.push_back(v_aux_collOffsetAbsolute);			
		}
			
  		
		
	

	
		
		
			
		collError .clear();
		vector<Angle> v_aux_collError;
		for (unsigned int i = 0; i < x.collError.length(); ++i) {
			v_aux_collError.clear();
			for (unsigned int j = 0; j < x.collError[0].length(); ++j) {
				
				v_aux_collError.push_back(Angle (x.collError[i][j]));
				
  			}
  			collError.push_back(v_aux_collError);			
		}
			
  		
		
	

	
		
		
			
		collOffsetTied .clear();
		vector<bool> v_aux_collOffsetTied;
		for (unsigned int i = 0; i < x.collOffsetTied.length(); ++i) {
			v_aux_collOffsetTied.clear();
			for (unsigned int j = 0; j < x.collOffsetTied[0].length(); ++j) {
				
				v_aux_collOffsetTied.push_back(x.collOffsetTied[i][j]);
	  			
  			}
  			collOffsetTied.push_back(v_aux_collOffsetTied);			
		}
			
  		
		
	

	
		
		
			
		reducedChiSquared .clear();
		for (unsigned int i = 0; i <x.reducedChiSquared.length(); ++i) {
			
			reducedChiSquared.push_back(x.reducedChiSquared[i]);
  			
		}
			
  		
		
	

	
		
		averagedPolarizationsExists = x.averagedPolarizationsExists;
		if (x.averagedPolarizationsExists) {
		
		
			
		setAveragedPolarizations(x.averagedPolarizations);
  			
 		
		
		}
		
	

	
		
		beamPAExists = x.beamPAExists;
		if (x.beamPAExists) {
		
		
			
		beamPA .clear();
		for (unsigned int i = 0; i <x.beamPA.length(); ++i) {
			
			beamPA.push_back(Angle (x.beamPA[i]));
			
		}
			
  		
		
		}
		
	

	
		
		beamPAErrorExists = x.beamPAErrorExists;
		if (x.beamPAErrorExists) {
		
		
			
		beamPAError .clear();
		for (unsigned int i = 0; i <x.beamPAError.length(); ++i) {
			
			beamPAError.push_back(Angle (x.beamPAError[i]));
			
		}
			
  		
		
		}
		
	

	
		
		beamPAWasFixedExists = x.beamPAWasFixedExists;
		if (x.beamPAWasFixedExists) {
		
		
			
		setBeamPAWasFixed(x.beamPAWasFixed);
  			
 		
		
		}
		
	

	
		
		beamWidthExists = x.beamWidthExists;
		if (x.beamWidthExists) {
		
		
			
		beamWidth .clear();
		vector<Angle> v_aux_beamWidth;
		for (unsigned int i = 0; i < x.beamWidth.length(); ++i) {
			v_aux_beamWidth.clear();
			for (unsigned int j = 0; j < x.beamWidth[0].length(); ++j) {
				
				v_aux_beamWidth.push_back(Angle (x.beamWidth[i][j]));
				
  			}
  			beamWidth.push_back(v_aux_beamWidth);			
		}
			
  		
		
		}
		
	

	
		
		beamWidthErrorExists = x.beamWidthErrorExists;
		if (x.beamWidthErrorExists) {
		
		
			
		beamWidthError .clear();
		vector<Angle> v_aux_beamWidthError;
		for (unsigned int i = 0; i < x.beamWidthError.length(); ++i) {
			v_aux_beamWidthError.clear();
			for (unsigned int j = 0; j < x.beamWidthError[0].length(); ++j) {
				
				v_aux_beamWidthError.push_back(Angle (x.beamWidthError[i][j]));
				
  			}
  			beamWidthError.push_back(v_aux_beamWidthError);			
		}
			
  		
		
		}
		
	

	
		
		beamWidthWasFixedExists = x.beamWidthWasFixedExists;
		if (x.beamWidthWasFixedExists) {
		
		
			
		beamWidthWasFixed .clear();
		for (unsigned int i = 0; i <x.beamWidthWasFixed.length(); ++i) {
			
			beamWidthWasFixed.push_back(x.beamWidthWasFixed[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		offIntensityExists = x.offIntensityExists;
		if (x.offIntensityExists) {
		
		
			
		offIntensity .clear();
		for (unsigned int i = 0; i <x.offIntensity.length(); ++i) {
			
			offIntensity.push_back(Temperature (x.offIntensity[i]));
			
		}
			
  		
		
		}
		
	

	
		
		offIntensityErrorExists = x.offIntensityErrorExists;
		if (x.offIntensityErrorExists) {
		
		
			
		offIntensityError .clear();
		for (unsigned int i = 0; i <x.offIntensityError.length(); ++i) {
			
			offIntensityError.push_back(Temperature (x.offIntensityError[i]));
			
		}
			
  		
		
		}
		
	

	
		
		offIntensityWasFixedExists = x.offIntensityWasFixedExists;
		if (x.offIntensityWasFixedExists) {
		
		
			
		setOffIntensityWasFixed(x.offIntensityWasFixed);
  			
 		
		
		}
		
	

	
		
		peakIntensityExists = x.peakIntensityExists;
		if (x.peakIntensityExists) {
		
		
			
		peakIntensity .clear();
		for (unsigned int i = 0; i <x.peakIntensity.length(); ++i) {
			
			peakIntensity.push_back(Temperature (x.peakIntensity[i]));
			
		}
			
  		
		
		}
		
	

	
		
		peakIntensityErrorExists = x.peakIntensityErrorExists;
		if (x.peakIntensityErrorExists) {
		
		
			
		peakIntensityError .clear();
		for (unsigned int i = 0; i <x.peakIntensityError.length(); ++i) {
			
			peakIntensityError.push_back(Temperature (x.peakIntensityError[i]));
			
		}
			
  		
		
		}
		
	

	
		
		peakIntensityWasFixedExists = x.peakIntensityWasFixedExists;
		if (x.peakIntensityWasFixedExists) {
		
		
			
		setPeakIntensityWasFixed(x.peakIntensityWasFixed);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPointing");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalPointingRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(ambientTemperature, "ambientTemperature", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaMake", antennaMake));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrection", atmPhaseCorrection));
		
		
	

  	
 		
		
		Parser::toXML(direction, "direction", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("pointingModelMode", pointingModelMode));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("pointingMethod", pointingMethod));
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(collOffsetRelative, "collOffsetRelative", buf);
		
		
	

  	
 		
		
		Parser::toXML(collOffsetAbsolute, "collOffsetAbsolute", buf);
		
		
	

  	
 		
		
		Parser::toXML(collError, "collError", buf);
		
		
	

  	
 		
		
		Parser::toXML(collOffsetTied, "collOffsetTied", buf);
		
		
	

  	
 		
		
		Parser::toXML(reducedChiSquared, "reducedChiSquared", buf);
		
		
	

  	
 		
		if (averagedPolarizationsExists) {
		
		
		Parser::toXML(averagedPolarizations, "averagedPolarizations", buf);
		
		
		}
		
	

  	
 		
		if (beamPAExists) {
		
		
		Parser::toXML(beamPA, "beamPA", buf);
		
		
		}
		
	

  	
 		
		if (beamPAErrorExists) {
		
		
		Parser::toXML(beamPAError, "beamPAError", buf);
		
		
		}
		
	

  	
 		
		if (beamPAWasFixedExists) {
		
		
		Parser::toXML(beamPAWasFixed, "beamPAWasFixed", buf);
		
		
		}
		
	

  	
 		
		if (beamWidthExists) {
		
		
		Parser::toXML(beamWidth, "beamWidth", buf);
		
		
		}
		
	

  	
 		
		if (beamWidthErrorExists) {
		
		
		Parser::toXML(beamWidthError, "beamWidthError", buf);
		
		
		}
		
	

  	
 		
		if (beamWidthWasFixedExists) {
		
		
		Parser::toXML(beamWidthWasFixed, "beamWidthWasFixed", buf);
		
		
		}
		
	

  	
 		
		if (offIntensityExists) {
		
		
		Parser::toXML(offIntensity, "offIntensity", buf);
		
		
		}
		
	

  	
 		
		if (offIntensityErrorExists) {
		
		
		Parser::toXML(offIntensityError, "offIntensityError", buf);
		
		
		}
		
	

  	
 		
		if (offIntensityWasFixedExists) {
		
		
		Parser::toXML(offIntensityWasFixed, "offIntensityWasFixed", buf);
		
		
		}
		
	

  	
 		
		if (peakIntensityExists) {
		
		
		Parser::toXML(peakIntensity, "peakIntensity", buf);
		
		
		}
		
	

  	
 		
		if (peakIntensityErrorExists) {
		
		
		Parser::toXML(peakIntensityError, "peakIntensityError", buf);
		
		
		}
		
	

  	
 		
		if (peakIntensityWasFixedExists) {
		
		
		Parser::toXML(peakIntensityWasFixed, "peakIntensityWasFixed", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(calDataId, "calDataId", buf);
		
		
	

  	
 		
		
		Parser::toXML(calReductionId, "calReductionId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalPointingRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalPointing",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalPointing",rowDoc);
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalPointing",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalPointing",rowDoc));
			
		
	

	
  		
			
	  	setAmbientTemperature(Parser::getTemperature("ambientTemperature","CalPointing",rowDoc));
			
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","CalPointing",rowDoc);
		
		
		
	

	
		
		
		
		atmPhaseCorrection = EnumerationParser::getAtmPhaseCorrection("atmPhaseCorrection","CalPointing",rowDoc);
		
		
		
	

	
  		
			
					
	  	setDirection(Parser::get1DAngle("direction","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalPointing",rowDoc));
	  			
	  		
		
	

	
		
		
		
		pointingModelMode = EnumerationParser::getPointingModelMode("pointingModelMode","CalPointing",rowDoc);
		
		
		
	

	
		
		
		
		pointingMethod = EnumerationParser::getPointingMethod("pointingMethod","CalPointing",rowDoc);
		
		
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalPointing",rowDoc));
			
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalPointing",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setCollOffsetRelative(Parser::get2DAngle("collOffsetRelative","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCollOffsetAbsolute(Parser::get2DAngle("collOffsetAbsolute","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCollError(Parser::get2DAngle("collError","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCollOffsetTied(Parser::get2DBoolean("collOffsetTied","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setReducedChiSquared(Parser::get1DDouble("reducedChiSquared","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<averagedPolarizations>")) {
			
	  		setAveragedPolarizations(Parser::getBoolean("averagedPolarizations","CalPointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<beamPA>")) {
			
								
	  		setBeamPA(Parser::get1DAngle("beamPA","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<beamPAError>")) {
			
								
	  		setBeamPAError(Parser::get1DAngle("beamPAError","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<beamPAWasFixed>")) {
			
	  		setBeamPAWasFixed(Parser::getBoolean("beamPAWasFixed","CalPointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<beamWidth>")) {
			
								
	  		setBeamWidth(Parser::get2DAngle("beamWidth","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<beamWidthError>")) {
			
								
	  		setBeamWidthError(Parser::get2DAngle("beamWidthError","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<beamWidthWasFixed>")) {
			
								
	  		setBeamWidthWasFixed(Parser::get1DBoolean("beamWidthWasFixed","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<offIntensity>")) {
			
								
	  		setOffIntensity(Parser::get1DTemperature("offIntensity","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<offIntensityError>")) {
			
								
	  		setOffIntensityError(Parser::get1DTemperature("offIntensityError","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<offIntensityWasFixed>")) {
			
	  		setOffIntensityWasFixed(Parser::getBoolean("offIntensityWasFixed","CalPointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<peakIntensity>")) {
			
								
	  		setPeakIntensity(Parser::get1DTemperature("peakIntensity","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<peakIntensityError>")) {
			
								
	  		setPeakIntensityError(Parser::get1DTemperature("peakIntensityError","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<peakIntensityWasFixed>")) {
			
	  		setPeakIntensityWasFixed(Parser::getBoolean("peakIntensityWasFixed","CalPointing",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPointing");
		}
	}
	
	void CalPointingRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeString(antennaName);
				
		
	

	
	
		
					
			eoss.writeInt(receiverBand);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
	ambientTemperature.toBin(eoss);
		
	

	
	
		
					
			eoss.writeInt(antennaMake);
				
		
	

	
	
		
					
			eoss.writeInt(atmPhaseCorrection);
				
		
	

	
	
		
	Angle::toBin(direction, eoss);
		
	

	
	
		
	Frequency::toBin(frequencyRange, eoss);
		
	

	
	
		
					
			eoss.writeInt(pointingModelMode);
				
		
	

	
	
		
					
			eoss.writeInt(pointingMethod);
				
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
	Angle::toBin(collOffsetRelative, eoss);
		
	

	
	
		
	Angle::toBin(collOffsetAbsolute, eoss);
		
	

	
	
		
	Angle::toBin(collError, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) collOffsetTied.size());
		eoss.writeInt((int) collOffsetTied.at(0).size());
		for (unsigned int i = 0; i < collOffsetTied.size(); i++) 
			for (unsigned int j = 0;  j < collOffsetTied.at(0).size(); j++) 
							 
				eoss.writeBoolean(collOffsetTied.at(i).at(j));
				
	
						
		
	

	
	
		
		
			
		eoss.writeInt((int) reducedChiSquared.size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); i++)
				
			eoss.writeDouble(reducedChiSquared.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(averagedPolarizationsExists);
	if (averagedPolarizationsExists) {
	
	
	
		
						
			eoss.writeBoolean(averagedPolarizations);
				
		
	

	}

	eoss.writeBoolean(beamPAExists);
	if (beamPAExists) {
	
	
	
		
	Angle::toBin(beamPA, eoss);
		
	

	}

	eoss.writeBoolean(beamPAErrorExists);
	if (beamPAErrorExists) {
	
	
	
		
	Angle::toBin(beamPAError, eoss);
		
	

	}

	eoss.writeBoolean(beamPAWasFixedExists);
	if (beamPAWasFixedExists) {
	
	
	
		
						
			eoss.writeBoolean(beamPAWasFixed);
				
		
	

	}

	eoss.writeBoolean(beamWidthExists);
	if (beamWidthExists) {
	
	
	
		
	Angle::toBin(beamWidth, eoss);
		
	

	}

	eoss.writeBoolean(beamWidthErrorExists);
	if (beamWidthErrorExists) {
	
	
	
		
	Angle::toBin(beamWidthError, eoss);
		
	

	}

	eoss.writeBoolean(beamWidthWasFixedExists);
	if (beamWidthWasFixedExists) {
	
	
	
		
		
			
		eoss.writeInt((int) beamWidthWasFixed.size());
		for (unsigned int i = 0; i < beamWidthWasFixed.size(); i++)
				
			eoss.writeBoolean(beamWidthWasFixed.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(offIntensityExists);
	if (offIntensityExists) {
	
	
	
		
	Temperature::toBin(offIntensity, eoss);
		
	

	}

	eoss.writeBoolean(offIntensityErrorExists);
	if (offIntensityErrorExists) {
	
	
	
		
	Temperature::toBin(offIntensityError, eoss);
		
	

	}

	eoss.writeBoolean(offIntensityWasFixedExists);
	if (offIntensityWasFixedExists) {
	
	
	
		
						
			eoss.writeBoolean(offIntensityWasFixed);
				
		
	

	}

	eoss.writeBoolean(peakIntensityExists);
	if (peakIntensityExists) {
	
	
	
		
	Temperature::toBin(peakIntensity, eoss);
		
	

	}

	eoss.writeBoolean(peakIntensityErrorExists);
	if (peakIntensityErrorExists) {
	
	
	
		
	Temperature::toBin(peakIntensityError, eoss);
		
	

	}

	eoss.writeBoolean(peakIntensityWasFixedExists);
	if (peakIntensityWasFixedExists) {
	
	
	
		
						
			eoss.writeBoolean(peakIntensityWasFixed);
				
		
	

	}

	}
	
void CalPointingRow::antennaNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaName =  eiss.readString();
			
		
	
	
}
void CalPointingRow::receiverBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		receiverBand = CReceiverBand::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingRow::calDataIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calDataId =  Tag::fromBin(eiss);
		
	
	
}
void CalPointingRow::calReductionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calReductionId =  Tag::fromBin(eiss);
		
	
	
}
void CalPointingRow::startValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalPointingRow::endValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalPointingRow::ambientTemperatureFromBin(EndianISStream& eiss) {
		
	
		
		
		ambientTemperature =  Temperature::fromBin(eiss);
		
	
	
}
void CalPointingRow::antennaMakeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaMake = CAntennaMake::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingRow::atmPhaseCorrectionFromBin(EndianISStream& eiss) {
		
	
	
		
			
		atmPhaseCorrection = CAtmPhaseCorrection::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingRow::directionFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	direction = Angle::from1DBin(eiss);	
	

		
	
	
}
void CalPointingRow::frequencyRangeFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	frequencyRange = Frequency::from1DBin(eiss);	
	

		
	
	
}
void CalPointingRow::pointingModelModeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		pointingModelMode = CPointingModelMode::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingRow::pointingMethodFromBin(EndianISStream& eiss) {
		
	
	
		
			
		pointingMethod = CPointingMethod::from_int(eiss.readInt());
			
		
	
	
}
void CalPointingRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void CalPointingRow::polarizationTypesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	
	
}
void CalPointingRow::collOffsetRelativeFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	collOffsetRelative = Angle::from2DBin(eiss);		
	

		
	
	
}
void CalPointingRow::collOffsetAbsoluteFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	collOffsetAbsolute = Angle::from2DBin(eiss);		
	

		
	
	
}
void CalPointingRow::collErrorFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	collError = Angle::from2DBin(eiss);		
	

		
	
	
}
void CalPointingRow::collOffsetTiedFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		collOffsetTied.clear();
		
		unsigned int collOffsetTiedDim1 = eiss.readInt();
		unsigned int collOffsetTiedDim2 = eiss.readInt();
		vector <bool> collOffsetTiedAux1;
		for (unsigned int i = 0; i < collOffsetTiedDim1; i++) {
			collOffsetTiedAux1.clear();
			for (unsigned int j = 0; j < collOffsetTiedDim2 ; j++)			
			
			collOffsetTiedAux1.push_back(eiss.readBoolean());
			
			collOffsetTied.push_back(collOffsetTiedAux1);
		}
	
	

		
	
	
}
void CalPointingRow::reducedChiSquaredFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		reducedChiSquared.clear();
		
		unsigned int reducedChiSquaredDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < reducedChiSquaredDim1; i++)
			
			reducedChiSquared.push_back(eiss.readDouble());
			
	

		
	
	
}

void CalPointingRow::averagedPolarizationsFromBin(EndianISStream& eiss) {
		
	averagedPolarizationsExists = eiss.readBoolean();
	if (averagedPolarizationsExists) {
		
	
	
		
			
		averagedPolarizations =  eiss.readBoolean();
			
		
	

	}
	
}
void CalPointingRow::beamPAFromBin(EndianISStream& eiss) {
		
	beamPAExists = eiss.readBoolean();
	if (beamPAExists) {
		
	
		
		
			
	
	beamPA = Angle::from1DBin(eiss);	
	

		
	

	}
	
}
void CalPointingRow::beamPAErrorFromBin(EndianISStream& eiss) {
		
	beamPAErrorExists = eiss.readBoolean();
	if (beamPAErrorExists) {
		
	
		
		
			
	
	beamPAError = Angle::from1DBin(eiss);	
	

		
	

	}
	
}
void CalPointingRow::beamPAWasFixedFromBin(EndianISStream& eiss) {
		
	beamPAWasFixedExists = eiss.readBoolean();
	if (beamPAWasFixedExists) {
		
	
	
		
			
		beamPAWasFixed =  eiss.readBoolean();
			
		
	

	}
	
}
void CalPointingRow::beamWidthFromBin(EndianISStream& eiss) {
		
	beamWidthExists = eiss.readBoolean();
	if (beamWidthExists) {
		
	
		
		
			
	
	beamWidth = Angle::from2DBin(eiss);		
	

		
	

	}
	
}
void CalPointingRow::beamWidthErrorFromBin(EndianISStream& eiss) {
		
	beamWidthErrorExists = eiss.readBoolean();
	if (beamWidthErrorExists) {
		
	
		
		
			
	
	beamWidthError = Angle::from2DBin(eiss);		
	

		
	

	}
	
}
void CalPointingRow::beamWidthWasFixedFromBin(EndianISStream& eiss) {
		
	beamWidthWasFixedExists = eiss.readBoolean();
	if (beamWidthWasFixedExists) {
		
	
	
		
			
	
		beamWidthWasFixed.clear();
		
		unsigned int beamWidthWasFixedDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < beamWidthWasFixedDim1; i++)
			
			beamWidthWasFixed.push_back(eiss.readBoolean());
			
	

		
	

	}
	
}
void CalPointingRow::offIntensityFromBin(EndianISStream& eiss) {
		
	offIntensityExists = eiss.readBoolean();
	if (offIntensityExists) {
		
	
		
		
			
	
	offIntensity = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalPointingRow::offIntensityErrorFromBin(EndianISStream& eiss) {
		
	offIntensityErrorExists = eiss.readBoolean();
	if (offIntensityErrorExists) {
		
	
		
		
			
	
	offIntensityError = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalPointingRow::offIntensityWasFixedFromBin(EndianISStream& eiss) {
		
	offIntensityWasFixedExists = eiss.readBoolean();
	if (offIntensityWasFixedExists) {
		
	
	
		
			
		offIntensityWasFixed =  eiss.readBoolean();
			
		
	

	}
	
}
void CalPointingRow::peakIntensityFromBin(EndianISStream& eiss) {
		
	peakIntensityExists = eiss.readBoolean();
	if (peakIntensityExists) {
		
	
		
		
			
	
	peakIntensity = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalPointingRow::peakIntensityErrorFromBin(EndianISStream& eiss) {
		
	peakIntensityErrorExists = eiss.readBoolean();
	if (peakIntensityErrorExists) {
		
	
		
		
			
	
	peakIntensityError = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalPointingRow::peakIntensityWasFixedFromBin(EndianISStream& eiss) {
		
	peakIntensityWasFixedExists = eiss.readBoolean();
	if (peakIntensityWasFixedExists) {
		
	
	
		
			
		peakIntensityWasFixed =  eiss.readBoolean();
			
		
	

	}
	
}
	
	
	CalPointingRow* CalPointingRow::fromBin(EndianISStream& eiss, CalPointingTable& table, const vector<string>& attributesSeq) {
		CalPointingRow* row = new  CalPointingRow(table);
		
		map<string, CalPointingAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalPointingTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalPointingRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalPointing");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalPointingRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalPointing");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature CalPointingRow::getAmbientTemperature() const {
	
  		return ambientTemperature;
 	}

 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setAmbientTemperature (Temperature ambientTemperature)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->ambientTemperature = ambientTemperature;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake CalPointingRow::getAntennaMake() const {
	
  		return antennaMake;
 	}

 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setAntennaMake (AntennaMakeMod::AntennaMake antennaMake)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaMake = antennaMake;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection CalPointingRow::getAtmPhaseCorrection() const {
	
  		return atmPhaseCorrection;
 	}

 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmPhaseCorrection = atmPhaseCorrection;
	
 	}
	
	

	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > CalPointingRow::getDirection() const {
	
  		return direction;
 	}

 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setDirection (vector<Angle > direction)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->direction = direction;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalPointingRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingModelMode.
 	 * @return pointingModelMode as PointingModelModeMod::PointingModelMode
 	 */
 	PointingModelModeMod::PointingModelMode CalPointingRow::getPointingModelMode() const {
	
  		return pointingModelMode;
 	}

 	/**
 	 * Set pointingModelMode with the specified PointingModelModeMod::PointingModelMode.
 	 * @param pointingModelMode The PointingModelModeMod::PointingModelMode value to which pointingModelMode is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setPointingModelMode (PointingModelModeMod::PointingModelMode pointingModelMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingModelMode = pointingModelMode;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingMethod.
 	 * @return pointingMethod as PointingMethodMod::PointingMethod
 	 */
 	PointingMethodMod::PointingMethod CalPointingRow::getPointingMethod() const {
	
  		return pointingMethod;
 	}

 	/**
 	 * Set pointingMethod with the specified PointingMethodMod::PointingMethod.
 	 * @param pointingMethod The PointingMethodMod::PointingMethod value to which pointingMethod is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setPointingMethod (PointingMethodMod::PointingMethod pointingMethod)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingMethod = pointingMethod;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalPointingRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalPointingRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get collOffsetRelative.
 	 * @return collOffsetRelative as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > CalPointingRow::getCollOffsetRelative() const {
	
  		return collOffsetRelative;
 	}

 	/**
 	 * Set collOffsetRelative with the specified vector<vector<Angle > >.
 	 * @param collOffsetRelative The vector<vector<Angle > > value to which collOffsetRelative is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setCollOffsetRelative (vector<vector<Angle > > collOffsetRelative)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->collOffsetRelative = collOffsetRelative;
	
 	}
	
	

	

	
 	/**
 	 * Get collOffsetAbsolute.
 	 * @return collOffsetAbsolute as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > CalPointingRow::getCollOffsetAbsolute() const {
	
  		return collOffsetAbsolute;
 	}

 	/**
 	 * Set collOffsetAbsolute with the specified vector<vector<Angle > >.
 	 * @param collOffsetAbsolute The vector<vector<Angle > > value to which collOffsetAbsolute is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setCollOffsetAbsolute (vector<vector<Angle > > collOffsetAbsolute)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->collOffsetAbsolute = collOffsetAbsolute;
	
 	}
	
	

	

	
 	/**
 	 * Get collError.
 	 * @return collError as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > CalPointingRow::getCollError() const {
	
  		return collError;
 	}

 	/**
 	 * Set collError with the specified vector<vector<Angle > >.
 	 * @param collError The vector<vector<Angle > > value to which collError is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setCollError (vector<vector<Angle > > collError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->collError = collError;
	
 	}
	
	

	

	
 	/**
 	 * Get collOffsetTied.
 	 * @return collOffsetTied as vector<vector<bool > >
 	 */
 	vector<vector<bool > > CalPointingRow::getCollOffsetTied() const {
	
  		return collOffsetTied;
 	}

 	/**
 	 * Set collOffsetTied with the specified vector<vector<bool > >.
 	 * @param collOffsetTied The vector<vector<bool > > value to which collOffsetTied is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setCollOffsetTied (vector<vector<bool > > collOffsetTied)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->collOffsetTied = collOffsetTied;
	
 	}
	
	

	

	
 	/**
 	 * Get reducedChiSquared.
 	 * @return reducedChiSquared as vector<double >
 	 */
 	vector<double > CalPointingRow::getReducedChiSquared() const {
	
  		return reducedChiSquared;
 	}

 	/**
 	 * Set reducedChiSquared with the specified vector<double >.
 	 * @param reducedChiSquared The vector<double > value to which reducedChiSquared is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setReducedChiSquared (vector<double > reducedChiSquared)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->reducedChiSquared = reducedChiSquared;
	
 	}
	
	

	
	/**
	 * The attribute averagedPolarizations is optional. Return true if this attribute exists.
	 * @return true if and only if the averagedPolarizations attribute exists. 
	 */
	bool CalPointingRow::isAveragedPolarizationsExists() const {
		return averagedPolarizationsExists;
	}
	

	
 	/**
 	 * Get averagedPolarizations, which is optional.
 	 * @return averagedPolarizations as bool
 	 * @throw IllegalAccessException If averagedPolarizations does not exist.
 	 */
 	bool CalPointingRow::getAveragedPolarizations() const  {
		if (!averagedPolarizationsExists) {
			throw IllegalAccessException("averagedPolarizations", "CalPointing");
		}
	
  		return averagedPolarizations;
 	}

 	/**
 	 * Set averagedPolarizations with the specified bool.
 	 * @param averagedPolarizations The bool value to which averagedPolarizations is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setAveragedPolarizations (bool averagedPolarizations) {
	
 		this->averagedPolarizations = averagedPolarizations;
	
		averagedPolarizationsExists = true;
	
 	}
	
	
	/**
	 * Mark averagedPolarizations, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearAveragedPolarizations () {
		averagedPolarizationsExists = false;
	}
	

	
	/**
	 * The attribute beamPA is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPA attribute exists. 
	 */
	bool CalPointingRow::isBeamPAExists() const {
		return beamPAExists;
	}
	

	
 	/**
 	 * Get beamPA, which is optional.
 	 * @return beamPA as vector<Angle >
 	 * @throw IllegalAccessException If beamPA does not exist.
 	 */
 	vector<Angle > CalPointingRow::getBeamPA() const  {
		if (!beamPAExists) {
			throw IllegalAccessException("beamPA", "CalPointing");
		}
	
  		return beamPA;
 	}

 	/**
 	 * Set beamPA with the specified vector<Angle >.
 	 * @param beamPA The vector<Angle > value to which beamPA is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamPA (vector<Angle > beamPA) {
	
 		this->beamPA = beamPA;
	
		beamPAExists = true;
	
 	}
	
	
	/**
	 * Mark beamPA, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamPA () {
		beamPAExists = false;
	}
	

	
	/**
	 * The attribute beamPAError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAError attribute exists. 
	 */
	bool CalPointingRow::isBeamPAErrorExists() const {
		return beamPAErrorExists;
	}
	

	
 	/**
 	 * Get beamPAError, which is optional.
 	 * @return beamPAError as vector<Angle >
 	 * @throw IllegalAccessException If beamPAError does not exist.
 	 */
 	vector<Angle > CalPointingRow::getBeamPAError() const  {
		if (!beamPAErrorExists) {
			throw IllegalAccessException("beamPAError", "CalPointing");
		}
	
  		return beamPAError;
 	}

 	/**
 	 * Set beamPAError with the specified vector<Angle >.
 	 * @param beamPAError The vector<Angle > value to which beamPAError is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamPAError (vector<Angle > beamPAError) {
	
 		this->beamPAError = beamPAError;
	
		beamPAErrorExists = true;
	
 	}
	
	
	/**
	 * Mark beamPAError, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamPAError () {
		beamPAErrorExists = false;
	}
	

	
	/**
	 * The attribute beamPAWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAWasFixed attribute exists. 
	 */
	bool CalPointingRow::isBeamPAWasFixedExists() const {
		return beamPAWasFixedExists;
	}
	

	
 	/**
 	 * Get beamPAWasFixed, which is optional.
 	 * @return beamPAWasFixed as bool
 	 * @throw IllegalAccessException If beamPAWasFixed does not exist.
 	 */
 	bool CalPointingRow::getBeamPAWasFixed() const  {
		if (!beamPAWasFixedExists) {
			throw IllegalAccessException("beamPAWasFixed", "CalPointing");
		}
	
  		return beamPAWasFixed;
 	}

 	/**
 	 * Set beamPAWasFixed with the specified bool.
 	 * @param beamPAWasFixed The bool value to which beamPAWasFixed is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamPAWasFixed (bool beamPAWasFixed) {
	
 		this->beamPAWasFixed = beamPAWasFixed;
	
		beamPAWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark beamPAWasFixed, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamPAWasFixed () {
		beamPAWasFixedExists = false;
	}
	

	
	/**
	 * The attribute beamWidth is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidth attribute exists. 
	 */
	bool CalPointingRow::isBeamWidthExists() const {
		return beamWidthExists;
	}
	

	
 	/**
 	 * Get beamWidth, which is optional.
 	 * @return beamWidth as vector<vector<Angle > >
 	 * @throw IllegalAccessException If beamWidth does not exist.
 	 */
 	vector<vector<Angle > > CalPointingRow::getBeamWidth() const  {
		if (!beamWidthExists) {
			throw IllegalAccessException("beamWidth", "CalPointing");
		}
	
  		return beamWidth;
 	}

 	/**
 	 * Set beamWidth with the specified vector<vector<Angle > >.
 	 * @param beamWidth The vector<vector<Angle > > value to which beamWidth is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamWidth (vector<vector<Angle > > beamWidth) {
	
 		this->beamWidth = beamWidth;
	
		beamWidthExists = true;
	
 	}
	
	
	/**
	 * Mark beamWidth, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamWidth () {
		beamWidthExists = false;
	}
	

	
	/**
	 * The attribute beamWidthError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthError attribute exists. 
	 */
	bool CalPointingRow::isBeamWidthErrorExists() const {
		return beamWidthErrorExists;
	}
	

	
 	/**
 	 * Get beamWidthError, which is optional.
 	 * @return beamWidthError as vector<vector<Angle > >
 	 * @throw IllegalAccessException If beamWidthError does not exist.
 	 */
 	vector<vector<Angle > > CalPointingRow::getBeamWidthError() const  {
		if (!beamWidthErrorExists) {
			throw IllegalAccessException("beamWidthError", "CalPointing");
		}
	
  		return beamWidthError;
 	}

 	/**
 	 * Set beamWidthError with the specified vector<vector<Angle > >.
 	 * @param beamWidthError The vector<vector<Angle > > value to which beamWidthError is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamWidthError (vector<vector<Angle > > beamWidthError) {
	
 		this->beamWidthError = beamWidthError;
	
		beamWidthErrorExists = true;
	
 	}
	
	
	/**
	 * Mark beamWidthError, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamWidthError () {
		beamWidthErrorExists = false;
	}
	

	
	/**
	 * The attribute beamWidthWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthWasFixed attribute exists. 
	 */
	bool CalPointingRow::isBeamWidthWasFixedExists() const {
		return beamWidthWasFixedExists;
	}
	

	
 	/**
 	 * Get beamWidthWasFixed, which is optional.
 	 * @return beamWidthWasFixed as vector<bool >
 	 * @throw IllegalAccessException If beamWidthWasFixed does not exist.
 	 */
 	vector<bool > CalPointingRow::getBeamWidthWasFixed() const  {
		if (!beamWidthWasFixedExists) {
			throw IllegalAccessException("beamWidthWasFixed", "CalPointing");
		}
	
  		return beamWidthWasFixed;
 	}

 	/**
 	 * Set beamWidthWasFixed with the specified vector<bool >.
 	 * @param beamWidthWasFixed The vector<bool > value to which beamWidthWasFixed is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamWidthWasFixed (vector<bool > beamWidthWasFixed) {
	
 		this->beamWidthWasFixed = beamWidthWasFixed;
	
		beamWidthWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark beamWidthWasFixed, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamWidthWasFixed () {
		beamWidthWasFixedExists = false;
	}
	

	
	/**
	 * The attribute offIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensity attribute exists. 
	 */
	bool CalPointingRow::isOffIntensityExists() const {
		return offIntensityExists;
	}
	

	
 	/**
 	 * Get offIntensity, which is optional.
 	 * @return offIntensity as vector<Temperature >
 	 * @throw IllegalAccessException If offIntensity does not exist.
 	 */
 	vector<Temperature > CalPointingRow::getOffIntensity() const  {
		if (!offIntensityExists) {
			throw IllegalAccessException("offIntensity", "CalPointing");
		}
	
  		return offIntensity;
 	}

 	/**
 	 * Set offIntensity with the specified vector<Temperature >.
 	 * @param offIntensity The vector<Temperature > value to which offIntensity is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setOffIntensity (vector<Temperature > offIntensity) {
	
 		this->offIntensity = offIntensity;
	
		offIntensityExists = true;
	
 	}
	
	
	/**
	 * Mark offIntensity, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearOffIntensity () {
		offIntensityExists = false;
	}
	

	
	/**
	 * The attribute offIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensityError attribute exists. 
	 */
	bool CalPointingRow::isOffIntensityErrorExists() const {
		return offIntensityErrorExists;
	}
	

	
 	/**
 	 * Get offIntensityError, which is optional.
 	 * @return offIntensityError as vector<Temperature >
 	 * @throw IllegalAccessException If offIntensityError does not exist.
 	 */
 	vector<Temperature > CalPointingRow::getOffIntensityError() const  {
		if (!offIntensityErrorExists) {
			throw IllegalAccessException("offIntensityError", "CalPointing");
		}
	
  		return offIntensityError;
 	}

 	/**
 	 * Set offIntensityError with the specified vector<Temperature >.
 	 * @param offIntensityError The vector<Temperature > value to which offIntensityError is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setOffIntensityError (vector<Temperature > offIntensityError) {
	
 		this->offIntensityError = offIntensityError;
	
		offIntensityErrorExists = true;
	
 	}
	
	
	/**
	 * Mark offIntensityError, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearOffIntensityError () {
		offIntensityErrorExists = false;
	}
	

	
	/**
	 * The attribute offIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensityWasFixed attribute exists. 
	 */
	bool CalPointingRow::isOffIntensityWasFixedExists() const {
		return offIntensityWasFixedExists;
	}
	

	
 	/**
 	 * Get offIntensityWasFixed, which is optional.
 	 * @return offIntensityWasFixed as bool
 	 * @throw IllegalAccessException If offIntensityWasFixed does not exist.
 	 */
 	bool CalPointingRow::getOffIntensityWasFixed() const  {
		if (!offIntensityWasFixedExists) {
			throw IllegalAccessException("offIntensityWasFixed", "CalPointing");
		}
	
  		return offIntensityWasFixed;
 	}

 	/**
 	 * Set offIntensityWasFixed with the specified bool.
 	 * @param offIntensityWasFixed The bool value to which offIntensityWasFixed is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setOffIntensityWasFixed (bool offIntensityWasFixed) {
	
 		this->offIntensityWasFixed = offIntensityWasFixed;
	
		offIntensityWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark offIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearOffIntensityWasFixed () {
		offIntensityWasFixedExists = false;
	}
	

	
	/**
	 * The attribute peakIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensity attribute exists. 
	 */
	bool CalPointingRow::isPeakIntensityExists() const {
		return peakIntensityExists;
	}
	

	
 	/**
 	 * Get peakIntensity, which is optional.
 	 * @return peakIntensity as vector<Temperature >
 	 * @throw IllegalAccessException If peakIntensity does not exist.
 	 */
 	vector<Temperature > CalPointingRow::getPeakIntensity() const  {
		if (!peakIntensityExists) {
			throw IllegalAccessException("peakIntensity", "CalPointing");
		}
	
  		return peakIntensity;
 	}

 	/**
 	 * Set peakIntensity with the specified vector<Temperature >.
 	 * @param peakIntensity The vector<Temperature > value to which peakIntensity is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setPeakIntensity (vector<Temperature > peakIntensity) {
	
 		this->peakIntensity = peakIntensity;
	
		peakIntensityExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensity, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearPeakIntensity () {
		peakIntensityExists = false;
	}
	

	
	/**
	 * The attribute peakIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityError attribute exists. 
	 */
	bool CalPointingRow::isPeakIntensityErrorExists() const {
		return peakIntensityErrorExists;
	}
	

	
 	/**
 	 * Get peakIntensityError, which is optional.
 	 * @return peakIntensityError as vector<Temperature >
 	 * @throw IllegalAccessException If peakIntensityError does not exist.
 	 */
 	vector<Temperature > CalPointingRow::getPeakIntensityError() const  {
		if (!peakIntensityErrorExists) {
			throw IllegalAccessException("peakIntensityError", "CalPointing");
		}
	
  		return peakIntensityError;
 	}

 	/**
 	 * Set peakIntensityError with the specified vector<Temperature >.
 	 * @param peakIntensityError The vector<Temperature > value to which peakIntensityError is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setPeakIntensityError (vector<Temperature > peakIntensityError) {
	
 		this->peakIntensityError = peakIntensityError;
	
		peakIntensityErrorExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensityError, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearPeakIntensityError () {
		peakIntensityErrorExists = false;
	}
	

	
	/**
	 * The attribute peakIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityWasFixed attribute exists. 
	 */
	bool CalPointingRow::isPeakIntensityWasFixedExists() const {
		return peakIntensityWasFixedExists;
	}
	

	
 	/**
 	 * Get peakIntensityWasFixed, which is optional.
 	 * @return peakIntensityWasFixed as bool
 	 * @throw IllegalAccessException If peakIntensityWasFixed does not exist.
 	 */
 	bool CalPointingRow::getPeakIntensityWasFixed() const  {
		if (!peakIntensityWasFixedExists) {
			throw IllegalAccessException("peakIntensityWasFixed", "CalPointing");
		}
	
  		return peakIntensityWasFixed;
 	}

 	/**
 	 * Set peakIntensityWasFixed with the specified bool.
 	 * @param peakIntensityWasFixed The bool value to which peakIntensityWasFixed is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setPeakIntensityWasFixed (bool peakIntensityWasFixed) {
	
 		this->peakIntensityWasFixed = peakIntensityWasFixed;
	
		peakIntensityWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearPeakIntensityWasFixed () {
		peakIntensityWasFixedExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalPointingRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalPointing");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalPointingRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalPointing");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalPointingRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalPointingRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalPointingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPointingRow::CalPointingRow (CalPointingTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		averagedPolarizationsExists = false;
	

	
		beamPAExists = false;
	

	
		beamPAErrorExists = false;
	

	
		beamPAWasFixedExists = false;
	

	
		beamWidthExists = false;
	

	
		beamWidthErrorExists = false;
	

	
		beamWidthWasFixedExists = false;
	

	
		offIntensityExists = false;
	

	
		offIntensityErrorExists = false;
	

	
		offIntensityWasFixedExists = false;
	

	
		peakIntensityExists = false;
	

	
		peakIntensityErrorExists = false;
	

	
		peakIntensityWasFixedExists = false;
	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaMake = CAntennaMake::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
atmPhaseCorrection = CAtmPhaseCorrection::from_int(0);
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
pointingModelMode = CPointingModelMode::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
pointingMethod = CPointingMethod::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaName"] = &CalPointingRow::antennaNameFromBin; 
	 fromBinMethods["receiverBand"] = &CalPointingRow::receiverBandFromBin; 
	 fromBinMethods["calDataId"] = &CalPointingRow::calDataIdFromBin; 
	 fromBinMethods["calReductionId"] = &CalPointingRow::calReductionIdFromBin; 
	 fromBinMethods["startValidTime"] = &CalPointingRow::startValidTimeFromBin; 
	 fromBinMethods["endValidTime"] = &CalPointingRow::endValidTimeFromBin; 
	 fromBinMethods["ambientTemperature"] = &CalPointingRow::ambientTemperatureFromBin; 
	 fromBinMethods["antennaMake"] = &CalPointingRow::antennaMakeFromBin; 
	 fromBinMethods["atmPhaseCorrection"] = &CalPointingRow::atmPhaseCorrectionFromBin; 
	 fromBinMethods["direction"] = &CalPointingRow::directionFromBin; 
	 fromBinMethods["frequencyRange"] = &CalPointingRow::frequencyRangeFromBin; 
	 fromBinMethods["pointingModelMode"] = &CalPointingRow::pointingModelModeFromBin; 
	 fromBinMethods["pointingMethod"] = &CalPointingRow::pointingMethodFromBin; 
	 fromBinMethods["numReceptor"] = &CalPointingRow::numReceptorFromBin; 
	 fromBinMethods["polarizationTypes"] = &CalPointingRow::polarizationTypesFromBin; 
	 fromBinMethods["collOffsetRelative"] = &CalPointingRow::collOffsetRelativeFromBin; 
	 fromBinMethods["collOffsetAbsolute"] = &CalPointingRow::collOffsetAbsoluteFromBin; 
	 fromBinMethods["collError"] = &CalPointingRow::collErrorFromBin; 
	 fromBinMethods["collOffsetTied"] = &CalPointingRow::collOffsetTiedFromBin; 
	 fromBinMethods["reducedChiSquared"] = &CalPointingRow::reducedChiSquaredFromBin; 
		
	
	 fromBinMethods["averagedPolarizations"] = &CalPointingRow::averagedPolarizationsFromBin; 
	 fromBinMethods["beamPA"] = &CalPointingRow::beamPAFromBin; 
	 fromBinMethods["beamPAError"] = &CalPointingRow::beamPAErrorFromBin; 
	 fromBinMethods["beamPAWasFixed"] = &CalPointingRow::beamPAWasFixedFromBin; 
	 fromBinMethods["beamWidth"] = &CalPointingRow::beamWidthFromBin; 
	 fromBinMethods["beamWidthError"] = &CalPointingRow::beamWidthErrorFromBin; 
	 fromBinMethods["beamWidthWasFixed"] = &CalPointingRow::beamWidthWasFixedFromBin; 
	 fromBinMethods["offIntensity"] = &CalPointingRow::offIntensityFromBin; 
	 fromBinMethods["offIntensityError"] = &CalPointingRow::offIntensityErrorFromBin; 
	 fromBinMethods["offIntensityWasFixed"] = &CalPointingRow::offIntensityWasFixedFromBin; 
	 fromBinMethods["peakIntensity"] = &CalPointingRow::peakIntensityFromBin; 
	 fromBinMethods["peakIntensityError"] = &CalPointingRow::peakIntensityErrorFromBin; 
	 fromBinMethods["peakIntensityWasFixed"] = &CalPointingRow::peakIntensityWasFixedFromBin; 
	
	}
	
	CalPointingRow::CalPointingRow (CalPointingTable &t, CalPointingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		averagedPolarizationsExists = false;
	

	
		beamPAExists = false;
	

	
		beamPAErrorExists = false;
	

	
		beamPAWasFixedExists = false;
	

	
		beamWidthExists = false;
	

	
		beamWidthErrorExists = false;
	

	
		beamWidthWasFixedExists = false;
	

	
		offIntensityExists = false;
	

	
		offIntensityErrorExists = false;
	

	
		offIntensityWasFixedExists = false;
	

	
		peakIntensityExists = false;
	

	
		peakIntensityErrorExists = false;
	

	
		peakIntensityWasFixedExists = false;
	

	
	

	
		
		}
		else {
	
		
			antennaName = row.antennaName;
		
			receiverBand = row.receiverBand;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			ambientTemperature = row.ambientTemperature;
		
			antennaMake = row.antennaMake;
		
			atmPhaseCorrection = row.atmPhaseCorrection;
		
			direction = row.direction;
		
			frequencyRange = row.frequencyRange;
		
			pointingModelMode = row.pointingModelMode;
		
			pointingMethod = row.pointingMethod;
		
			numReceptor = row.numReceptor;
		
			polarizationTypes = row.polarizationTypes;
		
			collOffsetRelative = row.collOffsetRelative;
		
			collOffsetAbsolute = row.collOffsetAbsolute;
		
			collError = row.collError;
		
			collOffsetTied = row.collOffsetTied;
		
			reducedChiSquared = row.reducedChiSquared;
		
		
		
		
		if (row.averagedPolarizationsExists) {
			averagedPolarizations = row.averagedPolarizations;		
			averagedPolarizationsExists = true;
		}
		else
			averagedPolarizationsExists = false;
		
		if (row.beamPAExists) {
			beamPA = row.beamPA;		
			beamPAExists = true;
		}
		else
			beamPAExists = false;
		
		if (row.beamPAErrorExists) {
			beamPAError = row.beamPAError;		
			beamPAErrorExists = true;
		}
		else
			beamPAErrorExists = false;
		
		if (row.beamPAWasFixedExists) {
			beamPAWasFixed = row.beamPAWasFixed;		
			beamPAWasFixedExists = true;
		}
		else
			beamPAWasFixedExists = false;
		
		if (row.beamWidthExists) {
			beamWidth = row.beamWidth;		
			beamWidthExists = true;
		}
		else
			beamWidthExists = false;
		
		if (row.beamWidthErrorExists) {
			beamWidthError = row.beamWidthError;		
			beamWidthErrorExists = true;
		}
		else
			beamWidthErrorExists = false;
		
		if (row.beamWidthWasFixedExists) {
			beamWidthWasFixed = row.beamWidthWasFixed;		
			beamWidthWasFixedExists = true;
		}
		else
			beamWidthWasFixedExists = false;
		
		if (row.offIntensityExists) {
			offIntensity = row.offIntensity;		
			offIntensityExists = true;
		}
		else
			offIntensityExists = false;
		
		if (row.offIntensityErrorExists) {
			offIntensityError = row.offIntensityError;		
			offIntensityErrorExists = true;
		}
		else
			offIntensityErrorExists = false;
		
		if (row.offIntensityWasFixedExists) {
			offIntensityWasFixed = row.offIntensityWasFixed;		
			offIntensityWasFixedExists = true;
		}
		else
			offIntensityWasFixedExists = false;
		
		if (row.peakIntensityExists) {
			peakIntensity = row.peakIntensity;		
			peakIntensityExists = true;
		}
		else
			peakIntensityExists = false;
		
		if (row.peakIntensityErrorExists) {
			peakIntensityError = row.peakIntensityError;		
			peakIntensityErrorExists = true;
		}
		else
			peakIntensityErrorExists = false;
		
		if (row.peakIntensityWasFixedExists) {
			peakIntensityWasFixed = row.peakIntensityWasFixed;		
			peakIntensityWasFixedExists = true;
		}
		else
			peakIntensityWasFixedExists = false;
		
		}
		
		 fromBinMethods["antennaName"] = &CalPointingRow::antennaNameFromBin; 
		 fromBinMethods["receiverBand"] = &CalPointingRow::receiverBandFromBin; 
		 fromBinMethods["calDataId"] = &CalPointingRow::calDataIdFromBin; 
		 fromBinMethods["calReductionId"] = &CalPointingRow::calReductionIdFromBin; 
		 fromBinMethods["startValidTime"] = &CalPointingRow::startValidTimeFromBin; 
		 fromBinMethods["endValidTime"] = &CalPointingRow::endValidTimeFromBin; 
		 fromBinMethods["ambientTemperature"] = &CalPointingRow::ambientTemperatureFromBin; 
		 fromBinMethods["antennaMake"] = &CalPointingRow::antennaMakeFromBin; 
		 fromBinMethods["atmPhaseCorrection"] = &CalPointingRow::atmPhaseCorrectionFromBin; 
		 fromBinMethods["direction"] = &CalPointingRow::directionFromBin; 
		 fromBinMethods["frequencyRange"] = &CalPointingRow::frequencyRangeFromBin; 
		 fromBinMethods["pointingModelMode"] = &CalPointingRow::pointingModelModeFromBin; 
		 fromBinMethods["pointingMethod"] = &CalPointingRow::pointingMethodFromBin; 
		 fromBinMethods["numReceptor"] = &CalPointingRow::numReceptorFromBin; 
		 fromBinMethods["polarizationTypes"] = &CalPointingRow::polarizationTypesFromBin; 
		 fromBinMethods["collOffsetRelative"] = &CalPointingRow::collOffsetRelativeFromBin; 
		 fromBinMethods["collOffsetAbsolute"] = &CalPointingRow::collOffsetAbsoluteFromBin; 
		 fromBinMethods["collError"] = &CalPointingRow::collErrorFromBin; 
		 fromBinMethods["collOffsetTied"] = &CalPointingRow::collOffsetTiedFromBin; 
		 fromBinMethods["reducedChiSquared"] = &CalPointingRow::reducedChiSquaredFromBin; 
			
	
		 fromBinMethods["averagedPolarizations"] = &CalPointingRow::averagedPolarizationsFromBin; 
		 fromBinMethods["beamPA"] = &CalPointingRow::beamPAFromBin; 
		 fromBinMethods["beamPAError"] = &CalPointingRow::beamPAErrorFromBin; 
		 fromBinMethods["beamPAWasFixed"] = &CalPointingRow::beamPAWasFixedFromBin; 
		 fromBinMethods["beamWidth"] = &CalPointingRow::beamWidthFromBin; 
		 fromBinMethods["beamWidthError"] = &CalPointingRow::beamWidthErrorFromBin; 
		 fromBinMethods["beamWidthWasFixed"] = &CalPointingRow::beamWidthWasFixedFromBin; 
		 fromBinMethods["offIntensity"] = &CalPointingRow::offIntensityFromBin; 
		 fromBinMethods["offIntensityError"] = &CalPointingRow::offIntensityErrorFromBin; 
		 fromBinMethods["offIntensityWasFixed"] = &CalPointingRow::offIntensityWasFixedFromBin; 
		 fromBinMethods["peakIntensity"] = &CalPointingRow::peakIntensityFromBin; 
		 fromBinMethods["peakIntensityError"] = &CalPointingRow::peakIntensityErrorFromBin; 
		 fromBinMethods["peakIntensityWasFixed"] = &CalPointingRow::peakIntensityWasFixedFromBin; 
			
	}

	
	bool CalPointingRow::compareNoAutoInc(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AntennaMakeMod::AntennaMake antennaMake, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, vector<Angle > direction, vector<Frequency > frequencyRange, PointingModelModeMod::PointingModelMode pointingModelMode, PointingMethodMod::PointingMethod pointingMethod, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Angle > > collOffsetRelative, vector<vector<Angle > > collOffsetAbsolute, vector<vector<Angle > > collError, vector<vector<bool > > collOffsetTied, vector<double > reducedChiSquared) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->ambientTemperature == ambientTemperature);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrection == atmPhaseCorrection);
		
		if (!result) return false;
	

	
		
		result = result && (this->direction == direction);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingModelMode == pointingModelMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingMethod == pointingMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->collOffsetRelative == collOffsetRelative);
		
		if (!result) return false;
	

	
		
		result = result && (this->collOffsetAbsolute == collOffsetAbsolute);
		
		if (!result) return false;
	

	
		
		result = result && (this->collError == collError);
		
		if (!result) return false;
	

	
		
		result = result && (this->collOffsetTied == collOffsetTied);
		
		if (!result) return false;
	

	
		
		result = result && (this->reducedChiSquared == reducedChiSquared);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalPointingRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AntennaMakeMod::AntennaMake antennaMake, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, vector<Angle > direction, vector<Frequency > frequencyRange, PointingModelModeMod::PointingModelMode pointingModelMode, PointingMethodMod::PointingMethod pointingMethod, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Angle > > collOffsetRelative, vector<vector<Angle > > collOffsetAbsolute, vector<vector<Angle > > collError, vector<vector<bool > > collOffsetTied, vector<double > reducedChiSquared) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->ambientTemperature == ambientTemperature)) return false;
	

	
		if (!(this->antennaMake == antennaMake)) return false;
	

	
		if (!(this->atmPhaseCorrection == atmPhaseCorrection)) return false;
	

	
		if (!(this->direction == direction)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->pointingModelMode == pointingModelMode)) return false;
	

	
		if (!(this->pointingMethod == pointingMethod)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->collOffsetRelative == collOffsetRelative)) return false;
	

	
		if (!(this->collOffsetAbsolute == collOffsetAbsolute)) return false;
	

	
		if (!(this->collError == collError)) return false;
	

	
		if (!(this->collOffsetTied == collOffsetTied)) return false;
	

	
		if (!(this->reducedChiSquared == reducedChiSquared)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPointingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalPointingRow::equalByRequiredValue(CalPointingRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->ambientTemperature != x->ambientTemperature) return false;
			
		if (this->antennaMake != x->antennaMake) return false;
			
		if (this->atmPhaseCorrection != x->atmPhaseCorrection) return false;
			
		if (this->direction != x->direction) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->pointingModelMode != x->pointingModelMode) return false;
			
		if (this->pointingMethod != x->pointingMethod) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->collOffsetRelative != x->collOffsetRelative) return false;
			
		if (this->collOffsetAbsolute != x->collOffsetAbsolute) return false;
			
		if (this->collError != x->collError) return false;
			
		if (this->collOffsetTied != x->collOffsetTied) return false;
			
		if (this->reducedChiSquared != x->reducedChiSquared) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalPointingAttributeFromBin> CalPointingRow::initFromBinMethods() {
		map<string, CalPointingAttributeFromBin> result;
		
		result["antennaName"] = &CalPointingRow::antennaNameFromBin;
		result["receiverBand"] = &CalPointingRow::receiverBandFromBin;
		result["calDataId"] = &CalPointingRow::calDataIdFromBin;
		result["calReductionId"] = &CalPointingRow::calReductionIdFromBin;
		result["startValidTime"] = &CalPointingRow::startValidTimeFromBin;
		result["endValidTime"] = &CalPointingRow::endValidTimeFromBin;
		result["ambientTemperature"] = &CalPointingRow::ambientTemperatureFromBin;
		result["antennaMake"] = &CalPointingRow::antennaMakeFromBin;
		result["atmPhaseCorrection"] = &CalPointingRow::atmPhaseCorrectionFromBin;
		result["direction"] = &CalPointingRow::directionFromBin;
		result["frequencyRange"] = &CalPointingRow::frequencyRangeFromBin;
		result["pointingModelMode"] = &CalPointingRow::pointingModelModeFromBin;
		result["pointingMethod"] = &CalPointingRow::pointingMethodFromBin;
		result["numReceptor"] = &CalPointingRow::numReceptorFromBin;
		result["polarizationTypes"] = &CalPointingRow::polarizationTypesFromBin;
		result["collOffsetRelative"] = &CalPointingRow::collOffsetRelativeFromBin;
		result["collOffsetAbsolute"] = &CalPointingRow::collOffsetAbsoluteFromBin;
		result["collError"] = &CalPointingRow::collErrorFromBin;
		result["collOffsetTied"] = &CalPointingRow::collOffsetTiedFromBin;
		result["reducedChiSquared"] = &CalPointingRow::reducedChiSquaredFromBin;
		
		
		result["averagedPolarizations"] = &CalPointingRow::averagedPolarizationsFromBin;
		result["beamPA"] = &CalPointingRow::beamPAFromBin;
		result["beamPAError"] = &CalPointingRow::beamPAErrorFromBin;
		result["beamPAWasFixed"] = &CalPointingRow::beamPAWasFixedFromBin;
		result["beamWidth"] = &CalPointingRow::beamWidthFromBin;
		result["beamWidthError"] = &CalPointingRow::beamWidthErrorFromBin;
		result["beamWidthWasFixed"] = &CalPointingRow::beamWidthWasFixedFromBin;
		result["offIntensity"] = &CalPointingRow::offIntensityFromBin;
		result["offIntensityError"] = &CalPointingRow::offIntensityErrorFromBin;
		result["offIntensityWasFixed"] = &CalPointingRow::offIntensityWasFixedFromBin;
		result["peakIntensity"] = &CalPointingRow::peakIntensityFromBin;
		result["peakIntensityError"] = &CalPointingRow::peakIntensityErrorFromBin;
		result["peakIntensityWasFixed"] = &CalPointingRow::peakIntensityWasFixedFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
