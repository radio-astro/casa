
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
 * File CalBandpassRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalBandpassRow.h>
#include <CalBandpassTable.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>

#include <CalDataTable.h>
#include <CalDataRow.h>
	

using asdm::ASDM;
using asdm::CalBandpassRow;
using asdm::CalBandpassTable;

using asdm::CalReductionTable;
using asdm::CalReductionRow;

using asdm::CalDataTable;
using asdm::CalDataRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CalBandpassRow::~CalBandpassRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalBandpassTable &CalBandpassRow::getTable() const {
		return table;
	}
	
	void CalBandpassRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalBandpassRowIDL struct.
	 */
	CalBandpassRowIDL *CalBandpassRow::toIDL() const {
		CalBandpassRowIDL *x = new CalBandpassRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->basebandName = basebandName;
 				
 			
		
	

	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		
			
				
		x->numBaseline = numBaseline;
 				
 			
		
	

	
  		
		
		
			
				
		x->numAPC = numAPC;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->freqLimits.length(freqLimits.size());
		for (unsigned int i = 0; i < freqLimits.size(); ++i) {
			
			x->freqLimits[i] = freqLimits.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->antennaNames.length(antennaNames.size());
		for (unsigned int i = 0; i < antennaNames.size(); ++i) {
			
				
			x->antennaNames[i] = CORBA::string_dup(antennaNames.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->refAntennaName = CORBA::string_dup(refAntennaName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->atmPhaseCorrections.length(atmPhaseCorrections.size());
		for (unsigned int i = 0; i < atmPhaseCorrections.size(); ++i) {
			
				
			x->atmPhaseCorrections[i] = atmPhaseCorrections.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->numAmpliPoly = numAmpliPoly;
 				
 			
		
	

	
  		
		
		
			
		x->ampliCurve.length(ampliCurve.size());
		for (unsigned int i = 0; i < ampliCurve.size(); i++) {
			x->ampliCurve[i].length(ampliCurve.at(i).size());
			for (unsigned int j = 0; j < ampliCurve.at(i).size(); j++) {
				x->ampliCurve[i][j].length(ampliCurve.at(i).at(j).size());
				for (unsigned int k = 0; k < ampliCurve.at(i).at(j).size(); k++) {
					 x->ampliCurve[i][j][k].length(ampliCurve.at(i).at(j).at(k).size());
			    }
			}					 		
		}		
		
		for (unsigned int i = 0; i < ampliCurve.size() ; i++)
			for (unsigned int j = 0; j < ampliCurve.at(i).size(); j++)
				for (unsigned int k = 0; k < ampliCurve.at(i).at(j).size(); k++)
					for (unsigned int l = 0; l < ampliCurve.at(i).at(j).at(k).size(); l++)
					
						
						x->ampliCurve[i][j][k][l] = ampliCurve.at(i).at(j).at(k).at(l);
		 				
			 							
	 	cout << "Converted successfully" << endl;
			
		
	

	
  		
		
		
			
		x->ampliRms.length(ampliRms.size());
		for (unsigned int i = 0; i < ampliRms.size(); i++) {
			x->ampliRms[i].length(ampliRms.at(i).size());
			for (unsigned int j = 0; j < ampliRms.at(i).size(); j++) {
				x->ampliRms[i][j].length(ampliRms.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < ampliRms.size() ; i++)
			for (unsigned int j = 0; j < ampliRms.at(i).size(); j++)
				for (unsigned int k = 0; k < ampliRms.at(i).at(j).size(); k++)
					
						
					x->ampliRms[i][j][k] = ampliRms.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
  		
		
		
			
				
		x->numPhasePoly = numPhasePoly;
 				
 			
		
	

	
  		
		
		
			
		x->phaseCurve.length(phaseCurve.size());
		for (unsigned int i = 0; i < phaseCurve.size(); i++) {
			x->phaseCurve[i].length(phaseCurve.at(i).size());
			for (unsigned int j = 0; j < phaseCurve.at(i).size(); j++) {
				x->phaseCurve[i][j].length(phaseCurve.at(i).at(j).size());
				for (unsigned int k = 0; k < phaseCurve.at(i).at(j).size(); k++) {
					 x->phaseCurve[i][j][k].length(phaseCurve.at(i).at(j).at(k).size());
			    }
			}					 		
		}		
		
		for (unsigned int i = 0; i < phaseCurve.size() ; i++)
			for (unsigned int j = 0; j < phaseCurve.at(i).size(); j++)
				for (unsigned int k = 0; k < phaseCurve.at(i).at(j).size(); k++)
					for (unsigned int l = 0; l < phaseCurve.at(i).at(j).at(k).size(); l++)
					
						x->phaseCurve[i][j][k][l] = phaseCurve.at(i).at(j).at(k).at(l).toIDLAngle();
										
	 	cout << "Converted successfully" << endl;
			
		
	

	
  		
		
		
			
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
					
						
					x->phaseRms[i][j][k] = phaseRms.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalBandpassRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalBandpassRow::setFromIDL (CalBandpassRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setBasebandName(x.basebandName);
  			
 		
		
	

	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		
			
		setNumBaseline(x.numBaseline);
  			
 		
		
	

	
		
		
			
		setNumAPC(x.numAPC);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		freqLimits .clear();
		for (unsigned int i = 0; i <x.freqLimits.length(); ++i) {
			
			freqLimits.push_back(Frequency (x.freqLimits[i]));
			
		}
			
  		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		antennaNames .clear();
		for (unsigned int i = 0; i <x.antennaNames.length(); ++i) {
			
			antennaNames.push_back(string (x.antennaNames[i]));
			
		}
			
  		
		
	

	
		
		
			
		setRefAntennaName(string (x.refAntennaName));
			
 		
		
	

	
		
		
			
		atmPhaseCorrections .clear();
		for (unsigned int i = 0; i <x.atmPhaseCorrections.length(); ++i) {
			
			atmPhaseCorrections.push_back(x.atmPhaseCorrections[i]);
  			
		}
			
  		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setNumAmpliPoly(x.numAmpliPoly);
  			
 		
		
	

	
		
		
			
		ampliCurve .clear();
		vector <vector< vector<float> > > vvv_aux_ampliCurve;
		vector< vector<float> > vv_aux_ampliCurve;
		vector<float> v_aux_ampliCurve;
		
		for (unsigned int i = 0; i <x.ampliCurve.length(); ++i) {
			vvv_aux_ampliCurve.clear();
			for (unsigned int j = 0; j <x.ampliCurve[0].length(); ++j) { 
				vv_aux_ampliCurve.clear();
				for (unsigned int k = 0; k < x.ampliCurve[0][0].length(); ++k) { 
					v_aux_ampliCurve.clear();
					for (unsigned int l = 0; l <x.ampliCurve[0][0][0].length(); ++l) { 
					
						v_aux_ampliCurve.push_back(x.ampliCurve[i][j][k][l]);
		  			
		  		   }
		  		   vv_aux_ampliCurve.push_back(v_aux_ampliCurve);
		  	   }
		  	   vvv_aux_ampliCurve.push_back(vv_aux_ampliCurve);
		  	}
		  	ampliCurve.push_back(vvv_aux_ampliCurve);
		} 		
			
  		
		
	

	
		
		
			
		ampliRms .clear();
		vector< vector<float> > vv_aux_ampliRms;
		vector<float> v_aux_ampliRms;
		
		for (unsigned int i = 0; i < x.ampliRms.length(); ++i) {
			vv_aux_ampliRms.clear();
			for (unsigned int j = 0; j < x.ampliRms[0].length(); ++j) {
				v_aux_ampliRms.clear();
				for (unsigned int k = 0; k < x.ampliRms[0][0].length(); ++k) {
					
					v_aux_ampliRms.push_back(x.ampliRms[i][j][k]);
		  			
		  		}
		  		vv_aux_ampliRms.push_back(v_aux_ampliRms);
  			}
  			ampliRms.push_back(vv_aux_ampliRms);
		}
			
  		
		
	

	
		
		
			
		setNumPhasePoly(x.numPhasePoly);
  			
 		
		
	

	
		
		
			
		phaseCurve .clear();
		vector <vector< vector<Angle> > > vvv_aux_phaseCurve;
		vector< vector<Angle> > vv_aux_phaseCurve;
		vector<Angle> v_aux_phaseCurve;
		
		for (unsigned int i = 0; i <x.phaseCurve.length(); ++i) {
			vvv_aux_phaseCurve.clear();
			for (unsigned int j = 0; j <x.phaseCurve[0].length(); ++j) { 
				vv_aux_phaseCurve.clear();
				for (unsigned int k = 0; k < x.phaseCurve[0][0].length(); ++k) { 
					v_aux_phaseCurve.clear();
					for (unsigned int l = 0; l <x.phaseCurve[0][0][0].length(); ++l) { 
					
						v_aux_phaseCurve.push_back(Angle (x.phaseCurve[i][j][k][l]));
					
		  		   }
		  		   vv_aux_phaseCurve.push_back(v_aux_phaseCurve);
		  	   }
		  	   vvv_aux_phaseCurve.push_back(vv_aux_phaseCurve);
		  	}
		  	phaseCurve.push_back(vvv_aux_phaseCurve);
		} 		
			
  		
		
	

	
		
		
			
		phaseRms .clear();
		vector< vector<float> > vv_aux_phaseRms;
		vector<float> v_aux_phaseRms;
		
		for (unsigned int i = 0; i < x.phaseRms.length(); ++i) {
			vv_aux_phaseRms.clear();
			for (unsigned int j = 0; j < x.phaseRms[0].length(); ++j) {
				v_aux_phaseRms.clear();
				for (unsigned int k = 0; k < x.phaseRms[0][0].length(); ++k) {
					
					v_aux_phaseRms.push_back(x.phaseRms[i][j][k]);
		  			
		  		}
		  		vv_aux_phaseRms.push_back(v_aux_phaseRms);
  			}
  			phaseRms.push_back(vv_aux_phaseRms);
		}
			
  		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalBandpass");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalBandpassRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
			buf.append(EnumerationParser::toXML("basebandName", basebandName));
		
		
	

  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		
		Parser::toXML(numBaseline, "numBaseline", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAPC, "numAPC", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(freqLimits, "freqLimits", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(antennaNames, "antennaNames", buf);
		
		
	

  	
 		
		
		Parser::toXML(refAntennaName, "refAntennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrections", atmPhaseCorrections));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(numAmpliPoly, "numAmpliPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(ampliCurve, "ampliCurve", buf);
		
		
	

  	
 		
		
		Parser::toXML(ampliRms, "ampliRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPhasePoly, "numPhasePoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseCurve, "phaseCurve", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseRms, "phaseRms", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalBandpassRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
		
		
		
		basebandName = EnumerationParser::getBasebandName("basebandName","CalBandpass",rowDoc);
		
		
		
	

	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setNumBaseline(Parser::getInteger("numBaseline","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setNumAPC(Parser::getInteger("numAPC","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalBandpass",rowDoc));
			
		
	

	
  		
			
					
	  	setFreqLimits(Parser::get1DFrequency("freqLimits","CalBandpass",rowDoc));
	  			
	  		
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalBandpass",rowDoc);
		
		
		
	

	
  		
			
					
	  	setAntennaNames(Parser::get1DString("antennaNames","CalBandpass",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setRefAntennaName(Parser::getString("refAntennaName","CalBandpass",rowDoc));
			
		
	

	
		
		
		
		atmPhaseCorrections = EnumerationParser::getAtmPhaseCorrection1D("atmPhaseCorrections","CalBandpass",rowDoc);			
		
		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalBandpass",rowDoc);			
		
		
		
	

	
  		
			
	  	setNumAmpliPoly(Parser::getInteger("numAmpliPoly","CalBandpass",rowDoc));
			
		
	

	
  		
			
					
	  	setAmpliCurve(Parser::get4DFloat("ampliCurve","CalBandpass",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setAmpliRms(Parser::get3DFloat("ampliRms","CalBandpass",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumPhasePoly(Parser::getInteger("numPhasePoly","CalBandpass",rowDoc));
			
		
	

	
  		
			
					
	  	setPhaseCurve(Parser::get4DAngle("phaseCurve","CalBandpass",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPhaseRms(Parser::get3DFloat("phaseRms","CalBandpass",rowDoc));
	  			
	  		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalBandpass");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName CalBandpassRow::getBasebandName() const {
	
  		return basebandName;
 	}

 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setBasebandName (BasebandNameMod::BasebandName basebandName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("basebandName", "CalBandpass");
		
  		}
  	
 		this->basebandName = basebandName;
	
 	}
	
	

	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int CalBandpassRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	

	
 	/**
 	 * Get numBaseline.
 	 * @return numBaseline as int
 	 */
 	int CalBandpassRow::getNumBaseline() const {
	
  		return numBaseline;
 	}

 	/**
 	 * Set numBaseline with the specified int.
 	 * @param numBaseline The int value to which numBaseline is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumBaseline (int numBaseline)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBaseline = numBaseline;
	
 	}
	
	

	

	
 	/**
 	 * Get numAPC.
 	 * @return numAPC as int
 	 */
 	int CalBandpassRow::getNumAPC() const {
	
  		return numAPC;
 	}

 	/**
 	 * Set numAPC with the specified int.
 	 * @param numAPC The int value to which numAPC is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumAPC (int numAPC)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAPC = numAPC;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalBandpassRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalBandpassRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalBandpassRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get freqLimits.
 	 * @return freqLimits as vector<Frequency >
 	 */
 	vector<Frequency > CalBandpassRow::getFreqLimits() const {
	
  		return freqLimits;
 	}

 	/**
 	 * Set freqLimits with the specified vector<Frequency >.
 	 * @param freqLimits The vector<Frequency > value to which freqLimits is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setFreqLimits (vector<Frequency > freqLimits)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->freqLimits = freqLimits;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalBandpassRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaNames.
 	 * @return antennaNames as vector<string >
 	 */
 	vector<string > CalBandpassRow::getAntennaNames() const {
	
  		return antennaNames;
 	}

 	/**
 	 * Set antennaNames with the specified vector<string >.
 	 * @param antennaNames The vector<string > value to which antennaNames is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setAntennaNames (vector<string > antennaNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaNames = antennaNames;
	
 	}
	
	

	

	
 	/**
 	 * Get refAntennaName.
 	 * @return refAntennaName as string
 	 */
 	string CalBandpassRow::getRefAntennaName() const {
	
  		return refAntennaName;
 	}

 	/**
 	 * Set refAntennaName with the specified string.
 	 * @param refAntennaName The string value to which refAntennaName is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setRefAntennaName (string refAntennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refAntennaName = refAntennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrections.
 	 * @return atmPhaseCorrections as vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >
 	 */
 	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > CalBandpassRow::getAtmPhaseCorrections() const {
	
  		return atmPhaseCorrections;
 	}

 	/**
 	 * Set atmPhaseCorrections with the specified vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >.
 	 * @param atmPhaseCorrections The vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > value to which atmPhaseCorrections is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setAtmPhaseCorrections (vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmPhaseCorrections = atmPhaseCorrections;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalBandpassRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get numAmpliPoly.
 	 * @return numAmpliPoly as int
 	 */
 	int CalBandpassRow::getNumAmpliPoly() const {
	
  		return numAmpliPoly;
 	}

 	/**
 	 * Set numAmpliPoly with the specified int.
 	 * @param numAmpliPoly The int value to which numAmpliPoly is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumAmpliPoly (int numAmpliPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAmpliPoly = numAmpliPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get ampliCurve.
 	 * @return ampliCurve as vector<vector<vector<vector<float > > > >
 	 */
 	vector<vector<vector<vector<float > > > > CalBandpassRow::getAmpliCurve() const {
	
  		return ampliCurve;
 	}

 	/**
 	 * Set ampliCurve with the specified vector<vector<vector<vector<float > > > >.
 	 * @param ampliCurve The vector<vector<vector<vector<float > > > > value to which ampliCurve is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setAmpliCurve (vector<vector<vector<vector<float > > > > ampliCurve)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->ampliCurve = ampliCurve;
	
 	}
	
	

	

	
 	/**
 	 * Get ampliRms.
 	 * @return ampliRms as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > CalBandpassRow::getAmpliRms() const {
	
  		return ampliRms;
 	}

 	/**
 	 * Set ampliRms with the specified vector<vector<vector<float > > >.
 	 * @param ampliRms The vector<vector<vector<float > > > value to which ampliRms is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setAmpliRms (vector<vector<vector<float > > > ampliRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->ampliRms = ampliRms;
	
 	}
	
	

	

	
 	/**
 	 * Get numPhasePoly.
 	 * @return numPhasePoly as int
 	 */
 	int CalBandpassRow::getNumPhasePoly() const {
	
  		return numPhasePoly;
 	}

 	/**
 	 * Set numPhasePoly with the specified int.
 	 * @param numPhasePoly The int value to which numPhasePoly is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumPhasePoly (int numPhasePoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPhasePoly = numPhasePoly;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseCurve.
 	 * @return phaseCurve as vector<vector<vector<vector<Angle > > > >
 	 */
 	vector<vector<vector<vector<Angle > > > > CalBandpassRow::getPhaseCurve() const {
	
  		return phaseCurve;
 	}

 	/**
 	 * Set phaseCurve with the specified vector<vector<vector<vector<Angle > > > >.
 	 * @param phaseCurve The vector<vector<vector<vector<Angle > > > > value to which phaseCurve is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setPhaseCurve (vector<vector<vector<vector<Angle > > > > phaseCurve)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseCurve = phaseCurve;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseRms.
 	 * @return phaseRms as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > CalBandpassRow::getPhaseRms() const {
	
  		return phaseRms;
 	}

 	/**
 	 * Set phaseRms with the specified vector<vector<vector<float > > >.
 	 * @param phaseRms The vector<vector<vector<float > > > value to which phaseRms is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setPhaseRms (vector<vector<vector<float > > > phaseRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseRms = phaseRms;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalBandpassRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalBandpass");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalBandpassRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalBandpass");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalBandpassRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalBandpassRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	/**
	 * Create a CalBandpassRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalBandpassRow::CalBandpassRow (CalBandpassTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
basebandName = CBasebandName::from_int(0);
	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	CalBandpassRow::CalBandpassRow (CalBandpassTable &t, CalBandpassRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			basebandName = row.basebandName;
		
		
		
		
			numAntenna = row.numAntenna;
		
			numBaseline = row.numBaseline;
		
			numAPC = row.numAPC;
		
			numReceptor = row.numReceptor;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			freqLimits = row.freqLimits;
		
			receiverBand = row.receiverBand;
		
			antennaNames = row.antennaNames;
		
			refAntennaName = row.refAntennaName;
		
			atmPhaseCorrections = row.atmPhaseCorrections;
		
			polarizationTypes = row.polarizationTypes;
		
			numAmpliPoly = row.numAmpliPoly;
		
			ampliCurve = row.ampliCurve;
		
			ampliRms = row.ampliRms;
		
			numPhasePoly = row.numPhasePoly;
		
			phaseCurve = row.phaseCurve;
		
			phaseRms = row.phaseRms;
		
		
		
		
		}	
	}

	
	bool CalBandpassRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, BasebandNameMod::BasebandName basebandName, int numAntenna, int numBaseline, int numAPC, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > freqLimits, ReceiverBandMod::ReceiverBand receiverBand, vector<string > antennaNames, string refAntennaName, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, int numAmpliPoly, vector<vector<vector<vector<float > > > > ampliCurve, vector<vector<vector<float > > > ampliRms, int numPhasePoly, vector<vector<vector<vector<Angle > > > > phaseCurve, vector<vector<vector<float > > > phaseRms) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->basebandName == basebandName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->numBaseline == numBaseline);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAPC == numAPC);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->freqLimits == freqLimits);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaNames == antennaNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->refAntennaName == refAntennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrections == atmPhaseCorrections);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAmpliPoly == numAmpliPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->ampliCurve == ampliCurve);
		
		if (!result) return false;
	

	
		
		result = result && (this->ampliRms == ampliRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPhasePoly == numPhasePoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseCurve == phaseCurve);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseRms == phaseRms);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalBandpassRow::compareRequiredValue(int numAntenna, int numBaseline, int numAPC, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > freqLimits, ReceiverBandMod::ReceiverBand receiverBand, vector<string > antennaNames, string refAntennaName, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, int numAmpliPoly, vector<vector<vector<vector<float > > > > ampliCurve, vector<vector<vector<float > > > ampliRms, int numPhasePoly, vector<vector<vector<vector<Angle > > > > phaseCurve, vector<vector<vector<float > > > phaseRms) {
		bool result;
		result = true;
		
	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->numBaseline == numBaseline)) return false;
	

	
		if (!(this->numAPC == numAPC)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->freqLimits == freqLimits)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->antennaNames == antennaNames)) return false;
	

	
		if (!(this->refAntennaName == refAntennaName)) return false;
	

	
		if (!(this->atmPhaseCorrections == atmPhaseCorrections)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->numAmpliPoly == numAmpliPoly)) return false;
	

	
		if (!(this->ampliCurve == ampliCurve)) return false;
	

	
		if (!(this->ampliRms == ampliRms)) return false;
	

	
		if (!(this->numPhasePoly == numPhasePoly)) return false;
	

	
		if (!(this->phaseCurve == phaseCurve)) return false;
	

	
		if (!(this->phaseRms == phaseRms)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalBandpassRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalBandpassRow::equalByRequiredValue(CalBandpassRow* x) {
		
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->numBaseline != x->numBaseline) return false;
			
		if (this->numAPC != x->numAPC) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->freqLimits != x->freqLimits) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->antennaNames != x->antennaNames) return false;
			
		if (this->refAntennaName != x->refAntennaName) return false;
			
		if (this->atmPhaseCorrections != x->atmPhaseCorrections) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->numAmpliPoly != x->numAmpliPoly) return false;
			
		if (this->ampliCurve != x->ampliCurve) return false;
			
		if (this->ampliRms != x->ampliRms) return false;
			
		if (this->numPhasePoly != x->numPhasePoly) return false;
			
		if (this->phaseCurve != x->phaseCurve) return false;
			
		if (this->phaseRms != x->phaseRms) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
