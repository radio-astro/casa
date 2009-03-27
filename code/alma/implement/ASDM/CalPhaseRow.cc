
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
 * File CalPhaseRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalPhaseRow.h>
#include <CalPhaseTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalPhaseRow;
using asdm::CalPhaseTable;

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

	CalPhaseRow::~CalPhaseRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalPhaseTable &CalPhaseRow::getTable() const {
		return table;
	}
	
	void CalPhaseRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPhaseRowIDL struct.
	 */
	CalPhaseRowIDL *CalPhaseRow::toIDL() const {
		CalPhaseRowIDL *x = new CalPhaseRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->basebandName = basebandName;
 				
 			
		
	

	
  		
		
		
			
				
		x->numBaseline = numBaseline;
 				
 			
		
	

	
  		
		
		
			
				
		x->numAPC = numAPC;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->antennaNames.length(antennaNames.size());
		for (unsigned int i = 0; i < antennaNames.size(); i++) {
			x->antennaNames[i].length(antennaNames.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < antennaNames.size() ; i++)
			for (unsigned int j = 0; j < antennaNames.at(i).size(); j++)
					
						
				x->antennaNames[i][j] = CORBA::string_dup(antennaNames.at(i).at(j).c_str());
						
			 						
		
			
		
	

	
  		
		
		
			
		x->atmPhaseCorrections.length(atmPhaseCorrections.size());
		for (unsigned int i = 0; i < atmPhaseCorrections.size(); ++i) {
			
				
			x->atmPhaseCorrections[i] = atmPhaseCorrections.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->ampli.length(ampli.size());
		for (unsigned int i = 0; i < ampli.size(); i++) {
			x->ampli[i].length(ampli.at(i).size());
			for (unsigned int j = 0; j < ampli.at(i).size(); j++) {
				x->ampli[i][j].length(ampli.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < ampli.size() ; i++)
			for (unsigned int j = 0; j < ampli.at(i).size(); j++)
				for (unsigned int k = 0; k < ampli.at(i).at(j).size(); k++)
					
						
					x->ampli[i][j][k] = ampli.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
  		
		
		
			
		x->phase.length(phase.size());
		for (unsigned int i = 0; i < phase.size(); i++) {
			x->phase[i].length(phase.at(i).size());
			for (unsigned int j = 0; j < phase.at(i).size(); j++) {
				x->phase[i][j].length(phase.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < phase.size() ; i++)
			for (unsigned int j = 0; j < phase.at(i).size(); j++)
				for (unsigned int k = 0; k < phase.at(i).at(j).size(); k++)
					
						
					x->phase[i][j][k] = phase.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
  		
		
		
			
		x->phaseRms.length(phaseRms.size());
		for (unsigned int i = 0; i < phaseRms.size(); i++) {
			x->phaseRms[i].length(phaseRms.at(i).size());
			for (unsigned int j = 0; j < phaseRms.at(i).size(); j++) {
				x->phaseRms[i][j].length(phaseRms.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < phaseRms.size() ; i++)
			for (unsigned int j = 0; j < phaseRms.at(i).size(); j++)
				for (unsigned int k = 0; k < phaseRms.at(i).at(j).size(); k++)
					
					x->phaseRms[i][j][k] = phaseRms.at(i).at(j).at(k).toIDLAngle();
												
			
		
	

	
  		
		
		
			
		x->statPhaseRms.length(statPhaseRms.size());
		for (unsigned int i = 0; i < statPhaseRms.size(); i++) {
			x->statPhaseRms[i].length(statPhaseRms.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < statPhaseRms.size() ; i++)
			for (unsigned int j = 0; j < statPhaseRms.at(i).size(); j++)
					
				x->statPhaseRms[i][j]= statPhaseRms.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->correctionValidityExists = correctionValidityExists;
		
		
			
		x->correctionValidity.length(correctionValidity.size());
		for (unsigned int i = 0; i < correctionValidity.size(); ++i) {
			
				
			x->correctionValidity[i] = correctionValidity.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->decorrelationFactor.length(decorrelationFactor.size());
		for (unsigned int i = 0; i < decorrelationFactor.size(); i++) {
			x->decorrelationFactor[i].length(decorrelationFactor.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < decorrelationFactor.size() ; i++)
			for (unsigned int j = 0; j < decorrelationFactor.at(i).size(); j++)
					
						
				x->decorrelationFactor[i][j] = decorrelationFactor.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->baselineLengths.length(baselineLengths.size());
		for (unsigned int i = 0; i < baselineLengths.size(); ++i) {
			
			x->baselineLengths[i] = baselineLengths.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->direction.length(direction.size());
		for (unsigned int i = 0; i < direction.size(); ++i) {
			
			x->direction[i] = direction.at(i).toIDLAngle();
			
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPhaseRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalPhaseRow::setFromIDL (CalPhaseRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setBasebandName(x.basebandName);
  			
 		
		
	

	
		
		
			
		setNumBaseline(x.numBaseline);
  			
 		
		
	

	
		
		
			
		setNumAPC(x.numAPC);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		antennaNames .clear();
		vector<string> v_aux_antennaNames;
		for (unsigned int i = 0; i < x.antennaNames.length(); ++i) {
			v_aux_antennaNames.clear();
			for (unsigned int j = 0; j < x.antennaNames[0].length(); ++j) {
				
				v_aux_antennaNames.push_back(string (x.antennaNames[i][j]));
				
  			}
  			antennaNames.push_back(v_aux_antennaNames);			
		}
			
  		
		
	

	
		
		
			
		atmPhaseCorrections .clear();
		for (unsigned int i = 0; i <x.atmPhaseCorrections.length(); ++i) {
			
			atmPhaseCorrections.push_back(x.atmPhaseCorrections[i]);
  			
		}
			
  		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		ampli .clear();
		vector< vector<float> > vv_aux_ampli;
		vector<float> v_aux_ampli;
		
		for (unsigned int i = 0; i < x.ampli.length(); ++i) {
			vv_aux_ampli.clear();
			for (unsigned int j = 0; j < x.ampli[0].length(); ++j) {
				v_aux_ampli.clear();
				for (unsigned int k = 0; k < x.ampli[0][0].length(); ++k) {
					
					v_aux_ampli.push_back(x.ampli[i][j][k]);
		  			
		  		}
		  		vv_aux_ampli.push_back(v_aux_ampli);
  			}
  			ampli.push_back(vv_aux_ampli);
		}
			
  		
		
	

	
		
		
			
		phase .clear();
		vector< vector<float> > vv_aux_phase;
		vector<float> v_aux_phase;
		
		for (unsigned int i = 0; i < x.phase.length(); ++i) {
			vv_aux_phase.clear();
			for (unsigned int j = 0; j < x.phase[0].length(); ++j) {
				v_aux_phase.clear();
				for (unsigned int k = 0; k < x.phase[0][0].length(); ++k) {
					
					v_aux_phase.push_back(x.phase[i][j][k]);
		  			
		  		}
		  		vv_aux_phase.push_back(v_aux_phase);
  			}
  			phase.push_back(vv_aux_phase);
		}
			
  		
		
	

	
		
		
			
		phaseRms .clear();
		vector< vector<Angle> > vv_aux_phaseRms;
		vector<Angle> v_aux_phaseRms;
		
		for (unsigned int i = 0; i < x.phaseRms.length(); ++i) {
			vv_aux_phaseRms.clear();
			for (unsigned int j = 0; j < x.phaseRms[0].length(); ++j) {
				v_aux_phaseRms.clear();
				for (unsigned int k = 0; k < x.phaseRms[0][0].length(); ++k) {
					
					v_aux_phaseRms.push_back(Angle (x.phaseRms[i][j][k]));
					
		  		}
		  		vv_aux_phaseRms.push_back(v_aux_phaseRms);
  			}
  			phaseRms.push_back(vv_aux_phaseRms);
		}
			
  		
		
	

	
		
		
			
		statPhaseRms .clear();
		vector<Angle> v_aux_statPhaseRms;
		for (unsigned int i = 0; i < x.statPhaseRms.length(); ++i) {
			v_aux_statPhaseRms.clear();
			for (unsigned int j = 0; j < x.statPhaseRms[0].length(); ++j) {
				
				v_aux_statPhaseRms.push_back(Angle (x.statPhaseRms[i][j]));
				
  			}
  			statPhaseRms.push_back(v_aux_statPhaseRms);			
		}
			
  		
		
	

	
		
		correctionValidityExists = x.correctionValidityExists;
		if (x.correctionValidityExists) {
		
		
			
		correctionValidity .clear();
		for (unsigned int i = 0; i <x.correctionValidity.length(); ++i) {
			
			correctionValidity.push_back(x.correctionValidity[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		
			
		decorrelationFactor .clear();
		vector<float> v_aux_decorrelationFactor;
		for (unsigned int i = 0; i < x.decorrelationFactor.length(); ++i) {
			v_aux_decorrelationFactor.clear();
			for (unsigned int j = 0; j < x.decorrelationFactor[0].length(); ++j) {
				
				v_aux_decorrelationFactor.push_back(x.decorrelationFactor[i][j]);
	  			
  			}
  			decorrelationFactor.push_back(v_aux_decorrelationFactor);			
		}
			
  		
		
	

	
		
		
			
		baselineLengths .clear();
		for (unsigned int i = 0; i <x.baselineLengths.length(); ++i) {
			
			baselineLengths.push_back(Length (x.baselineLengths[i]));
			
		}
			
  		
		
	

	
		
		
			
		direction .clear();
		for (unsigned int i = 0; i <x.direction.length(); ++i) {
			
			direction.push_back(Angle (x.direction[i]));
			
		}
			
  		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalPhase");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalPhaseRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
			buf.append(EnumerationParser::toXML("basebandName", basebandName));
		
		
	

  	
 		
		
		Parser::toXML(numBaseline, "numBaseline", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAPC, "numAPC", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(antennaNames, "antennaNames", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrections", atmPhaseCorrections));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(ampli, "ampli", buf);
		
		
	

  	
 		
		
		Parser::toXML(phase, "phase", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseRms, "phaseRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(statPhaseRms, "statPhaseRms", buf);
		
		
	

  	
 		
		if (correctionValidityExists) {
		
		
		Parser::toXML(correctionValidity, "correctionValidity", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(decorrelationFactor, "decorrelationFactor", buf);
		
		
	

  	
 		
		
		Parser::toXML(baselineLengths, "baselineLengths", buf);
		
		
	

  	
 		
		
		Parser::toXML(direction, "direction", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalPhaseRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
		
		
		
		basebandName = EnumerationParser::getBasebandName("basebandName","CalPhase",rowDoc);
		
		
		
	

	
  		
			
	  	setNumBaseline(Parser::getInteger("numBaseline","CalPhase",rowDoc));
			
		
	

	
  		
			
	  	setNumAPC(Parser::getInteger("numAPC","CalPhase",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalPhase",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalPhase",rowDoc);
		
		
		
	

	
  		
			
					
	  	setAntennaNames(Parser::get2DString("antennaNames","CalPhase",rowDoc));
	  			
	  		
		
	

	
		
		
		
		atmPhaseCorrections = EnumerationParser::getAtmPhaseCorrection1D("atmPhaseCorrections","CalPhase",rowDoc);			
		
		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalPhase",rowDoc);			
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalPhase",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalPhase",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalPhase",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setAmpli(Parser::get3DFloat("ampli","CalPhase",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPhase(Parser::get3DFloat("phase","CalPhase",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPhaseRms(Parser::get3DAngle("phaseRms","CalPhase",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setStatPhaseRms(Parser::get2DAngle("statPhaseRms","CalPhase",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<correctionValidity>")) {
			
								
	  		setCorrectionValidity(Parser::get1DBoolean("correctionValidity","CalPhase",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
					
	  	setDecorrelationFactor(Parser::get2DFloat("decorrelationFactor","CalPhase",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setBaselineLengths(Parser::get1DLength("baselineLengths","CalPhase",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setDirection(Parser::get1DAngle("direction","CalPhase",rowDoc));
	  			
	  		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPhase");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName CalPhaseRow::getBasebandName() const {
	
  		return basebandName;
 	}

 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPhaseRow::setBasebandName (BasebandNameMod::BasebandName basebandName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("basebandName", "CalPhase");
		
  		}
  	
 		this->basebandName = basebandName;
	
 	}
	
	

	

	
 	/**
 	 * Get numBaseline.
 	 * @return numBaseline as int
 	 */
 	int CalPhaseRow::getNumBaseline() const {
	
  		return numBaseline;
 	}

 	/**
 	 * Set numBaseline with the specified int.
 	 * @param numBaseline The int value to which numBaseline is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setNumBaseline (int numBaseline)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBaseline = numBaseline;
	
 	}
	
	

	

	
 	/**
 	 * Get numAPC.
 	 * @return numAPC as int
 	 */
 	int CalPhaseRow::getNumAPC() const {
	
  		return numAPC;
 	}

 	/**
 	 * Set numAPC with the specified int.
 	 * @param numAPC The int value to which numAPC is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setNumAPC (int numAPC)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAPC = numAPC;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalPhaseRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalPhaseRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaNames.
 	 * @return antennaNames as vector<vector<string > >
 	 */
 	vector<vector<string > > CalPhaseRow::getAntennaNames() const {
	
  		return antennaNames;
 	}

 	/**
 	 * Set antennaNames with the specified vector<vector<string > >.
 	 * @param antennaNames The vector<vector<string > > value to which antennaNames is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setAntennaNames (vector<vector<string > > antennaNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaNames = antennaNames;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrections.
 	 * @return atmPhaseCorrections as vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >
 	 */
 	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > CalPhaseRow::getAtmPhaseCorrections() const {
	
  		return atmPhaseCorrections;
 	}

 	/**
 	 * Set atmPhaseCorrections with the specified vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >.
 	 * @param atmPhaseCorrections The vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > value to which atmPhaseCorrections is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setAtmPhaseCorrections (vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmPhaseCorrections = atmPhaseCorrections;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalPhaseRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalPhaseRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalPhaseRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalPhaseRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get ampli.
 	 * @return ampli as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > CalPhaseRow::getAmpli() const {
	
  		return ampli;
 	}

 	/**
 	 * Set ampli with the specified vector<vector<vector<float > > >.
 	 * @param ampli The vector<vector<vector<float > > > value to which ampli is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setAmpli (vector<vector<vector<float > > > ampli)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->ampli = ampli;
	
 	}
	
	

	

	
 	/**
 	 * Get phase.
 	 * @return phase as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > CalPhaseRow::getPhase() const {
	
  		return phase;
 	}

 	/**
 	 * Set phase with the specified vector<vector<vector<float > > >.
 	 * @param phase The vector<vector<vector<float > > > value to which phase is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setPhase (vector<vector<vector<float > > > phase)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phase = phase;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseRms.
 	 * @return phaseRms as vector<vector<vector<Angle > > >
 	 */
 	vector<vector<vector<Angle > > > CalPhaseRow::getPhaseRms() const {
	
  		return phaseRms;
 	}

 	/**
 	 * Set phaseRms with the specified vector<vector<vector<Angle > > >.
 	 * @param phaseRms The vector<vector<vector<Angle > > > value to which phaseRms is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setPhaseRms (vector<vector<vector<Angle > > > phaseRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseRms = phaseRms;
	
 	}
	
	

	

	
 	/**
 	 * Get statPhaseRms.
 	 * @return statPhaseRms as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > CalPhaseRow::getStatPhaseRms() const {
	
  		return statPhaseRms;
 	}

 	/**
 	 * Set statPhaseRms with the specified vector<vector<Angle > >.
 	 * @param statPhaseRms The vector<vector<Angle > > value to which statPhaseRms is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setStatPhaseRms (vector<vector<Angle > > statPhaseRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->statPhaseRms = statPhaseRms;
	
 	}
	
	

	
	/**
	 * The attribute correctionValidity is optional. Return true if this attribute exists.
	 * @return true if and only if the correctionValidity attribute exists. 
	 */
	bool CalPhaseRow::isCorrectionValidityExists() const {
		return correctionValidityExists;
	}
	

	
 	/**
 	 * Get correctionValidity, which is optional.
 	 * @return correctionValidity as vector<bool >
 	 * @throw IllegalAccessException If correctionValidity does not exist.
 	 */
 	vector<bool > CalPhaseRow::getCorrectionValidity() const throw(IllegalAccessException) {
		if (!correctionValidityExists) {
			throw IllegalAccessException("correctionValidity", "CalPhase");
		}
	
  		return correctionValidity;
 	}

 	/**
 	 * Set correctionValidity with the specified vector<bool >.
 	 * @param correctionValidity The vector<bool > value to which correctionValidity is to be set.
 	 
 	
 	 */
 	void CalPhaseRow::setCorrectionValidity (vector<bool > correctionValidity) {
	
 		this->correctionValidity = correctionValidity;
	
		correctionValidityExists = true;
	
 	}
	
	
	/**
	 * Mark correctionValidity, which is an optional field, as non-existent.
	 */
	void CalPhaseRow::clearCorrectionValidity () {
		correctionValidityExists = false;
	}
	

	

	
 	/**
 	 * Get decorrelationFactor.
 	 * @return decorrelationFactor as vector<vector<float > >
 	 */
 	vector<vector<float > > CalPhaseRow::getDecorrelationFactor() const {
	
  		return decorrelationFactor;
 	}

 	/**
 	 * Set decorrelationFactor with the specified vector<vector<float > >.
 	 * @param decorrelationFactor The vector<vector<float > > value to which decorrelationFactor is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setDecorrelationFactor (vector<vector<float > > decorrelationFactor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->decorrelationFactor = decorrelationFactor;
	
 	}
	
	

	

	
 	/**
 	 * Get baselineLengths.
 	 * @return baselineLengths as vector<Length >
 	 */
 	vector<Length > CalPhaseRow::getBaselineLengths() const {
	
  		return baselineLengths;
 	}

 	/**
 	 * Set baselineLengths with the specified vector<Length >.
 	 * @param baselineLengths The vector<Length > value to which baselineLengths is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setBaselineLengths (vector<Length > baselineLengths)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->baselineLengths = baselineLengths;
	
 	}
	
	

	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > CalPhaseRow::getDirection() const {
	
  		return direction;
 	}

 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 	
 		
 	 */
 	void CalPhaseRow::setDirection (vector<Angle > direction)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->direction = direction;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalPhaseRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPhaseRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalPhase");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalPhaseRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPhaseRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalPhase");
		
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
	 CalDataRow* CalPhaseRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalPhaseRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalPhaseRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPhaseRow::CalPhaseRow (CalPhaseTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		correctionValidityExists = false;
	

	

	

	

	
	

	

	
	
	
	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
basebandName = CBasebandName::from_int(0);
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	CalPhaseRow::CalPhaseRow (CalPhaseTable &t, CalPhaseRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		correctionValidityExists = false;
	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			basebandName = row.basebandName;
		
		
		
		
			numBaseline = row.numBaseline;
		
			numAPC = row.numAPC;
		
			numReceptor = row.numReceptor;
		
			receiverBand = row.receiverBand;
		
			antennaNames = row.antennaNames;
		
			atmPhaseCorrections = row.atmPhaseCorrections;
		
			polarizationTypes = row.polarizationTypes;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			frequencyRange = row.frequencyRange;
		
			ampli = row.ampli;
		
			phase = row.phase;
		
			phaseRms = row.phaseRms;
		
			statPhaseRms = row.statPhaseRms;
		
			decorrelationFactor = row.decorrelationFactor;
		
			baselineLengths = row.baselineLengths;
		
			direction = row.direction;
		
		
		
		
		if (row.correctionValidityExists) {
			correctionValidity = row.correctionValidity;		
			correctionValidityExists = true;
		}
		else
			correctionValidityExists = false;
		
		}	
	}

	
	bool CalPhaseRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, BasebandNameMod::BasebandName basebandName, int numBaseline, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<vector<string > > antennaNames, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<vector<vector<float > > > ampli, vector<vector<vector<float > > > phase, vector<vector<vector<Angle > > > phaseRms, vector<vector<Angle > > statPhaseRms, vector<vector<float > > decorrelationFactor, vector<Length > baselineLengths, vector<Angle > direction) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->basebandName == basebandName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numBaseline == numBaseline);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAPC == numAPC);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaNames == antennaNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrections == atmPhaseCorrections);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->ampli == ampli);
		
		if (!result) return false;
	

	
		
		result = result && (this->phase == phase);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseRms == phaseRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->statPhaseRms == statPhaseRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->decorrelationFactor == decorrelationFactor);
		
		if (!result) return false;
	

	
		
		result = result && (this->baselineLengths == baselineLengths);
		
		if (!result) return false;
	

	
		
		result = result && (this->direction == direction);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalPhaseRow::compareRequiredValue(int numBaseline, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<vector<string > > antennaNames, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<vector<vector<float > > > ampli, vector<vector<vector<float > > > phase, vector<vector<vector<Angle > > > phaseRms, vector<vector<Angle > > statPhaseRms, vector<vector<float > > decorrelationFactor, vector<Length > baselineLengths, vector<Angle > direction) {
		bool result;
		result = true;
		
	
		if (!(this->numBaseline == numBaseline)) return false;
	

	
		if (!(this->numAPC == numAPC)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->antennaNames == antennaNames)) return false;
	

	
		if (!(this->atmPhaseCorrections == atmPhaseCorrections)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->ampli == ampli)) return false;
	

	
		if (!(this->phase == phase)) return false;
	

	
		if (!(this->phaseRms == phaseRms)) return false;
	

	
		if (!(this->statPhaseRms == statPhaseRms)) return false;
	

	
		if (!(this->decorrelationFactor == decorrelationFactor)) return false;
	

	
		if (!(this->baselineLengths == baselineLengths)) return false;
	

	
		if (!(this->direction == direction)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPhaseRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalPhaseRow::equalByRequiredValue(CalPhaseRow* x) {
		
			
		if (this->numBaseline != x->numBaseline) return false;
			
		if (this->numAPC != x->numAPC) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->antennaNames != x->antennaNames) return false;
			
		if (this->atmPhaseCorrections != x->atmPhaseCorrections) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->ampli != x->ampli) return false;
			
		if (this->phase != x->phase) return false;
			
		if (this->phaseRms != x->phaseRms) return false;
			
		if (this->statPhaseRms != x->statPhaseRms) return false;
			
		if (this->decorrelationFactor != x->decorrelationFactor) return false;
			
		if (this->baselineLengths != x->baselineLengths) return false;
			
		if (this->direction != x->direction) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
