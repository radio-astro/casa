
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
 * File CalFocusRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalFocusRow.h>
#include <CalFocusTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalFocusRow;
using asdm::CalFocusTable;

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
	CalFocusRow::~CalFocusRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalFocusTable &CalFocusRow::getTable() const {
		return table;
	}

	bool CalFocusRow::isAdded() const {
		return hasBeenAdded;
	}	

	void CalFocusRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalFocusRowIDL struct.
	 */
	CalFocusRowIDL *CalFocusRow::toIDL() const {
		CalFocusRowIDL *x = new CalFocusRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->ambientTemperature = ambientTemperature.toIDLTemperature();
			
		
	

	
  		
		
		
			
				
		x->atmPhaseCorrection = atmPhaseCorrection;
 				
 			
		
	

	
  		
		
		
			
				
		x->focusMethod = focusMethod;
 				
 			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->pointingDirection.length(pointingDirection.size());
		for (unsigned int i = 0; i < pointingDirection.size(); ++i) {
			
			x->pointingDirection[i] = pointingDirection.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->wereFixed.length(wereFixed.size());
		for (unsigned int i = 0; i < wereFixed.size(); ++i) {
			
				
			x->wereFixed[i] = wereFixed.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->offset.length(offset.size());
		for (unsigned int i = 0; i < offset.size(); i++) {
			x->offset[i].length(offset.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < offset.size() ; i++)
			for (unsigned int j = 0; j < offset.at(i).size(); j++)
					
				x->offset[i][j]= offset.at(i).at(j).toIDLLength();
									
		
			
		
	

	
  		
		
		
			
		x->offsetError.length(offsetError.size());
		for (unsigned int i = 0; i < offsetError.size(); i++) {
			x->offsetError[i].length(offsetError.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < offsetError.size() ; i++)
			for (unsigned int j = 0; j < offsetError.at(i).size(); j++)
					
				x->offsetError[i][j]= offsetError.at(i).at(j).toIDLLength();
									
		
			
		
	

	
  		
		
		
			
		x->offsetWasTied.length(offsetWasTied.size());
		for (unsigned int i = 0; i < offsetWasTied.size(); i++) {
			x->offsetWasTied[i].length(offsetWasTied.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < offsetWasTied.size() ; i++)
			for (unsigned int j = 0; j < offsetWasTied.at(i).size(); j++)
					
						
				x->offsetWasTied[i][j] = offsetWasTied.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->reducedChiSquared.length(reducedChiSquared.size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); i++) {
			x->reducedChiSquared[i].length(reducedChiSquared.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < reducedChiSquared.size() ; i++)
			for (unsigned int j = 0; j < reducedChiSquared.at(i).size(); j++)
					
						
				x->reducedChiSquared[i][j] = reducedChiSquared.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->polarizationsAveragedExists = polarizationsAveragedExists;
		
		
			
				
		x->polarizationsAveraged = polarizationsAveraged;
 				
 			
		
	

	
  		
		
		x->focusCurveWidthExists = focusCurveWidthExists;
		
		
			
		x->focusCurveWidth.length(focusCurveWidth.size());
		for (unsigned int i = 0; i < focusCurveWidth.size(); i++) {
			x->focusCurveWidth[i].length(focusCurveWidth.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < focusCurveWidth.size() ; i++)
			for (unsigned int j = 0; j < focusCurveWidth.at(i).size(); j++)
					
				x->focusCurveWidth[i][j]= focusCurveWidth.at(i).at(j).toIDLLength();
									
		
			
		
	

	
  		
		
		x->focusCurveWidthErrorExists = focusCurveWidthErrorExists;
		
		
			
		x->focusCurveWidthError.length(focusCurveWidthError.size());
		for (unsigned int i = 0; i < focusCurveWidthError.size(); i++) {
			x->focusCurveWidthError[i].length(focusCurveWidthError.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < focusCurveWidthError.size() ; i++)
			for (unsigned int j = 0; j < focusCurveWidthError.at(i).size(); j++)
					
				x->focusCurveWidthError[i][j]= focusCurveWidthError.at(i).at(j).toIDLLength();
									
		
			
		
	

	
  		
		
		x->focusCurveWasFixedExists = focusCurveWasFixedExists;
		
		
			
		x->focusCurveWasFixed.length(focusCurveWasFixed.size());
		for (unsigned int i = 0; i < focusCurveWasFixed.size(); ++i) {
			
				
			x->focusCurveWasFixed[i] = focusCurveWasFixed.at(i);
	 			
	 		
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
	 * Fill the values of this row from the IDL struct CalFocusRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalFocusRow::setFromIDL (CalFocusRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setAmbientTemperature(Temperature (x.ambientTemperature));
			
 		
		
	

	
		
		
			
		setAtmPhaseCorrection(x.atmPhaseCorrection);
  			
 		
		
	

	
		
		
			
		setFocusMethod(x.focusMethod);
  			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		pointingDirection .clear();
		for (unsigned int i = 0; i <x.pointingDirection.length(); ++i) {
			
			pointingDirection.push_back(Angle (x.pointingDirection[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		wereFixed .clear();
		for (unsigned int i = 0; i <x.wereFixed.length(); ++i) {
			
			wereFixed.push_back(x.wereFixed[i]);
  			
		}
			
  		
		
	

	
		
		
			
		offset .clear();
		vector<Length> v_aux_offset;
		for (unsigned int i = 0; i < x.offset.length(); ++i) {
			v_aux_offset.clear();
			for (unsigned int j = 0; j < x.offset[0].length(); ++j) {
				
				v_aux_offset.push_back(Length (x.offset[i][j]));
				
  			}
  			offset.push_back(v_aux_offset);			
		}
			
  		
		
	

	
		
		
			
		offsetError .clear();
		vector<Length> v_aux_offsetError;
		for (unsigned int i = 0; i < x.offsetError.length(); ++i) {
			v_aux_offsetError.clear();
			for (unsigned int j = 0; j < x.offsetError[0].length(); ++j) {
				
				v_aux_offsetError.push_back(Length (x.offsetError[i][j]));
				
  			}
  			offsetError.push_back(v_aux_offsetError);			
		}
			
  		
		
	

	
		
		
			
		offsetWasTied .clear();
		vector<bool> v_aux_offsetWasTied;
		for (unsigned int i = 0; i < x.offsetWasTied.length(); ++i) {
			v_aux_offsetWasTied.clear();
			for (unsigned int j = 0; j < x.offsetWasTied[0].length(); ++j) {
				
				v_aux_offsetWasTied.push_back(x.offsetWasTied[i][j]);
	  			
  			}
  			offsetWasTied.push_back(v_aux_offsetWasTied);			
		}
			
  		
		
	

	
		
		
			
		reducedChiSquared .clear();
		vector<double> v_aux_reducedChiSquared;
		for (unsigned int i = 0; i < x.reducedChiSquared.length(); ++i) {
			v_aux_reducedChiSquared.clear();
			for (unsigned int j = 0; j < x.reducedChiSquared[0].length(); ++j) {
				
				v_aux_reducedChiSquared.push_back(x.reducedChiSquared[i][j]);
	  			
  			}
  			reducedChiSquared.push_back(v_aux_reducedChiSquared);			
		}
			
  		
		
	

	
		
		polarizationsAveragedExists = x.polarizationsAveragedExists;
		if (x.polarizationsAveragedExists) {
		
		
			
		setPolarizationsAveraged(x.polarizationsAveraged);
  			
 		
		
		}
		
	

	
		
		focusCurveWidthExists = x.focusCurveWidthExists;
		if (x.focusCurveWidthExists) {
		
		
			
		focusCurveWidth .clear();
		vector<Length> v_aux_focusCurveWidth;
		for (unsigned int i = 0; i < x.focusCurveWidth.length(); ++i) {
			v_aux_focusCurveWidth.clear();
			for (unsigned int j = 0; j < x.focusCurveWidth[0].length(); ++j) {
				
				v_aux_focusCurveWidth.push_back(Length (x.focusCurveWidth[i][j]));
				
  			}
  			focusCurveWidth.push_back(v_aux_focusCurveWidth);			
		}
			
  		
		
		}
		
	

	
		
		focusCurveWidthErrorExists = x.focusCurveWidthErrorExists;
		if (x.focusCurveWidthErrorExists) {
		
		
			
		focusCurveWidthError .clear();
		vector<Length> v_aux_focusCurveWidthError;
		for (unsigned int i = 0; i < x.focusCurveWidthError.length(); ++i) {
			v_aux_focusCurveWidthError.clear();
			for (unsigned int j = 0; j < x.focusCurveWidthError[0].length(); ++j) {
				
				v_aux_focusCurveWidthError.push_back(Length (x.focusCurveWidthError[i][j]));
				
  			}
  			focusCurveWidthError.push_back(v_aux_focusCurveWidthError);			
		}
			
  		
		
		}
		
	

	
		
		focusCurveWasFixedExists = x.focusCurveWasFixedExists;
		if (x.focusCurveWasFixedExists) {
		
		
			
		focusCurveWasFixed .clear();
		for (unsigned int i = 0; i <x.focusCurveWasFixed.length(); ++i) {
			
			focusCurveWasFixed.push_back(x.focusCurveWasFixed[i]);
  			
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
			throw ConversionException (err.getMessage(),"CalFocus");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalFocusRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(ambientTemperature, "ambientTemperature", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrection", atmPhaseCorrection));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("focusMethod", focusMethod));
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(pointingDirection, "pointingDirection", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(wereFixed, "wereFixed", buf);
		
		
	

  	
 		
		
		Parser::toXML(offset, "offset", buf);
		
		
	

  	
 		
		
		Parser::toXML(offsetError, "offsetError", buf);
		
		
	

  	
 		
		
		Parser::toXML(offsetWasTied, "offsetWasTied", buf);
		
		
	

  	
 		
		
		Parser::toXML(reducedChiSquared, "reducedChiSquared", buf);
		
		
	

  	
 		
		if (polarizationsAveragedExists) {
		
		
		Parser::toXML(polarizationsAveraged, "polarizationsAveraged", buf);
		
		
		}
		
	

  	
 		
		if (focusCurveWidthExists) {
		
		
		Parser::toXML(focusCurveWidth, "focusCurveWidth", buf);
		
		
		}
		
	

  	
 		
		if (focusCurveWidthErrorExists) {
		
		
		Parser::toXML(focusCurveWidthError, "focusCurveWidthError", buf);
		
		
		}
		
	

  	
 		
		if (focusCurveWasFixedExists) {
		
		
		Parser::toXML(focusCurveWasFixed, "focusCurveWasFixed", buf);
		
		
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
	void CalFocusRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalFocus",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalFocus",rowDoc));
			
		
	

	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalFocus",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalFocus",rowDoc);
		
		
		
	

	
  		
			
	  	setAmbientTemperature(Parser::getTemperature("ambientTemperature","CalFocus",rowDoc));
			
		
	

	
		
		
		
		atmPhaseCorrection = EnumerationParser::getAtmPhaseCorrection("atmPhaseCorrection","CalFocus",rowDoc);
		
		
		
	

	
		
		
		
		focusMethod = EnumerationParser::getFocusMethod("focusMethod","CalFocus",rowDoc);
		
		
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPointingDirection(Parser::get1DAngle("pointingDirection","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalFocus",rowDoc));
			
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalFocus",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setWereFixed(Parser::get1DBoolean("wereFixed","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setOffset(Parser::get2DLength("offset","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setOffsetError(Parser::get2DLength("offsetError","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setOffsetWasTied(Parser::get2DBoolean("offsetWasTied","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setReducedChiSquared(Parser::get2DDouble("reducedChiSquared","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<polarizationsAveraged>")) {
			
	  		setPolarizationsAveraged(Parser::getBoolean("polarizationsAveraged","CalFocus",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<focusCurveWidth>")) {
			
								
	  		setFocusCurveWidth(Parser::get2DLength("focusCurveWidth","CalFocus",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<focusCurveWidthError>")) {
			
								
	  		setFocusCurveWidthError(Parser::get2DLength("focusCurveWidthError","CalFocus",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<focusCurveWasFixed>")) {
			
								
	  		setFocusCurveWasFixed(Parser::get1DBoolean("focusCurveWasFixed","CalFocus",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<offIntensity>")) {
			
								
	  		setOffIntensity(Parser::get1DTemperature("offIntensity","CalFocus",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<offIntensityError>")) {
			
								
	  		setOffIntensityError(Parser::get1DTemperature("offIntensityError","CalFocus",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<offIntensityWasFixed>")) {
			
	  		setOffIntensityWasFixed(Parser::getBoolean("offIntensityWasFixed","CalFocus",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<peakIntensity>")) {
			
								
	  		setPeakIntensity(Parser::get1DTemperature("peakIntensity","CalFocus",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<peakIntensityError>")) {
			
								
	  		setPeakIntensityError(Parser::get1DTemperature("peakIntensityError","CalFocus",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<peakIntensityWasFixed>")) {
			
	  		setPeakIntensityWasFixed(Parser::getBoolean("peakIntensityWasFixed","CalFocus",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalFocus");
		}
	}
	
	void CalFocusRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeString(antennaName);
				
		
	

	
	
		
					
			eoss.writeInt(receiverBand);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
	ambientTemperature.toBin(eoss);
		
	

	
	
		
					
			eoss.writeInt(atmPhaseCorrection);
				
		
	

	
	
		
					
			eoss.writeInt(focusMethod);
				
		
	

	
	
		
	Frequency::toBin(frequencyRange, eoss);
		
	

	
	
		
	Angle::toBin(pointingDirection, eoss);
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) wereFixed.size());
		for (unsigned int i = 0; i < wereFixed.size(); i++)
				
			eoss.writeBoolean(wereFixed.at(i));
				
				
						
		
	

	
	
		
	Length::toBin(offset, eoss);
		
	

	
	
		
	Length::toBin(offsetError, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) offsetWasTied.size());
		eoss.writeInt((int) offsetWasTied.at(0).size());
		for (unsigned int i = 0; i < offsetWasTied.size(); i++) 
			for (unsigned int j = 0;  j < offsetWasTied.at(0).size(); j++) 
							 
				eoss.writeBoolean(offsetWasTied.at(i).at(j));
				
	
						
		
	

	
	
		
		
			
		eoss.writeInt((int) reducedChiSquared.size());
		eoss.writeInt((int) reducedChiSquared.at(0).size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); i++) 
			for (unsigned int j = 0;  j < reducedChiSquared.at(0).size(); j++) 
							 
				eoss.writeDouble(reducedChiSquared.at(i).at(j));
				
	
						
		
	


	
	
	eoss.writeBoolean(polarizationsAveragedExists);
	if (polarizationsAveragedExists) {
	
	
	
		
						
			eoss.writeBoolean(polarizationsAveraged);
				
		
	

	}

	eoss.writeBoolean(focusCurveWidthExists);
	if (focusCurveWidthExists) {
	
	
	
		
	Length::toBin(focusCurveWidth, eoss);
		
	

	}

	eoss.writeBoolean(focusCurveWidthErrorExists);
	if (focusCurveWidthErrorExists) {
	
	
	
		
	Length::toBin(focusCurveWidthError, eoss);
		
	

	}

	eoss.writeBoolean(focusCurveWasFixedExists);
	if (focusCurveWasFixedExists) {
	
	
	
		
		
			
		eoss.writeInt((int) focusCurveWasFixed.size());
		for (unsigned int i = 0; i < focusCurveWasFixed.size(); i++)
				
			eoss.writeBoolean(focusCurveWasFixed.at(i));
				
				
						
		
	

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
	
void CalFocusRow::antennaNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaName =  eiss.readString();
			
		
	
	
}
void CalFocusRow::receiverBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		receiverBand = CReceiverBand::from_int(eiss.readInt());
			
		
	
	
}
void CalFocusRow::calDataIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calDataId =  Tag::fromBin(eiss);
		
	
	
}
void CalFocusRow::calReductionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calReductionId =  Tag::fromBin(eiss);
		
	
	
}
void CalFocusRow::startValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalFocusRow::endValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalFocusRow::ambientTemperatureFromBin(EndianISStream& eiss) {
		
	
		
		
		ambientTemperature =  Temperature::fromBin(eiss);
		
	
	
}
void CalFocusRow::atmPhaseCorrectionFromBin(EndianISStream& eiss) {
		
	
	
		
			
		atmPhaseCorrection = CAtmPhaseCorrection::from_int(eiss.readInt());
			
		
	
	
}
void CalFocusRow::focusMethodFromBin(EndianISStream& eiss) {
		
	
	
		
			
		focusMethod = CFocusMethod::from_int(eiss.readInt());
			
		
	
	
}
void CalFocusRow::frequencyRangeFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	frequencyRange = Frequency::from1DBin(eiss);	
	

		
	
	
}
void CalFocusRow::pointingDirectionFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	pointingDirection = Angle::from1DBin(eiss);	
	

		
	
	
}
void CalFocusRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void CalFocusRow::polarizationTypesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	
	
}
void CalFocusRow::wereFixedFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		wereFixed.clear();
		
		unsigned int wereFixedDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < wereFixedDim1; i++)
			
			wereFixed.push_back(eiss.readBoolean());
			
	

		
	
	
}
void CalFocusRow::offsetFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	offset = Length::from2DBin(eiss);		
	

		
	
	
}
void CalFocusRow::offsetErrorFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	offsetError = Length::from2DBin(eiss);		
	

		
	
	
}
void CalFocusRow::offsetWasTiedFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		offsetWasTied.clear();
		
		unsigned int offsetWasTiedDim1 = eiss.readInt();
		unsigned int offsetWasTiedDim2 = eiss.readInt();
		vector <bool> offsetWasTiedAux1;
		for (unsigned int i = 0; i < offsetWasTiedDim1; i++) {
			offsetWasTiedAux1.clear();
			for (unsigned int j = 0; j < offsetWasTiedDim2 ; j++)			
			
			offsetWasTiedAux1.push_back(eiss.readBoolean());
			
			offsetWasTied.push_back(offsetWasTiedAux1);
		}
	
	

		
	
	
}
void CalFocusRow::reducedChiSquaredFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		reducedChiSquared.clear();
		
		unsigned int reducedChiSquaredDim1 = eiss.readInt();
		unsigned int reducedChiSquaredDim2 = eiss.readInt();
		vector <double> reducedChiSquaredAux1;
		for (unsigned int i = 0; i < reducedChiSquaredDim1; i++) {
			reducedChiSquaredAux1.clear();
			for (unsigned int j = 0; j < reducedChiSquaredDim2 ; j++)			
			
			reducedChiSquaredAux1.push_back(eiss.readDouble());
			
			reducedChiSquared.push_back(reducedChiSquaredAux1);
		}
	
	

		
	
	
}

void CalFocusRow::polarizationsAveragedFromBin(EndianISStream& eiss) {
		
	polarizationsAveragedExists = eiss.readBoolean();
	if (polarizationsAveragedExists) {
		
	
	
		
			
		polarizationsAveraged =  eiss.readBoolean();
			
		
	

	}
	
}
void CalFocusRow::focusCurveWidthFromBin(EndianISStream& eiss) {
		
	focusCurveWidthExists = eiss.readBoolean();
	if (focusCurveWidthExists) {
		
	
		
		
			
	
	focusCurveWidth = Length::from2DBin(eiss);		
	

		
	

	}
	
}
void CalFocusRow::focusCurveWidthErrorFromBin(EndianISStream& eiss) {
		
	focusCurveWidthErrorExists = eiss.readBoolean();
	if (focusCurveWidthErrorExists) {
		
	
		
		
			
	
	focusCurveWidthError = Length::from2DBin(eiss);		
	

		
	

	}
	
}
void CalFocusRow::focusCurveWasFixedFromBin(EndianISStream& eiss) {
		
	focusCurveWasFixedExists = eiss.readBoolean();
	if (focusCurveWasFixedExists) {
		
	
	
		
			
	
		focusCurveWasFixed.clear();
		
		unsigned int focusCurveWasFixedDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < focusCurveWasFixedDim1; i++)
			
			focusCurveWasFixed.push_back(eiss.readBoolean());
			
	

		
	

	}
	
}
void CalFocusRow::offIntensityFromBin(EndianISStream& eiss) {
		
	offIntensityExists = eiss.readBoolean();
	if (offIntensityExists) {
		
	
		
		
			
	
	offIntensity = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalFocusRow::offIntensityErrorFromBin(EndianISStream& eiss) {
		
	offIntensityErrorExists = eiss.readBoolean();
	if (offIntensityErrorExists) {
		
	
		
		
			
	
	offIntensityError = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalFocusRow::offIntensityWasFixedFromBin(EndianISStream& eiss) {
		
	offIntensityWasFixedExists = eiss.readBoolean();
	if (offIntensityWasFixedExists) {
		
	
	
		
			
		offIntensityWasFixed =  eiss.readBoolean();
			
		
	

	}
	
}
void CalFocusRow::peakIntensityFromBin(EndianISStream& eiss) {
		
	peakIntensityExists = eiss.readBoolean();
	if (peakIntensityExists) {
		
	
		
		
			
	
	peakIntensity = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalFocusRow::peakIntensityErrorFromBin(EndianISStream& eiss) {
		
	peakIntensityErrorExists = eiss.readBoolean();
	if (peakIntensityErrorExists) {
		
	
		
		
			
	
	peakIntensityError = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void CalFocusRow::peakIntensityWasFixedFromBin(EndianISStream& eiss) {
		
	peakIntensityWasFixedExists = eiss.readBoolean();
	if (peakIntensityWasFixedExists) {
		
	
	
		
			
		peakIntensityWasFixed =  eiss.readBoolean();
			
		
	

	}
	
}
	
	
	CalFocusRow* CalFocusRow::fromBin(EndianISStream& eiss, CalFocusTable& table, const vector<string>& attributesSeq) {
		CalFocusRow* row = new  CalFocusRow(table);
		
		map<string, CalFocusAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalFocusTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalFocusRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalFocusRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalFocusRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalFocus");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalFocusRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalFocus");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature CalFocusRow::getAmbientTemperature() const {
	
  		return ambientTemperature;
 	}

 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setAmbientTemperature (Temperature ambientTemperature)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->ambientTemperature = ambientTemperature;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection CalFocusRow::getAtmPhaseCorrection() const {
	
  		return atmPhaseCorrection;
 	}

 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmPhaseCorrection = atmPhaseCorrection;
	
 	}
	
	

	

	
 	/**
 	 * Get focusMethod.
 	 * @return focusMethod as FocusMethodMod::FocusMethod
 	 */
 	FocusMethodMod::FocusMethod CalFocusRow::getFocusMethod() const {
	
  		return focusMethod;
 	}

 	/**
 	 * Set focusMethod with the specified FocusMethodMod::FocusMethod.
 	 * @param focusMethod The FocusMethodMod::FocusMethod value to which focusMethod is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setFocusMethod (FocusMethodMod::FocusMethod focusMethod)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusMethod = focusMethod;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalFocusRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingDirection.
 	 * @return pointingDirection as vector<Angle >
 	 */
 	vector<Angle > CalFocusRow::getPointingDirection() const {
	
  		return pointingDirection;
 	}

 	/**
 	 * Set pointingDirection with the specified vector<Angle >.
 	 * @param pointingDirection The vector<Angle > value to which pointingDirection is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setPointingDirection (vector<Angle > pointingDirection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingDirection = pointingDirection;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalFocusRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalFocusRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get wereFixed.
 	 * @return wereFixed as vector<bool >
 	 */
 	vector<bool > CalFocusRow::getWereFixed() const {
	
  		return wereFixed;
 	}

 	/**
 	 * Set wereFixed with the specified vector<bool >.
 	 * @param wereFixed The vector<bool > value to which wereFixed is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setWereFixed (vector<bool > wereFixed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->wereFixed = wereFixed;
	
 	}
	
	

	

	
 	/**
 	 * Get offset.
 	 * @return offset as vector<vector<Length > >
 	 */
 	vector<vector<Length > > CalFocusRow::getOffset() const {
	
  		return offset;
 	}

 	/**
 	 * Set offset with the specified vector<vector<Length > >.
 	 * @param offset The vector<vector<Length > > value to which offset is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setOffset (vector<vector<Length > > offset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->offset = offset;
	
 	}
	
	

	

	
 	/**
 	 * Get offsetError.
 	 * @return offsetError as vector<vector<Length > >
 	 */
 	vector<vector<Length > > CalFocusRow::getOffsetError() const {
	
  		return offsetError;
 	}

 	/**
 	 * Set offsetError with the specified vector<vector<Length > >.
 	 * @param offsetError The vector<vector<Length > > value to which offsetError is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setOffsetError (vector<vector<Length > > offsetError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->offsetError = offsetError;
	
 	}
	
	

	

	
 	/**
 	 * Get offsetWasTied.
 	 * @return offsetWasTied as vector<vector<bool > >
 	 */
 	vector<vector<bool > > CalFocusRow::getOffsetWasTied() const {
	
  		return offsetWasTied;
 	}

 	/**
 	 * Set offsetWasTied with the specified vector<vector<bool > >.
 	 * @param offsetWasTied The vector<vector<bool > > value to which offsetWasTied is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setOffsetWasTied (vector<vector<bool > > offsetWasTied)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->offsetWasTied = offsetWasTied;
	
 	}
	
	

	

	
 	/**
 	 * Get reducedChiSquared.
 	 * @return reducedChiSquared as vector<vector<double > >
 	 */
 	vector<vector<double > > CalFocusRow::getReducedChiSquared() const {
	
  		return reducedChiSquared;
 	}

 	/**
 	 * Set reducedChiSquared with the specified vector<vector<double > >.
 	 * @param reducedChiSquared The vector<vector<double > > value to which reducedChiSquared is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setReducedChiSquared (vector<vector<double > > reducedChiSquared)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->reducedChiSquared = reducedChiSquared;
	
 	}
	
	

	
	/**
	 * The attribute polarizationsAveraged is optional. Return true if this attribute exists.
	 * @return true if and only if the polarizationsAveraged attribute exists. 
	 */
	bool CalFocusRow::isPolarizationsAveragedExists() const {
		return polarizationsAveragedExists;
	}
	

	
 	/**
 	 * Get polarizationsAveraged, which is optional.
 	 * @return polarizationsAveraged as bool
 	 * @throw IllegalAccessException If polarizationsAveraged does not exist.
 	 */
 	bool CalFocusRow::getPolarizationsAveraged() const  {
		if (!polarizationsAveragedExists) {
			throw IllegalAccessException("polarizationsAveraged", "CalFocus");
		}
	
  		return polarizationsAveraged;
 	}

 	/**
 	 * Set polarizationsAveraged with the specified bool.
 	 * @param polarizationsAveraged The bool value to which polarizationsAveraged is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setPolarizationsAveraged (bool polarizationsAveraged) {
	
 		this->polarizationsAveraged = polarizationsAveraged;
	
		polarizationsAveragedExists = true;
	
 	}
	
	
	/**
	 * Mark polarizationsAveraged, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearPolarizationsAveraged () {
		polarizationsAveragedExists = false;
	}
	

	
	/**
	 * The attribute focusCurveWidth is optional. Return true if this attribute exists.
	 * @return true if and only if the focusCurveWidth attribute exists. 
	 */
	bool CalFocusRow::isFocusCurveWidthExists() const {
		return focusCurveWidthExists;
	}
	

	
 	/**
 	 * Get focusCurveWidth, which is optional.
 	 * @return focusCurveWidth as vector<vector<Length > >
 	 * @throw IllegalAccessException If focusCurveWidth does not exist.
 	 */
 	vector<vector<Length > > CalFocusRow::getFocusCurveWidth() const  {
		if (!focusCurveWidthExists) {
			throw IllegalAccessException("focusCurveWidth", "CalFocus");
		}
	
  		return focusCurveWidth;
 	}

 	/**
 	 * Set focusCurveWidth with the specified vector<vector<Length > >.
 	 * @param focusCurveWidth The vector<vector<Length > > value to which focusCurveWidth is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setFocusCurveWidth (vector<vector<Length > > focusCurveWidth) {
	
 		this->focusCurveWidth = focusCurveWidth;
	
		focusCurveWidthExists = true;
	
 	}
	
	
	/**
	 * Mark focusCurveWidth, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearFocusCurveWidth () {
		focusCurveWidthExists = false;
	}
	

	
	/**
	 * The attribute focusCurveWidthError is optional. Return true if this attribute exists.
	 * @return true if and only if the focusCurveWidthError attribute exists. 
	 */
	bool CalFocusRow::isFocusCurveWidthErrorExists() const {
		return focusCurveWidthErrorExists;
	}
	

	
 	/**
 	 * Get focusCurveWidthError, which is optional.
 	 * @return focusCurveWidthError as vector<vector<Length > >
 	 * @throw IllegalAccessException If focusCurveWidthError does not exist.
 	 */
 	vector<vector<Length > > CalFocusRow::getFocusCurveWidthError() const  {
		if (!focusCurveWidthErrorExists) {
			throw IllegalAccessException("focusCurveWidthError", "CalFocus");
		}
	
  		return focusCurveWidthError;
 	}

 	/**
 	 * Set focusCurveWidthError with the specified vector<vector<Length > >.
 	 * @param focusCurveWidthError The vector<vector<Length > > value to which focusCurveWidthError is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setFocusCurveWidthError (vector<vector<Length > > focusCurveWidthError) {
	
 		this->focusCurveWidthError = focusCurveWidthError;
	
		focusCurveWidthErrorExists = true;
	
 	}
	
	
	/**
	 * Mark focusCurveWidthError, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearFocusCurveWidthError () {
		focusCurveWidthErrorExists = false;
	}
	

	
	/**
	 * The attribute focusCurveWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the focusCurveWasFixed attribute exists. 
	 */
	bool CalFocusRow::isFocusCurveWasFixedExists() const {
		return focusCurveWasFixedExists;
	}
	

	
 	/**
 	 * Get focusCurveWasFixed, which is optional.
 	 * @return focusCurveWasFixed as vector<bool >
 	 * @throw IllegalAccessException If focusCurveWasFixed does not exist.
 	 */
 	vector<bool > CalFocusRow::getFocusCurveWasFixed() const  {
		if (!focusCurveWasFixedExists) {
			throw IllegalAccessException("focusCurveWasFixed", "CalFocus");
		}
	
  		return focusCurveWasFixed;
 	}

 	/**
 	 * Set focusCurveWasFixed with the specified vector<bool >.
 	 * @param focusCurveWasFixed The vector<bool > value to which focusCurveWasFixed is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setFocusCurveWasFixed (vector<bool > focusCurveWasFixed) {
	
 		this->focusCurveWasFixed = focusCurveWasFixed;
	
		focusCurveWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark focusCurveWasFixed, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearFocusCurveWasFixed () {
		focusCurveWasFixedExists = false;
	}
	

	
	/**
	 * The attribute offIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensity attribute exists. 
	 */
	bool CalFocusRow::isOffIntensityExists() const {
		return offIntensityExists;
	}
	

	
 	/**
 	 * Get offIntensity, which is optional.
 	 * @return offIntensity as vector<Temperature >
 	 * @throw IllegalAccessException If offIntensity does not exist.
 	 */
 	vector<Temperature > CalFocusRow::getOffIntensity() const  {
		if (!offIntensityExists) {
			throw IllegalAccessException("offIntensity", "CalFocus");
		}
	
  		return offIntensity;
 	}

 	/**
 	 * Set offIntensity with the specified vector<Temperature >.
 	 * @param offIntensity The vector<Temperature > value to which offIntensity is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setOffIntensity (vector<Temperature > offIntensity) {
	
 		this->offIntensity = offIntensity;
	
		offIntensityExists = true;
	
 	}
	
	
	/**
	 * Mark offIntensity, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearOffIntensity () {
		offIntensityExists = false;
	}
	

	
	/**
	 * The attribute offIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensityError attribute exists. 
	 */
	bool CalFocusRow::isOffIntensityErrorExists() const {
		return offIntensityErrorExists;
	}
	

	
 	/**
 	 * Get offIntensityError, which is optional.
 	 * @return offIntensityError as vector<Temperature >
 	 * @throw IllegalAccessException If offIntensityError does not exist.
 	 */
 	vector<Temperature > CalFocusRow::getOffIntensityError() const  {
		if (!offIntensityErrorExists) {
			throw IllegalAccessException("offIntensityError", "CalFocus");
		}
	
  		return offIntensityError;
 	}

 	/**
 	 * Set offIntensityError with the specified vector<Temperature >.
 	 * @param offIntensityError The vector<Temperature > value to which offIntensityError is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setOffIntensityError (vector<Temperature > offIntensityError) {
	
 		this->offIntensityError = offIntensityError;
	
		offIntensityErrorExists = true;
	
 	}
	
	
	/**
	 * Mark offIntensityError, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearOffIntensityError () {
		offIntensityErrorExists = false;
	}
	

	
	/**
	 * The attribute offIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the offIntensityWasFixed attribute exists. 
	 */
	bool CalFocusRow::isOffIntensityWasFixedExists() const {
		return offIntensityWasFixedExists;
	}
	

	
 	/**
 	 * Get offIntensityWasFixed, which is optional.
 	 * @return offIntensityWasFixed as bool
 	 * @throw IllegalAccessException If offIntensityWasFixed does not exist.
 	 */
 	bool CalFocusRow::getOffIntensityWasFixed() const  {
		if (!offIntensityWasFixedExists) {
			throw IllegalAccessException("offIntensityWasFixed", "CalFocus");
		}
	
  		return offIntensityWasFixed;
 	}

 	/**
 	 * Set offIntensityWasFixed with the specified bool.
 	 * @param offIntensityWasFixed The bool value to which offIntensityWasFixed is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setOffIntensityWasFixed (bool offIntensityWasFixed) {
	
 		this->offIntensityWasFixed = offIntensityWasFixed;
	
		offIntensityWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark offIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearOffIntensityWasFixed () {
		offIntensityWasFixedExists = false;
	}
	

	
	/**
	 * The attribute peakIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensity attribute exists. 
	 */
	bool CalFocusRow::isPeakIntensityExists() const {
		return peakIntensityExists;
	}
	

	
 	/**
 	 * Get peakIntensity, which is optional.
 	 * @return peakIntensity as vector<Temperature >
 	 * @throw IllegalAccessException If peakIntensity does not exist.
 	 */
 	vector<Temperature > CalFocusRow::getPeakIntensity() const  {
		if (!peakIntensityExists) {
			throw IllegalAccessException("peakIntensity", "CalFocus");
		}
	
  		return peakIntensity;
 	}

 	/**
 	 * Set peakIntensity with the specified vector<Temperature >.
 	 * @param peakIntensity The vector<Temperature > value to which peakIntensity is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setPeakIntensity (vector<Temperature > peakIntensity) {
	
 		this->peakIntensity = peakIntensity;
	
		peakIntensityExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensity, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearPeakIntensity () {
		peakIntensityExists = false;
	}
	

	
	/**
	 * The attribute peakIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityError attribute exists. 
	 */
	bool CalFocusRow::isPeakIntensityErrorExists() const {
		return peakIntensityErrorExists;
	}
	

	
 	/**
 	 * Get peakIntensityError, which is optional.
 	 * @return peakIntensityError as vector<Temperature >
 	 * @throw IllegalAccessException If peakIntensityError does not exist.
 	 */
 	vector<Temperature > CalFocusRow::getPeakIntensityError() const  {
		if (!peakIntensityErrorExists) {
			throw IllegalAccessException("peakIntensityError", "CalFocus");
		}
	
  		return peakIntensityError;
 	}

 	/**
 	 * Set peakIntensityError with the specified vector<Temperature >.
 	 * @param peakIntensityError The vector<Temperature > value to which peakIntensityError is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setPeakIntensityError (vector<Temperature > peakIntensityError) {
	
 		this->peakIntensityError = peakIntensityError;
	
		peakIntensityErrorExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensityError, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearPeakIntensityError () {
		peakIntensityErrorExists = false;
	}
	

	
	/**
	 * The attribute peakIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityWasFixed attribute exists. 
	 */
	bool CalFocusRow::isPeakIntensityWasFixedExists() const {
		return peakIntensityWasFixedExists;
	}
	

	
 	/**
 	 * Get peakIntensityWasFixed, which is optional.
 	 * @return peakIntensityWasFixed as bool
 	 * @throw IllegalAccessException If peakIntensityWasFixed does not exist.
 	 */
 	bool CalFocusRow::getPeakIntensityWasFixed() const  {
		if (!peakIntensityWasFixedExists) {
			throw IllegalAccessException("peakIntensityWasFixed", "CalFocus");
		}
	
  		return peakIntensityWasFixed;
 	}

 	/**
 	 * Set peakIntensityWasFixed with the specified bool.
 	 * @param peakIntensityWasFixed The bool value to which peakIntensityWasFixed is to be set.
 	 
 	
 	 */
 	void CalFocusRow::setPeakIntensityWasFixed (bool peakIntensityWasFixed) {
	
 		this->peakIntensityWasFixed = peakIntensityWasFixed;
	
		peakIntensityWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void CalFocusRow::clearPeakIntensityWasFixed () {
		peakIntensityWasFixedExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalFocusRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalFocus");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalFocusRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalFocus");
		
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
	 CalDataRow* CalFocusRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalFocusRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalFocusRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalFocusRow::CalFocusRow (CalFocusTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		polarizationsAveragedExists = false;
	

	
		focusCurveWidthExists = false;
	

	
		focusCurveWidthErrorExists = false;
	

	
		focusCurveWasFixedExists = false;
	

	
		offIntensityExists = false;
	

	
		offIntensityErrorExists = false;
	

	
		offIntensityWasFixedExists = false;
	

	
		peakIntensityExists = false;
	

	
		peakIntensityErrorExists = false;
	

	
		peakIntensityWasFixedExists = false;
	

	
	

	

	
	
	
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
atmPhaseCorrection = CAtmPhaseCorrection::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
focusMethod = CFocusMethod::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaName"] = &CalFocusRow::antennaNameFromBin; 
	 fromBinMethods["receiverBand"] = &CalFocusRow::receiverBandFromBin; 
	 fromBinMethods["calDataId"] = &CalFocusRow::calDataIdFromBin; 
	 fromBinMethods["calReductionId"] = &CalFocusRow::calReductionIdFromBin; 
	 fromBinMethods["startValidTime"] = &CalFocusRow::startValidTimeFromBin; 
	 fromBinMethods["endValidTime"] = &CalFocusRow::endValidTimeFromBin; 
	 fromBinMethods["ambientTemperature"] = &CalFocusRow::ambientTemperatureFromBin; 
	 fromBinMethods["atmPhaseCorrection"] = &CalFocusRow::atmPhaseCorrectionFromBin; 
	 fromBinMethods["focusMethod"] = &CalFocusRow::focusMethodFromBin; 
	 fromBinMethods["frequencyRange"] = &CalFocusRow::frequencyRangeFromBin; 
	 fromBinMethods["pointingDirection"] = &CalFocusRow::pointingDirectionFromBin; 
	 fromBinMethods["numReceptor"] = &CalFocusRow::numReceptorFromBin; 
	 fromBinMethods["polarizationTypes"] = &CalFocusRow::polarizationTypesFromBin; 
	 fromBinMethods["wereFixed"] = &CalFocusRow::wereFixedFromBin; 
	 fromBinMethods["offset"] = &CalFocusRow::offsetFromBin; 
	 fromBinMethods["offsetError"] = &CalFocusRow::offsetErrorFromBin; 
	 fromBinMethods["offsetWasTied"] = &CalFocusRow::offsetWasTiedFromBin; 
	 fromBinMethods["reducedChiSquared"] = &CalFocusRow::reducedChiSquaredFromBin; 
		
	
	 fromBinMethods["polarizationsAveraged"] = &CalFocusRow::polarizationsAveragedFromBin; 
	 fromBinMethods["focusCurveWidth"] = &CalFocusRow::focusCurveWidthFromBin; 
	 fromBinMethods["focusCurveWidthError"] = &CalFocusRow::focusCurveWidthErrorFromBin; 
	 fromBinMethods["focusCurveWasFixed"] = &CalFocusRow::focusCurveWasFixedFromBin; 
	 fromBinMethods["offIntensity"] = &CalFocusRow::offIntensityFromBin; 
	 fromBinMethods["offIntensityError"] = &CalFocusRow::offIntensityErrorFromBin; 
	 fromBinMethods["offIntensityWasFixed"] = &CalFocusRow::offIntensityWasFixedFromBin; 
	 fromBinMethods["peakIntensity"] = &CalFocusRow::peakIntensityFromBin; 
	 fromBinMethods["peakIntensityError"] = &CalFocusRow::peakIntensityErrorFromBin; 
	 fromBinMethods["peakIntensityWasFixed"] = &CalFocusRow::peakIntensityWasFixedFromBin; 
	
	}
	
	CalFocusRow::CalFocusRow (CalFocusTable &t, CalFocusRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		polarizationsAveragedExists = false;
	

	
		focusCurveWidthExists = false;
	

	
		focusCurveWidthErrorExists = false;
	

	
		focusCurveWasFixedExists = false;
	

	
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
		
			atmPhaseCorrection = row.atmPhaseCorrection;
		
			focusMethod = row.focusMethod;
		
			frequencyRange = row.frequencyRange;
		
			pointingDirection = row.pointingDirection;
		
			numReceptor = row.numReceptor;
		
			polarizationTypes = row.polarizationTypes;
		
			wereFixed = row.wereFixed;
		
			offset = row.offset;
		
			offsetError = row.offsetError;
		
			offsetWasTied = row.offsetWasTied;
		
			reducedChiSquared = row.reducedChiSquared;
		
		
		
		
		if (row.polarizationsAveragedExists) {
			polarizationsAveraged = row.polarizationsAveraged;		
			polarizationsAveragedExists = true;
		}
		else
			polarizationsAveragedExists = false;
		
		if (row.focusCurveWidthExists) {
			focusCurveWidth = row.focusCurveWidth;		
			focusCurveWidthExists = true;
		}
		else
			focusCurveWidthExists = false;
		
		if (row.focusCurveWidthErrorExists) {
			focusCurveWidthError = row.focusCurveWidthError;		
			focusCurveWidthErrorExists = true;
		}
		else
			focusCurveWidthErrorExists = false;
		
		if (row.focusCurveWasFixedExists) {
			focusCurveWasFixed = row.focusCurveWasFixed;		
			focusCurveWasFixedExists = true;
		}
		else
			focusCurveWasFixedExists = false;
		
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
		
		 fromBinMethods["antennaName"] = &CalFocusRow::antennaNameFromBin; 
		 fromBinMethods["receiverBand"] = &CalFocusRow::receiverBandFromBin; 
		 fromBinMethods["calDataId"] = &CalFocusRow::calDataIdFromBin; 
		 fromBinMethods["calReductionId"] = &CalFocusRow::calReductionIdFromBin; 
		 fromBinMethods["startValidTime"] = &CalFocusRow::startValidTimeFromBin; 
		 fromBinMethods["endValidTime"] = &CalFocusRow::endValidTimeFromBin; 
		 fromBinMethods["ambientTemperature"] = &CalFocusRow::ambientTemperatureFromBin; 
		 fromBinMethods["atmPhaseCorrection"] = &CalFocusRow::atmPhaseCorrectionFromBin; 
		 fromBinMethods["focusMethod"] = &CalFocusRow::focusMethodFromBin; 
		 fromBinMethods["frequencyRange"] = &CalFocusRow::frequencyRangeFromBin; 
		 fromBinMethods["pointingDirection"] = &CalFocusRow::pointingDirectionFromBin; 
		 fromBinMethods["numReceptor"] = &CalFocusRow::numReceptorFromBin; 
		 fromBinMethods["polarizationTypes"] = &CalFocusRow::polarizationTypesFromBin; 
		 fromBinMethods["wereFixed"] = &CalFocusRow::wereFixedFromBin; 
		 fromBinMethods["offset"] = &CalFocusRow::offsetFromBin; 
		 fromBinMethods["offsetError"] = &CalFocusRow::offsetErrorFromBin; 
		 fromBinMethods["offsetWasTied"] = &CalFocusRow::offsetWasTiedFromBin; 
		 fromBinMethods["reducedChiSquared"] = &CalFocusRow::reducedChiSquaredFromBin; 
			
	
		 fromBinMethods["polarizationsAveraged"] = &CalFocusRow::polarizationsAveragedFromBin; 
		 fromBinMethods["focusCurveWidth"] = &CalFocusRow::focusCurveWidthFromBin; 
		 fromBinMethods["focusCurveWidthError"] = &CalFocusRow::focusCurveWidthErrorFromBin; 
		 fromBinMethods["focusCurveWasFixed"] = &CalFocusRow::focusCurveWasFixedFromBin; 
		 fromBinMethods["offIntensity"] = &CalFocusRow::offIntensityFromBin; 
		 fromBinMethods["offIntensityError"] = &CalFocusRow::offIntensityErrorFromBin; 
		 fromBinMethods["offIntensityWasFixed"] = &CalFocusRow::offIntensityWasFixedFromBin; 
		 fromBinMethods["peakIntensity"] = &CalFocusRow::peakIntensityFromBin; 
		 fromBinMethods["peakIntensityError"] = &CalFocusRow::peakIntensityErrorFromBin; 
		 fromBinMethods["peakIntensityWasFixed"] = &CalFocusRow::peakIntensityWasFixedFromBin; 
			
	}

	
	bool CalFocusRow::compareNoAutoInc(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, FocusMethodMod::FocusMethod focusMethod, vector<Frequency > frequencyRange, vector<Angle > pointingDirection, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<bool > wereFixed, vector<vector<Length > > offset, vector<vector<Length > > offsetError, vector<vector<bool > > offsetWasTied, vector<vector<double > > reducedChiSquared) {
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
	

	
		
		result = result && (this->atmPhaseCorrection == atmPhaseCorrection);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusMethod == focusMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingDirection == pointingDirection);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->wereFixed == wereFixed);
		
		if (!result) return false;
	

	
		
		result = result && (this->offset == offset);
		
		if (!result) return false;
	

	
		
		result = result && (this->offsetError == offsetError);
		
		if (!result) return false;
	

	
		
		result = result && (this->offsetWasTied == offsetWasTied);
		
		if (!result) return false;
	

	
		
		result = result && (this->reducedChiSquared == reducedChiSquared);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalFocusRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, FocusMethodMod::FocusMethod focusMethod, vector<Frequency > frequencyRange, vector<Angle > pointingDirection, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<bool > wereFixed, vector<vector<Length > > offset, vector<vector<Length > > offsetError, vector<vector<bool > > offsetWasTied, vector<vector<double > > reducedChiSquared) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->ambientTemperature == ambientTemperature)) return false;
	

	
		if (!(this->atmPhaseCorrection == atmPhaseCorrection)) return false;
	

	
		if (!(this->focusMethod == focusMethod)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->pointingDirection == pointingDirection)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->wereFixed == wereFixed)) return false;
	

	
		if (!(this->offset == offset)) return false;
	

	
		if (!(this->offsetError == offsetError)) return false;
	

	
		if (!(this->offsetWasTied == offsetWasTied)) return false;
	

	
		if (!(this->reducedChiSquared == reducedChiSquared)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalFocusRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalFocusRow::equalByRequiredValue(CalFocusRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->ambientTemperature != x->ambientTemperature) return false;
			
		if (this->atmPhaseCorrection != x->atmPhaseCorrection) return false;
			
		if (this->focusMethod != x->focusMethod) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->pointingDirection != x->pointingDirection) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->wereFixed != x->wereFixed) return false;
			
		if (this->offset != x->offset) return false;
			
		if (this->offsetError != x->offsetError) return false;
			
		if (this->offsetWasTied != x->offsetWasTied) return false;
			
		if (this->reducedChiSquared != x->reducedChiSquared) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalFocusAttributeFromBin> CalFocusRow::initFromBinMethods() {
		map<string, CalFocusAttributeFromBin> result;
		
		result["antennaName"] = &CalFocusRow::antennaNameFromBin;
		result["receiverBand"] = &CalFocusRow::receiverBandFromBin;
		result["calDataId"] = &CalFocusRow::calDataIdFromBin;
		result["calReductionId"] = &CalFocusRow::calReductionIdFromBin;
		result["startValidTime"] = &CalFocusRow::startValidTimeFromBin;
		result["endValidTime"] = &CalFocusRow::endValidTimeFromBin;
		result["ambientTemperature"] = &CalFocusRow::ambientTemperatureFromBin;
		result["atmPhaseCorrection"] = &CalFocusRow::atmPhaseCorrectionFromBin;
		result["focusMethod"] = &CalFocusRow::focusMethodFromBin;
		result["frequencyRange"] = &CalFocusRow::frequencyRangeFromBin;
		result["pointingDirection"] = &CalFocusRow::pointingDirectionFromBin;
		result["numReceptor"] = &CalFocusRow::numReceptorFromBin;
		result["polarizationTypes"] = &CalFocusRow::polarizationTypesFromBin;
		result["wereFixed"] = &CalFocusRow::wereFixedFromBin;
		result["offset"] = &CalFocusRow::offsetFromBin;
		result["offsetError"] = &CalFocusRow::offsetErrorFromBin;
		result["offsetWasTied"] = &CalFocusRow::offsetWasTiedFromBin;
		result["reducedChiSquared"] = &CalFocusRow::reducedChiSquaredFromBin;
		
		
		result["polarizationsAveraged"] = &CalFocusRow::polarizationsAveragedFromBin;
		result["focusCurveWidth"] = &CalFocusRow::focusCurveWidthFromBin;
		result["focusCurveWidthError"] = &CalFocusRow::focusCurveWidthErrorFromBin;
		result["focusCurveWasFixed"] = &CalFocusRow::focusCurveWasFixedFromBin;
		result["offIntensity"] = &CalFocusRow::offIntensityFromBin;
		result["offIntensityError"] = &CalFocusRow::offIntensityErrorFromBin;
		result["offIntensityWasFixed"] = &CalFocusRow::offIntensityWasFixedFromBin;
		result["peakIntensity"] = &CalFocusRow::peakIntensityFromBin;
		result["peakIntensityError"] = &CalFocusRow::peakIntensityErrorFromBin;
		result["peakIntensityWasFixed"] = &CalFocusRow::peakIntensityWasFixedFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
