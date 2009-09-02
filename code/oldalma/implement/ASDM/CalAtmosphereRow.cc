
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
 * File CalAtmosphereRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalAtmosphereRow.h>
#include <CalAtmosphereTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalAtmosphereRow;
using asdm::CalAtmosphereTable;

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

	CalAtmosphereRow::~CalAtmosphereRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalAtmosphereTable &CalAtmosphereRow::getTable() const {
		return table;
	}
	
	void CalAtmosphereRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalAtmosphereRowIDL struct.
	 */
	CalAtmosphereRowIDL *CalAtmosphereRow::toIDL() const {
		CalAtmosphereRowIDL *x = new CalAtmosphereRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
				
		x->numFreq = numFreq;
 				
 			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->frequencySpectrum.length(frequencySpectrum.size());
		for (unsigned int i = 0; i < frequencySpectrum.size(); ++i) {
			
			x->frequencySpectrum[i] = frequencySpectrum.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->syscalType = syscalType;
 				
 			
		
	

	
  		
		
		
			
		x->tSysSpectrum.length(tSysSpectrum.size());
		for (unsigned int i = 0; i < tSysSpectrum.size(); i++) {
			x->tSysSpectrum[i].length(tSysSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tSysSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tSysSpectrum.at(i).size(); j++)
					
				x->tSysSpectrum[i][j]= tSysSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		
			
		x->tRecSpectrum.length(tRecSpectrum.size());
		for (unsigned int i = 0; i < tRecSpectrum.size(); i++) {
			x->tRecSpectrum[i].length(tRecSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tRecSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tRecSpectrum.at(i).size(); j++)
					
				x->tRecSpectrum[i][j]= tRecSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		
			
		x->tAtmSpectrum.length(tAtmSpectrum.size());
		for (unsigned int i = 0; i < tAtmSpectrum.size(); i++) {
			x->tAtmSpectrum[i].length(tAtmSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tAtmSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tAtmSpectrum.at(i).size(); j++)
					
				x->tAtmSpectrum[i][j]= tAtmSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		
			
		x->tauSpectrum.length(tauSpectrum.size());
		for (unsigned int i = 0; i < tauSpectrum.size(); i++) {
			x->tauSpectrum[i].length(tauSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tauSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tauSpectrum.at(i).size(); j++)
					
						
				x->tauSpectrum[i][j] = tauSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->sbGainSpectrum.length(sbGainSpectrum.size());
		for (unsigned int i = 0; i < sbGainSpectrum.size(); i++) {
			x->sbGainSpectrum[i].length(sbGainSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < sbGainSpectrum.size() ; i++)
			for (unsigned int j = 0; j < sbGainSpectrum.at(i).size(); j++)
					
						
				x->sbGainSpectrum[i][j] = sbGainSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->forwardEffSpectrum.length(forwardEffSpectrum.size());
		for (unsigned int i = 0; i < forwardEffSpectrum.size(); i++) {
			x->forwardEffSpectrum[i].length(forwardEffSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < forwardEffSpectrum.size() ; i++)
			for (unsigned int j = 0; j < forwardEffSpectrum.at(i).size(); j++)
					
						
				x->forwardEffSpectrum[i][j] = forwardEffSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->tSysExists = tSysExists;
		
		
			
		x->tSys.length(tSys.size());
		for (unsigned int i = 0; i < tSys.size(); ++i) {
			
			x->tSys[i] = tSys.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tRecExists = tRecExists;
		
		
			
		x->tRec.length(tRec.size());
		for (unsigned int i = 0; i < tRec.size(); ++i) {
			
			x->tRec[i] = tRec.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tAtmExists = tAtmExists;
		
		
			
		x->tAtm.length(tAtm.size());
		for (unsigned int i = 0; i < tAtm.size(); ++i) {
			
			x->tAtm[i] = tAtm.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->sbGainExists = sbGainExists;
		
		
			
		x->sbGain.length(sbGain.size());
		for (unsigned int i = 0; i < sbGain.size(); ++i) {
			
				
			x->sbGain[i] = sbGain.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->waterExists = waterExists;
		
		
			
		x->water.length(water.size());
		for (unsigned int i = 0; i < water.size(); ++i) {
			
			x->water[i] = water.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		x->forwardEfficiencyExists = forwardEfficiencyExists;
		
		
			
		x->forwardEfficiency.length(forwardEfficiency.size());
		for (unsigned int i = 0; i < forwardEfficiency.size(); ++i) {
			
				
			x->forwardEfficiency[i] = forwardEfficiency.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->tauExists = tauExists;
		
		
			
		x->tau.length(tau.size());
		for (unsigned int i = 0; i < tau.size(); ++i) {
			
				
			x->tau[i] = tau.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->groundPressure = groundPressure.toIDLPressure();
			
		
	

	
  		
		
		
			
		x->groundTemperature = groundTemperature.toIDLTemperature();
			
		
	

	
  		
		
		
			
		x->groundRelHumidity = groundRelHumidity.toIDLHumidity();
			
		
	

	
  		
		
		
			
				
		x->subType = CORBA::string_dup(subType.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalAtmosphereRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalAtmosphereRow::setFromIDL (CalAtmosphereRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setNumFreq(x.numFreq);
  			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		frequencySpectrum .clear();
		for (unsigned int i = 0; i <x.frequencySpectrum.length(); ++i) {
			
			frequencySpectrum.push_back(Frequency (x.frequencySpectrum[i]));
			
		}
			
  		
		
	

	
		
		
			
		setSyscalType(x.syscalType);
  			
 		
		
	

	
		
		
			
		tSysSpectrum .clear();
		vector<Temperature> v_aux_tSysSpectrum;
		for (unsigned int i = 0; i < x.tSysSpectrum.length(); ++i) {
			v_aux_tSysSpectrum.clear();
			for (unsigned int j = 0; j < x.tSysSpectrum[0].length(); ++j) {
				
				v_aux_tSysSpectrum.push_back(Temperature (x.tSysSpectrum[i][j]));
				
  			}
  			tSysSpectrum.push_back(v_aux_tSysSpectrum);			
		}
			
  		
		
	

	
		
		
			
		tRecSpectrum .clear();
		vector<Temperature> v_aux_tRecSpectrum;
		for (unsigned int i = 0; i < x.tRecSpectrum.length(); ++i) {
			v_aux_tRecSpectrum.clear();
			for (unsigned int j = 0; j < x.tRecSpectrum[0].length(); ++j) {
				
				v_aux_tRecSpectrum.push_back(Temperature (x.tRecSpectrum[i][j]));
				
  			}
  			tRecSpectrum.push_back(v_aux_tRecSpectrum);			
		}
			
  		
		
	

	
		
		
			
		tAtmSpectrum .clear();
		vector<Temperature> v_aux_tAtmSpectrum;
		for (unsigned int i = 0; i < x.tAtmSpectrum.length(); ++i) {
			v_aux_tAtmSpectrum.clear();
			for (unsigned int j = 0; j < x.tAtmSpectrum[0].length(); ++j) {
				
				v_aux_tAtmSpectrum.push_back(Temperature (x.tAtmSpectrum[i][j]));
				
  			}
  			tAtmSpectrum.push_back(v_aux_tAtmSpectrum);			
		}
			
  		
		
	

	
		
		
			
		tauSpectrum .clear();
		vector<float> v_aux_tauSpectrum;
		for (unsigned int i = 0; i < x.tauSpectrum.length(); ++i) {
			v_aux_tauSpectrum.clear();
			for (unsigned int j = 0; j < x.tauSpectrum[0].length(); ++j) {
				
				v_aux_tauSpectrum.push_back(x.tauSpectrum[i][j]);
	  			
  			}
  			tauSpectrum.push_back(v_aux_tauSpectrum);			
		}
			
  		
		
	

	
		
		
			
		sbGainSpectrum .clear();
		vector<float> v_aux_sbGainSpectrum;
		for (unsigned int i = 0; i < x.sbGainSpectrum.length(); ++i) {
			v_aux_sbGainSpectrum.clear();
			for (unsigned int j = 0; j < x.sbGainSpectrum[0].length(); ++j) {
				
				v_aux_sbGainSpectrum.push_back(x.sbGainSpectrum[i][j]);
	  			
  			}
  			sbGainSpectrum.push_back(v_aux_sbGainSpectrum);			
		}
			
  		
		
	

	
		
		
			
		forwardEffSpectrum .clear();
		vector<float> v_aux_forwardEffSpectrum;
		for (unsigned int i = 0; i < x.forwardEffSpectrum.length(); ++i) {
			v_aux_forwardEffSpectrum.clear();
			for (unsigned int j = 0; j < x.forwardEffSpectrum[0].length(); ++j) {
				
				v_aux_forwardEffSpectrum.push_back(x.forwardEffSpectrum[i][j]);
	  			
  			}
  			forwardEffSpectrum.push_back(v_aux_forwardEffSpectrum);			
		}
			
  		
		
	

	
		
		tSysExists = x.tSysExists;
		if (x.tSysExists) {
		
		
			
		tSys .clear();
		for (unsigned int i = 0; i <x.tSys.length(); ++i) {
			
			tSys.push_back(Temperature (x.tSys[i]));
			
		}
			
  		
		
		}
		
	

	
		
		tRecExists = x.tRecExists;
		if (x.tRecExists) {
		
		
			
		tRec .clear();
		for (unsigned int i = 0; i <x.tRec.length(); ++i) {
			
			tRec.push_back(Temperature (x.tRec[i]));
			
		}
			
  		
		
		}
		
	

	
		
		tAtmExists = x.tAtmExists;
		if (x.tAtmExists) {
		
		
			
		tAtm .clear();
		for (unsigned int i = 0; i <x.tAtm.length(); ++i) {
			
			tAtm.push_back(Temperature (x.tAtm[i]));
			
		}
			
  		
		
		}
		
	

	
		
		sbGainExists = x.sbGainExists;
		if (x.sbGainExists) {
		
		
			
		sbGain .clear();
		for (unsigned int i = 0; i <x.sbGain.length(); ++i) {
			
			sbGain.push_back(x.sbGain[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		waterExists = x.waterExists;
		if (x.waterExists) {
		
		
			
		water .clear();
		for (unsigned int i = 0; i <x.water.length(); ++i) {
			
			water.push_back(Length (x.water[i]));
			
		}
			
  		
		
		}
		
	

	
		
		forwardEfficiencyExists = x.forwardEfficiencyExists;
		if (x.forwardEfficiencyExists) {
		
		
			
		forwardEfficiency .clear();
		for (unsigned int i = 0; i <x.forwardEfficiency.length(); ++i) {
			
			forwardEfficiency.push_back(x.forwardEfficiency[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		tauExists = x.tauExists;
		if (x.tauExists) {
		
		
			
		tau .clear();
		for (unsigned int i = 0; i <x.tau.length(); ++i) {
			
			tau.push_back(x.tau[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		
			
		setGroundPressure(Pressure (x.groundPressure));
			
 		
		
	

	
		
		
			
		setGroundTemperature(Temperature (x.groundTemperature));
			
 		
		
	

	
		
		
			
		setGroundRelHumidity(Humidity (x.groundRelHumidity));
			
 		
		
	

	
		
		
			
		setSubType(string (x.subType));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalAtmosphere");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalAtmosphereRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(numFreq, "numFreq", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencySpectrum, "frequencySpectrum", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("syscalType", syscalType));
		
		
	

  	
 		
		
		Parser::toXML(tSysSpectrum, "tSysSpectrum", buf);
		
		
	

  	
 		
		
		Parser::toXML(tRecSpectrum, "tRecSpectrum", buf);
		
		
	

  	
 		
		
		Parser::toXML(tAtmSpectrum, "tAtmSpectrum", buf);
		
		
	

  	
 		
		
		Parser::toXML(tauSpectrum, "tauSpectrum", buf);
		
		
	

  	
 		
		
		Parser::toXML(sbGainSpectrum, "sbGainSpectrum", buf);
		
		
	

  	
 		
		
		Parser::toXML(forwardEffSpectrum, "forwardEffSpectrum", buf);
		
		
	

  	
 		
		if (tSysExists) {
		
		
		Parser::toXML(tSys, "tSys", buf);
		
		
		}
		
	

  	
 		
		if (tRecExists) {
		
		
		Parser::toXML(tRec, "tRec", buf);
		
		
		}
		
	

  	
 		
		if (tAtmExists) {
		
		
		Parser::toXML(tAtm, "tAtm", buf);
		
		
		}
		
	

  	
 		
		if (sbGainExists) {
		
		
		Parser::toXML(sbGain, "sbGain", buf);
		
		
		}
		
	

  	
 		
		if (waterExists) {
		
		
		Parser::toXML(water, "water", buf);
		
		
		}
		
	

  	
 		
		if (forwardEfficiencyExists) {
		
		
		Parser::toXML(forwardEfficiency, "forwardEfficiency", buf);
		
		
		}
		
	

  	
 		
		if (tauExists) {
		
		
		Parser::toXML(tau, "tau", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(groundPressure, "groundPressure", buf);
		
		
	

  	
 		
		
		Parser::toXML(groundTemperature, "groundTemperature", buf);
		
		
	

  	
 		
		
		Parser::toXML(groundRelHumidity, "groundRelHumidity", buf);
		
		
	

  	
 		
		
		Parser::toXML(subType, "subType", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

	
	
		
  	
 		
		
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
	void CalAtmosphereRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalAtmosphere",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalAtmosphere",rowDoc));
			
		
	

	
  		
			
	  	setNumFreq(Parser::getInteger("numFreq","CalAtmosphere",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalAtmosphere",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalAtmosphere",rowDoc));
			
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalAtmosphere",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFrequencySpectrum(Parser::get1DFrequency("frequencySpectrum","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
		
		
		
		syscalType = EnumerationParser::getSyscalMethod("syscalType","CalAtmosphere",rowDoc);
		
		
		
	

	
  		
			
					
	  	setTSysSpectrum(Parser::get2DTemperature("tSysSpectrum","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setTRecSpectrum(Parser::get2DTemperature("tRecSpectrum","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setTAtmSpectrum(Parser::get2DTemperature("tAtmSpectrum","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setTauSpectrum(Parser::get2DFloat("tauSpectrum","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setSbGainSpectrum(Parser::get2DFloat("sbGainSpectrum","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setForwardEffSpectrum(Parser::get2DFloat("forwardEffSpectrum","CalAtmosphere",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<tSys>")) {
			
								
	  		setTSys(Parser::get1DTemperature("tSys","CalAtmosphere",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tRec>")) {
			
								
	  		setTRec(Parser::get1DTemperature("tRec","CalAtmosphere",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tAtm>")) {
			
								
	  		setTAtm(Parser::get1DTemperature("tAtm","CalAtmosphere",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<sbGain>")) {
			
								
	  		setSbGain(Parser::get1DFloat("sbGain","CalAtmosphere",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<water>")) {
			
								
	  		setWater(Parser::get1DLength("water","CalAtmosphere",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<forwardEfficiency>")) {
			
								
	  		setForwardEfficiency(Parser::get1DFloat("forwardEfficiency","CalAtmosphere",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tau>")) {
			
								
	  		setTau(Parser::get1DFloat("tau","CalAtmosphere",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
	  	setGroundPressure(Parser::getPressure("groundPressure","CalAtmosphere",rowDoc));
			
		
	

	
  		
			
	  	setGroundTemperature(Parser::getTemperature("groundTemperature","CalAtmosphere",rowDoc));
			
		
	

	
  		
			
	  	setGroundRelHumidity(Parser::getHumidity("groundRelHumidity","CalAtmosphere",rowDoc));
			
		
	

	
  		
			
	  	setSubType(Parser::getString("subType","CalAtmosphere",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalAtmosphere",rowDoc);
		
		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalAtmosphere");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalAtmosphereRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAtmosphereRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalAtmosphere");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalAtmosphereRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get numFreq.
 	 * @return numFreq as int
 	 */
 	int CalAtmosphereRow::getNumFreq() const {
	
  		return numFreq;
 	}

 	/**
 	 * Set numFreq with the specified int.
 	 * @param numFreq The int value to which numFreq is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setNumFreq (int numFreq)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numFreq = numFreq;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalAtmosphereRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalAtmosphereRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalAtmosphereRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalAtmosphereRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencySpectrum.
 	 * @return frequencySpectrum as vector<Frequency >
 	 */
 	vector<Frequency > CalAtmosphereRow::getFrequencySpectrum() const {
	
  		return frequencySpectrum;
 	}

 	/**
 	 * Set frequencySpectrum with the specified vector<Frequency >.
 	 * @param frequencySpectrum The vector<Frequency > value to which frequencySpectrum is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setFrequencySpectrum (vector<Frequency > frequencySpectrum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencySpectrum = frequencySpectrum;
	
 	}
	
	

	

	
 	/**
 	 * Get syscalType.
 	 * @return syscalType as SyscalMethodMod::SyscalMethod
 	 */
 	SyscalMethodMod::SyscalMethod CalAtmosphereRow::getSyscalType() const {
	
  		return syscalType;
 	}

 	/**
 	 * Set syscalType with the specified SyscalMethodMod::SyscalMethod.
 	 * @param syscalType The SyscalMethodMod::SyscalMethod value to which syscalType is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setSyscalType (SyscalMethodMod::SyscalMethod syscalType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->syscalType = syscalType;
	
 	}
	
	

	

	
 	/**
 	 * Get tSysSpectrum.
 	 * @return tSysSpectrum as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > CalAtmosphereRow::getTSysSpectrum() const {
	
  		return tSysSpectrum;
 	}

 	/**
 	 * Set tSysSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tSysSpectrum The vector<vector<Temperature > > value to which tSysSpectrum is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setTSysSpectrum (vector<vector<Temperature > > tSysSpectrum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->tSysSpectrum = tSysSpectrum;
	
 	}
	
	

	

	
 	/**
 	 * Get tRecSpectrum.
 	 * @return tRecSpectrum as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > CalAtmosphereRow::getTRecSpectrum() const {
	
  		return tRecSpectrum;
 	}

 	/**
 	 * Set tRecSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tRecSpectrum The vector<vector<Temperature > > value to which tRecSpectrum is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setTRecSpectrum (vector<vector<Temperature > > tRecSpectrum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->tRecSpectrum = tRecSpectrum;
	
 	}
	
	

	

	
 	/**
 	 * Get tAtmSpectrum.
 	 * @return tAtmSpectrum as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > CalAtmosphereRow::getTAtmSpectrum() const {
	
  		return tAtmSpectrum;
 	}

 	/**
 	 * Set tAtmSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tAtmSpectrum The vector<vector<Temperature > > value to which tAtmSpectrum is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setTAtmSpectrum (vector<vector<Temperature > > tAtmSpectrum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->tAtmSpectrum = tAtmSpectrum;
	
 	}
	
	

	

	
 	/**
 	 * Get tauSpectrum.
 	 * @return tauSpectrum as vector<vector<float > >
 	 */
 	vector<vector<float > > CalAtmosphereRow::getTauSpectrum() const {
	
  		return tauSpectrum;
 	}

 	/**
 	 * Set tauSpectrum with the specified vector<vector<float > >.
 	 * @param tauSpectrum The vector<vector<float > > value to which tauSpectrum is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setTauSpectrum (vector<vector<float > > tauSpectrum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->tauSpectrum = tauSpectrum;
	
 	}
	
	

	

	
 	/**
 	 * Get sbGainSpectrum.
 	 * @return sbGainSpectrum as vector<vector<float > >
 	 */
 	vector<vector<float > > CalAtmosphereRow::getSbGainSpectrum() const {
	
  		return sbGainSpectrum;
 	}

 	/**
 	 * Set sbGainSpectrum with the specified vector<vector<float > >.
 	 * @param sbGainSpectrum The vector<vector<float > > value to which sbGainSpectrum is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setSbGainSpectrum (vector<vector<float > > sbGainSpectrum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbGainSpectrum = sbGainSpectrum;
	
 	}
	
	

	

	
 	/**
 	 * Get forwardEffSpectrum.
 	 * @return forwardEffSpectrum as vector<vector<float > >
 	 */
 	vector<vector<float > > CalAtmosphereRow::getForwardEffSpectrum() const {
	
  		return forwardEffSpectrum;
 	}

 	/**
 	 * Set forwardEffSpectrum with the specified vector<vector<float > >.
 	 * @param forwardEffSpectrum The vector<vector<float > > value to which forwardEffSpectrum is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setForwardEffSpectrum (vector<vector<float > > forwardEffSpectrum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->forwardEffSpectrum = forwardEffSpectrum;
	
 	}
	
	

	
	/**
	 * The attribute tSys is optional. Return true if this attribute exists.
	 * @return true if and only if the tSys attribute exists. 
	 */
	bool CalAtmosphereRow::isTSysExists() const {
		return tSysExists;
	}
	

	
 	/**
 	 * Get tSys, which is optional.
 	 * @return tSys as vector<Temperature >
 	 * @throw IllegalAccessException If tSys does not exist.
 	 */
 	vector<Temperature > CalAtmosphereRow::getTSys() const throw(IllegalAccessException) {
		if (!tSysExists) {
			throw IllegalAccessException("tSys", "CalAtmosphere");
		}
	
  		return tSys;
 	}

 	/**
 	 * Set tSys with the specified vector<Temperature >.
 	 * @param tSys The vector<Temperature > value to which tSys is to be set.
 	 
 	
 	 */
 	void CalAtmosphereRow::setTSys (vector<Temperature > tSys) {
	
 		this->tSys = tSys;
	
		tSysExists = true;
	
 	}
	
	
	/**
	 * Mark tSys, which is an optional field, as non-existent.
	 */
	void CalAtmosphereRow::clearTSys () {
		tSysExists = false;
	}
	

	
	/**
	 * The attribute tRec is optional. Return true if this attribute exists.
	 * @return true if and only if the tRec attribute exists. 
	 */
	bool CalAtmosphereRow::isTRecExists() const {
		return tRecExists;
	}
	

	
 	/**
 	 * Get tRec, which is optional.
 	 * @return tRec as vector<Temperature >
 	 * @throw IllegalAccessException If tRec does not exist.
 	 */
 	vector<Temperature > CalAtmosphereRow::getTRec() const throw(IllegalAccessException) {
		if (!tRecExists) {
			throw IllegalAccessException("tRec", "CalAtmosphere");
		}
	
  		return tRec;
 	}

 	/**
 	 * Set tRec with the specified vector<Temperature >.
 	 * @param tRec The vector<Temperature > value to which tRec is to be set.
 	 
 	
 	 */
 	void CalAtmosphereRow::setTRec (vector<Temperature > tRec) {
	
 		this->tRec = tRec;
	
		tRecExists = true;
	
 	}
	
	
	/**
	 * Mark tRec, which is an optional field, as non-existent.
	 */
	void CalAtmosphereRow::clearTRec () {
		tRecExists = false;
	}
	

	
	/**
	 * The attribute tAtm is optional. Return true if this attribute exists.
	 * @return true if and only if the tAtm attribute exists. 
	 */
	bool CalAtmosphereRow::isTAtmExists() const {
		return tAtmExists;
	}
	

	
 	/**
 	 * Get tAtm, which is optional.
 	 * @return tAtm as vector<Temperature >
 	 * @throw IllegalAccessException If tAtm does not exist.
 	 */
 	vector<Temperature > CalAtmosphereRow::getTAtm() const throw(IllegalAccessException) {
		if (!tAtmExists) {
			throw IllegalAccessException("tAtm", "CalAtmosphere");
		}
	
  		return tAtm;
 	}

 	/**
 	 * Set tAtm with the specified vector<Temperature >.
 	 * @param tAtm The vector<Temperature > value to which tAtm is to be set.
 	 
 	
 	 */
 	void CalAtmosphereRow::setTAtm (vector<Temperature > tAtm) {
	
 		this->tAtm = tAtm;
	
		tAtmExists = true;
	
 	}
	
	
	/**
	 * Mark tAtm, which is an optional field, as non-existent.
	 */
	void CalAtmosphereRow::clearTAtm () {
		tAtmExists = false;
	}
	

	
	/**
	 * The attribute sbGain is optional. Return true if this attribute exists.
	 * @return true if and only if the sbGain attribute exists. 
	 */
	bool CalAtmosphereRow::isSbGainExists() const {
		return sbGainExists;
	}
	

	
 	/**
 	 * Get sbGain, which is optional.
 	 * @return sbGain as vector<float >
 	 * @throw IllegalAccessException If sbGain does not exist.
 	 */
 	vector<float > CalAtmosphereRow::getSbGain() const throw(IllegalAccessException) {
		if (!sbGainExists) {
			throw IllegalAccessException("sbGain", "CalAtmosphere");
		}
	
  		return sbGain;
 	}

 	/**
 	 * Set sbGain with the specified vector<float >.
 	 * @param sbGain The vector<float > value to which sbGain is to be set.
 	 
 	
 	 */
 	void CalAtmosphereRow::setSbGain (vector<float > sbGain) {
	
 		this->sbGain = sbGain;
	
		sbGainExists = true;
	
 	}
	
	
	/**
	 * Mark sbGain, which is an optional field, as non-existent.
	 */
	void CalAtmosphereRow::clearSbGain () {
		sbGainExists = false;
	}
	

	
	/**
	 * The attribute water is optional. Return true if this attribute exists.
	 * @return true if and only if the water attribute exists. 
	 */
	bool CalAtmosphereRow::isWaterExists() const {
		return waterExists;
	}
	

	
 	/**
 	 * Get water, which is optional.
 	 * @return water as vector<Length >
 	 * @throw IllegalAccessException If water does not exist.
 	 */
 	vector<Length > CalAtmosphereRow::getWater() const throw(IllegalAccessException) {
		if (!waterExists) {
			throw IllegalAccessException("water", "CalAtmosphere");
		}
	
  		return water;
 	}

 	/**
 	 * Set water with the specified vector<Length >.
 	 * @param water The vector<Length > value to which water is to be set.
 	 
 	
 	 */
 	void CalAtmosphereRow::setWater (vector<Length > water) {
	
 		this->water = water;
	
		waterExists = true;
	
 	}
	
	
	/**
	 * Mark water, which is an optional field, as non-existent.
	 */
	void CalAtmosphereRow::clearWater () {
		waterExists = false;
	}
	

	
	/**
	 * The attribute forwardEfficiency is optional. Return true if this attribute exists.
	 * @return true if and only if the forwardEfficiency attribute exists. 
	 */
	bool CalAtmosphereRow::isForwardEfficiencyExists() const {
		return forwardEfficiencyExists;
	}
	

	
 	/**
 	 * Get forwardEfficiency, which is optional.
 	 * @return forwardEfficiency as vector<float >
 	 * @throw IllegalAccessException If forwardEfficiency does not exist.
 	 */
 	vector<float > CalAtmosphereRow::getForwardEfficiency() const throw(IllegalAccessException) {
		if (!forwardEfficiencyExists) {
			throw IllegalAccessException("forwardEfficiency", "CalAtmosphere");
		}
	
  		return forwardEfficiency;
 	}

 	/**
 	 * Set forwardEfficiency with the specified vector<float >.
 	 * @param forwardEfficiency The vector<float > value to which forwardEfficiency is to be set.
 	 
 	
 	 */
 	void CalAtmosphereRow::setForwardEfficiency (vector<float > forwardEfficiency) {
	
 		this->forwardEfficiency = forwardEfficiency;
	
		forwardEfficiencyExists = true;
	
 	}
	
	
	/**
	 * Mark forwardEfficiency, which is an optional field, as non-existent.
	 */
	void CalAtmosphereRow::clearForwardEfficiency () {
		forwardEfficiencyExists = false;
	}
	

	
	/**
	 * The attribute tau is optional. Return true if this attribute exists.
	 * @return true if and only if the tau attribute exists. 
	 */
	bool CalAtmosphereRow::isTauExists() const {
		return tauExists;
	}
	

	
 	/**
 	 * Get tau, which is optional.
 	 * @return tau as vector<float >
 	 * @throw IllegalAccessException If tau does not exist.
 	 */
 	vector<float > CalAtmosphereRow::getTau() const throw(IllegalAccessException) {
		if (!tauExists) {
			throw IllegalAccessException("tau", "CalAtmosphere");
		}
	
  		return tau;
 	}

 	/**
 	 * Set tau with the specified vector<float >.
 	 * @param tau The vector<float > value to which tau is to be set.
 	 
 	
 	 */
 	void CalAtmosphereRow::setTau (vector<float > tau) {
	
 		this->tau = tau;
	
		tauExists = true;
	
 	}
	
	
	/**
	 * Mark tau, which is an optional field, as non-existent.
	 */
	void CalAtmosphereRow::clearTau () {
		tauExists = false;
	}
	

	

	
 	/**
 	 * Get groundPressure.
 	 * @return groundPressure as Pressure
 	 */
 	Pressure CalAtmosphereRow::getGroundPressure() const {
	
  		return groundPressure;
 	}

 	/**
 	 * Set groundPressure with the specified Pressure.
 	 * @param groundPressure The Pressure value to which groundPressure is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setGroundPressure (Pressure groundPressure)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->groundPressure = groundPressure;
	
 	}
	
	

	

	
 	/**
 	 * Get groundTemperature.
 	 * @return groundTemperature as Temperature
 	 */
 	Temperature CalAtmosphereRow::getGroundTemperature() const {
	
  		return groundTemperature;
 	}

 	/**
 	 * Set groundTemperature with the specified Temperature.
 	 * @param groundTemperature The Temperature value to which groundTemperature is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setGroundTemperature (Temperature groundTemperature)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->groundTemperature = groundTemperature;
	
 	}
	
	

	

	
 	/**
 	 * Get groundRelHumidity.
 	 * @return groundRelHumidity as Humidity
 	 */
 	Humidity CalAtmosphereRow::getGroundRelHumidity() const {
	
  		return groundRelHumidity;
 	}

 	/**
 	 * Set groundRelHumidity with the specified Humidity.
 	 * @param groundRelHumidity The Humidity value to which groundRelHumidity is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setGroundRelHumidity (Humidity groundRelHumidity)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->groundRelHumidity = groundRelHumidity;
	
 	}
	
	

	

	
 	/**
 	 * Get subType.
 	 * @return subType as string
 	 */
 	string CalAtmosphereRow::getSubType() const {
	
  		return subType;
 	}

 	/**
 	 * Set subType with the specified string.
 	 * @param subType The string value to which subType is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setSubType (string subType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->subType = subType;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalAtmosphereRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalAtmosphereRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalAtmosphereRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAtmosphereRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalAtmosphere");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalAtmosphereRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAtmosphereRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalAtmosphere");
		
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
	 CalDataRow* CalAtmosphereRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalAtmosphereRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalAtmosphereRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalAtmosphereRow::CalAtmosphereRow (CalAtmosphereTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		tSysExists = false;
	

	
		tRecExists = false;
	

	
		tAtmExists = false;
	

	
		sbGainExists = false;
	

	
		waterExists = false;
	

	
		forwardEfficiencyExists = false;
	

	
		tauExists = false;
	

	

	

	

	

	

	
	

	

	
	
	
	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
syscalType = CSyscalMethod::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	
	
	}
	
	CalAtmosphereRow::CalAtmosphereRow (CalAtmosphereTable &t, CalAtmosphereRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		tSysExists = false;
	

	
		tRecExists = false;
	

	
		tAtmExists = false;
	

	
		sbGainExists = false;
	

	
		waterExists = false;
	

	
		forwardEfficiencyExists = false;
	

	
		tauExists = false;
	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
		
		
		
			numReceptor = row.numReceptor;
		
			numFreq = row.numFreq;
		
			endValidTime = row.endValidTime;
		
			startValidTime = row.startValidTime;
		
			polarizationTypes = row.polarizationTypes;
		
			frequencyRange = row.frequencyRange;
		
			frequencySpectrum = row.frequencySpectrum;
		
			syscalType = row.syscalType;
		
			tSysSpectrum = row.tSysSpectrum;
		
			tRecSpectrum = row.tRecSpectrum;
		
			tAtmSpectrum = row.tAtmSpectrum;
		
			tauSpectrum = row.tauSpectrum;
		
			sbGainSpectrum = row.sbGainSpectrum;
		
			forwardEffSpectrum = row.forwardEffSpectrum;
		
			groundPressure = row.groundPressure;
		
			groundTemperature = row.groundTemperature;
		
			groundRelHumidity = row.groundRelHumidity;
		
			subType = row.subType;
		
			receiverBand = row.receiverBand;
		
		
		
		
		if (row.tSysExists) {
			tSys = row.tSys;		
			tSysExists = true;
		}
		else
			tSysExists = false;
		
		if (row.tRecExists) {
			tRec = row.tRec;		
			tRecExists = true;
		}
		else
			tRecExists = false;
		
		if (row.tAtmExists) {
			tAtm = row.tAtm;		
			tAtmExists = true;
		}
		else
			tAtmExists = false;
		
		if (row.sbGainExists) {
			sbGain = row.sbGain;		
			sbGainExists = true;
		}
		else
			sbGainExists = false;
		
		if (row.waterExists) {
			water = row.water;		
			waterExists = true;
		}
		else
			waterExists = false;
		
		if (row.forwardEfficiencyExists) {
			forwardEfficiency = row.forwardEfficiency;		
			forwardEfficiencyExists = true;
		}
		else
			forwardEfficiencyExists = false;
		
		if (row.tauExists) {
			tau = row.tau;		
			tauExists = true;
		}
		else
			tauExists = false;
		
		}	
	}

	
	bool CalAtmosphereRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, int numReceptor, int numFreq, ArrayTime endValidTime, ArrayTime startValidTime, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<Frequency > frequencyRange, vector<Frequency > frequencySpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tSysSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tAtmSpectrum, vector<vector<float > > tauSpectrum, vector<vector<float > > sbGainSpectrum, vector<vector<float > > forwardEffSpectrum, Pressure groundPressure, Temperature groundTemperature, Humidity groundRelHumidity, string subType, ReceiverBandMod::ReceiverBand receiverBand) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->numFreq == numFreq);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencySpectrum == frequencySpectrum);
		
		if (!result) return false;
	

	
		
		result = result && (this->syscalType == syscalType);
		
		if (!result) return false;
	

	
		
		result = result && (this->tSysSpectrum == tSysSpectrum);
		
		if (!result) return false;
	

	
		
		result = result && (this->tRecSpectrum == tRecSpectrum);
		
		if (!result) return false;
	

	
		
		result = result && (this->tAtmSpectrum == tAtmSpectrum);
		
		if (!result) return false;
	

	
		
		result = result && (this->tauSpectrum == tauSpectrum);
		
		if (!result) return false;
	

	
		
		result = result && (this->sbGainSpectrum == sbGainSpectrum);
		
		if (!result) return false;
	

	
		
		result = result && (this->forwardEffSpectrum == forwardEffSpectrum);
		
		if (!result) return false;
	

	
		
		result = result && (this->groundPressure == groundPressure);
		
		if (!result) return false;
	

	
		
		result = result && (this->groundTemperature == groundTemperature);
		
		if (!result) return false;
	

	
		
		result = result && (this->groundRelHumidity == groundRelHumidity);
		
		if (!result) return false;
	

	
		
		result = result && (this->subType == subType);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalAtmosphereRow::compareRequiredValue(int numReceptor, int numFreq, ArrayTime endValidTime, ArrayTime startValidTime, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<Frequency > frequencyRange, vector<Frequency > frequencySpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tSysSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tAtmSpectrum, vector<vector<float > > tauSpectrum, vector<vector<float > > sbGainSpectrum, vector<vector<float > > forwardEffSpectrum, Pressure groundPressure, Temperature groundTemperature, Humidity groundRelHumidity, string subType, ReceiverBandMod::ReceiverBand receiverBand) {
		bool result;
		result = true;
		
	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->numFreq == numFreq)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->frequencySpectrum == frequencySpectrum)) return false;
	

	
		if (!(this->syscalType == syscalType)) return false;
	

	
		if (!(this->tSysSpectrum == tSysSpectrum)) return false;
	

	
		if (!(this->tRecSpectrum == tRecSpectrum)) return false;
	

	
		if (!(this->tAtmSpectrum == tAtmSpectrum)) return false;
	

	
		if (!(this->tauSpectrum == tauSpectrum)) return false;
	

	
		if (!(this->sbGainSpectrum == sbGainSpectrum)) return false;
	

	
		if (!(this->forwardEffSpectrum == forwardEffSpectrum)) return false;
	

	
		if (!(this->groundPressure == groundPressure)) return false;
	

	
		if (!(this->groundTemperature == groundTemperature)) return false;
	

	
		if (!(this->groundRelHumidity == groundRelHumidity)) return false;
	

	
		if (!(this->subType == subType)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalAtmosphereRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalAtmosphereRow::equalByRequiredValue(CalAtmosphereRow* x) {
		
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->numFreq != x->numFreq) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->frequencySpectrum != x->frequencySpectrum) return false;
			
		if (this->syscalType != x->syscalType) return false;
			
		if (this->tSysSpectrum != x->tSysSpectrum) return false;
			
		if (this->tRecSpectrum != x->tRecSpectrum) return false;
			
		if (this->tAtmSpectrum != x->tAtmSpectrum) return false;
			
		if (this->tauSpectrum != x->tauSpectrum) return false;
			
		if (this->sbGainSpectrum != x->sbGainSpectrum) return false;
			
		if (this->forwardEffSpectrum != x->forwardEffSpectrum) return false;
			
		if (this->groundPressure != x->groundPressure) return false;
			
		if (this->groundTemperature != x->groundTemperature) return false;
			
		if (this->groundRelHumidity != x->groundRelHumidity) return false;
			
		if (this->subType != x->subType) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
