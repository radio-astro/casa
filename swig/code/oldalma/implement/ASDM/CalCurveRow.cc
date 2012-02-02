
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
 * File CalCurveRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalCurveRow.h>
#include <CalCurveTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalCurveRow;
using asdm::CalCurveTable;

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

	CalCurveRow::~CalCurveRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalCurveTable &CalCurveRow::getTable() const {
		return table;
	}
	
	void CalCurveRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalCurveRowIDL struct.
	 */
	CalCurveRowIDL *CalCurveRow::toIDL() const {
		CalCurveRowIDL *x = new CalCurveRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		
			
				
		x->numBaseline = numBaseline;
 				
 			
		
	

	
  		
		
		
			
				
		x->numAPC = numAPC;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
		x->antennaNames.length(antennaNames.size());
		for (unsigned int i = 0; i < antennaNames.size(); ++i) {
			
				
			x->antennaNames[i] = CORBA::string_dup(antennaNames.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->refAntennaName = CORBA::string_dup(refAntennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
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
			
		
	

	
  		
		
		
			
				
		x->typeCurve = typeCurve;
 				
 			
		
	

	
  		
		
		
			
		x->timeOrigin = timeOrigin.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->curve.length(curve.size());
		for (unsigned int i = 0; i < curve.size(); i++) {
			x->curve[i].length(curve.at(i).size());
			for (unsigned int j = 0; j < curve.at(i).size(); j++) {
				x->curve[i][j].length(curve.at(i).at(j).size());
				for (unsigned int k = 0; k < curve.at(i).at(j).size(); k++) {
					 x->curve[i][j][k].length(curve.at(i).at(j).at(k).size());
			    }
			}					 		
		}		
		
		for (unsigned int i = 0; i < curve.size() ; i++)
			for (unsigned int j = 0; j < curve.at(i).size(); j++)
				for (unsigned int k = 0; k < curve.at(i).at(j).size(); k++)
					for (unsigned int l = 0; l < curve.at(i).at(j).at(k).size(); l++)
					
						
						x->curve[i][j][k][l] = curve.at(i).at(j).at(k).at(l);
		 				
			 							
	 	cout << "Converted successfully" << endl;
			
		
	

	
  		
		
		
			
		x->rms.length(rms.size());
		for (unsigned int i = 0; i < rms.size(); i++) {
			x->rms[i].length(rms.at(i).size());
			for (unsigned int j = 0; j < rms.at(i).size(); j++) {
				x->rms[i][j].length(rms.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < rms.size() ; i++)
			for (unsigned int j = 0; j < rms.at(i).size(); j++)
				for (unsigned int k = 0; k < rms.at(i).at(j).size(); k++)
					
						
					x->rms[i][j][k] = rms.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalCurveRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalCurveRow::setFromIDL (CalCurveRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		
			
		setNumBaseline(x.numBaseline);
  			
 		
		
	

	
		
		
			
		setNumAPC(x.numAPC);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		antennaNames .clear();
		for (unsigned int i = 0; i <x.antennaNames.length(); ++i) {
			
			antennaNames.push_back(string (x.antennaNames[i]));
			
		}
			
  		
		
	

	
		
		
			
		setRefAntennaName(string (x.refAntennaName));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
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
			
  		
		
	

	
		
		
			
		setTypeCurve(x.typeCurve);
  			
 		
		
	

	
		
		
			
		setTimeOrigin(ArrayTime (x.timeOrigin));
			
 		
		
	

	
		
		
			
		curve .clear();
		vector <vector< vector<float> > > vvv_aux_curve;
		vector< vector<float> > vv_aux_curve;
		vector<float> v_aux_curve;
		
		for (unsigned int i = 0; i <x.curve.length(); ++i) {
			vvv_aux_curve.clear();
			for (unsigned int j = 0; j <x.curve[0].length(); ++j) { 
				vv_aux_curve.clear();
				for (unsigned int k = 0; k < x.curve[0][0].length(); ++k) { 
					v_aux_curve.clear();
					for (unsigned int l = 0; l <x.curve[0][0][0].length(); ++l) { 
					
						v_aux_curve.push_back(x.curve[i][j][k][l]);
		  			
		  		   }
		  		   vv_aux_curve.push_back(v_aux_curve);
		  	   }
		  	   vvv_aux_curve.push_back(vv_aux_curve);
		  	}
		  	curve.push_back(vvv_aux_curve);
		} 		
			
  		
		
	

	
		
		
			
		rms .clear();
		vector< vector<float> > vv_aux_rms;
		vector<float> v_aux_rms;
		
		for (unsigned int i = 0; i < x.rms.length(); ++i) {
			vv_aux_rms.clear();
			for (unsigned int j = 0; j < x.rms[0].length(); ++j) {
				v_aux_rms.clear();
				for (unsigned int k = 0; k < x.rms[0][0].length(); ++k) {
					
					v_aux_rms.push_back(x.rms[i][j][k]);
		  			
		  		}
		  		vv_aux_rms.push_back(v_aux_rms);
  			}
  			rms.push_back(vv_aux_rms);
		}
			
  		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalCurve");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalCurveRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		
		Parser::toXML(numBaseline, "numBaseline", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAPC, "numAPC", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(antennaNames, "antennaNames", buf);
		
		
	

  	
 		
		
		Parser::toXML(refAntennaName, "refAntennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrections", atmPhaseCorrections));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("typeCurve", typeCurve));
		
		
	

  	
 		
		
		Parser::toXML(timeOrigin, "timeOrigin", buf);
		
		
	

  	
 		
		
		Parser::toXML(curve, "curve", buf);
		
		
	

  	
 		
		
		Parser::toXML(rms, "rms", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalCurveRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","CalCurve",rowDoc));
			
		
	

	
  		
			
	  	setNumBaseline(Parser::getInteger("numBaseline","CalCurve",rowDoc));
			
		
	

	
  		
			
	  	setNumAPC(Parser::getInteger("numAPC","CalCurve",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalCurve",rowDoc));
			
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","CalCurve",rowDoc));
			
		
	

	
  		
			
					
	  	setAntennaNames(Parser::get1DString("antennaNames","CalCurve",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setRefAntennaName(Parser::getString("refAntennaName","CalCurve",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalCurve",rowDoc);
		
		
		
	

	
		
		
		
		atmPhaseCorrections = EnumerationParser::getAtmPhaseCorrection1D("atmPhaseCorrections","CalCurve",rowDoc);			
		
		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalCurve",rowDoc);			
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalCurve",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalCurve",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalCurve",rowDoc));
	  			
	  		
		
	

	
		
		
		
		typeCurve = EnumerationParser::getCalCurveType("typeCurve","CalCurve",rowDoc);
		
		
		
	

	
  		
			
	  	setTimeOrigin(Parser::getArrayTime("timeOrigin","CalCurve",rowDoc));
			
		
	

	
  		
			
					
	  	setCurve(Parser::get4DFloat("curve","CalCurve",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setRms(Parser::get3DFloat("rms","CalCurve",rowDoc));
	  			
	  		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalCurve");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int CalCurveRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	

	
 	/**
 	 * Get numBaseline.
 	 * @return numBaseline as int
 	 */
 	int CalCurveRow::getNumBaseline() const {
	
  		return numBaseline;
 	}

 	/**
 	 * Set numBaseline with the specified int.
 	 * @param numBaseline The int value to which numBaseline is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setNumBaseline (int numBaseline)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBaseline = numBaseline;
	
 	}
	
	

	

	
 	/**
 	 * Get numAPC.
 	 * @return numAPC as int
 	 */
 	int CalCurveRow::getNumAPC() const {
	
  		return numAPC;
 	}

 	/**
 	 * Set numAPC with the specified int.
 	 * @param numAPC The int value to which numAPC is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setNumAPC (int numAPC)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAPC = numAPC;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalCurveRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int CalCurveRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setNumPoly (int numPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPoly = numPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaNames.
 	 * @return antennaNames as vector<string >
 	 */
 	vector<string > CalCurveRow::getAntennaNames() const {
	
  		return antennaNames;
 	}

 	/**
 	 * Set antennaNames with the specified vector<string >.
 	 * @param antennaNames The vector<string > value to which antennaNames is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setAntennaNames (vector<string > antennaNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaNames = antennaNames;
	
 	}
	
	

	

	
 	/**
 	 * Get refAntennaName.
 	 * @return refAntennaName as string
 	 */
 	string CalCurveRow::getRefAntennaName() const {
	
  		return refAntennaName;
 	}

 	/**
 	 * Set refAntennaName with the specified string.
 	 * @param refAntennaName The string value to which refAntennaName is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setRefAntennaName (string refAntennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refAntennaName = refAntennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalCurveRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrections.
 	 * @return atmPhaseCorrections as vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >
 	 */
 	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > CalCurveRow::getAtmPhaseCorrections() const {
	
  		return atmPhaseCorrections;
 	}

 	/**
 	 * Set atmPhaseCorrections with the specified vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >.
 	 * @param atmPhaseCorrections The vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > value to which atmPhaseCorrections is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setAtmPhaseCorrections (vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmPhaseCorrections = atmPhaseCorrections;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalCurveRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalCurveRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalCurveRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalCurveRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get typeCurve.
 	 * @return typeCurve as CalCurveTypeMod::CalCurveType
 	 */
 	CalCurveTypeMod::CalCurveType CalCurveRow::getTypeCurve() const {
	
  		return typeCurve;
 	}

 	/**
 	 * Set typeCurve with the specified CalCurveTypeMod::CalCurveType.
 	 * @param typeCurve The CalCurveTypeMod::CalCurveType value to which typeCurve is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setTypeCurve (CalCurveTypeMod::CalCurveType typeCurve)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->typeCurve = typeCurve;
	
 	}
	
	

	

	
 	/**
 	 * Get timeOrigin.
 	 * @return timeOrigin as ArrayTime
 	 */
 	ArrayTime CalCurveRow::getTimeOrigin() const {
	
  		return timeOrigin;
 	}

 	/**
 	 * Set timeOrigin with the specified ArrayTime.
 	 * @param timeOrigin The ArrayTime value to which timeOrigin is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setTimeOrigin (ArrayTime timeOrigin)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->timeOrigin = timeOrigin;
	
 	}
	
	

	

	
 	/**
 	 * Get curve.
 	 * @return curve as vector<vector<vector<vector<float > > > >
 	 */
 	vector<vector<vector<vector<float > > > > CalCurveRow::getCurve() const {
	
  		return curve;
 	}

 	/**
 	 * Set curve with the specified vector<vector<vector<vector<float > > > >.
 	 * @param curve The vector<vector<vector<vector<float > > > > value to which curve is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setCurve (vector<vector<vector<vector<float > > > > curve)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->curve = curve;
	
 	}
	
	

	

	
 	/**
 	 * Get rms.
 	 * @return rms as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > CalCurveRow::getRms() const {
	
  		return rms;
 	}

 	/**
 	 * Set rms with the specified vector<vector<vector<float > > >.
 	 * @param rms The vector<vector<vector<float > > > value to which rms is to be set.
 	 
 	
 		
 	 */
 	void CalCurveRow::setRms (vector<vector<vector<float > > > rms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->rms = rms;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalCurveRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalCurveRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalCurve");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalCurveRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalCurveRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalCurve");
		
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
	 CalDataRow* CalCurveRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalCurveRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalCurveRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalCurveRow::CalCurveRow (CalCurveTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
typeCurve = CCalCurveType::from_int(0);
	

	

	

	
	
	}
	
	CalCurveRow::CalCurveRow (CalCurveTable &t, CalCurveRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			numAntenna = row.numAntenna;
		
			numBaseline = row.numBaseline;
		
			numAPC = row.numAPC;
		
			numReceptor = row.numReceptor;
		
			numPoly = row.numPoly;
		
			antennaNames = row.antennaNames;
		
			refAntennaName = row.refAntennaName;
		
			receiverBand = row.receiverBand;
		
			atmPhaseCorrections = row.atmPhaseCorrections;
		
			polarizationTypes = row.polarizationTypes;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			frequencyRange = row.frequencyRange;
		
			typeCurve = row.typeCurve;
		
			timeOrigin = row.timeOrigin;
		
			curve = row.curve;
		
			rms = row.rms;
		
		
		
		
		}	
	}

	
	bool CalCurveRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->numBaseline == numBaseline);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAPC == numAPC);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaNames == antennaNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->refAntennaName == refAntennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
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
	

	
		
		result = result && (this->typeCurve == typeCurve);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeOrigin == timeOrigin);
		
		if (!result) return false;
	

	
		
		result = result && (this->curve == curve);
		
		if (!result) return false;
	

	
		
		result = result && (this->rms == rms);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalCurveRow::compareRequiredValue(int numAntenna, int numBaseline, int numAPC, int numReceptor, int numPoly, vector<string > antennaNames, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, CalCurveTypeMod::CalCurveType typeCurve, ArrayTime timeOrigin, vector<vector<vector<vector<float > > > > curve, vector<vector<vector<float > > > rms) {
		bool result;
		result = true;
		
	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->numBaseline == numBaseline)) return false;
	

	
		if (!(this->numAPC == numAPC)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->antennaNames == antennaNames)) return false;
	

	
		if (!(this->refAntennaName == refAntennaName)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->atmPhaseCorrections == atmPhaseCorrections)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->typeCurve == typeCurve)) return false;
	

	
		if (!(this->timeOrigin == timeOrigin)) return false;
	

	
		if (!(this->curve == curve)) return false;
	

	
		if (!(this->rms == rms)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalCurveRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalCurveRow::equalByRequiredValue(CalCurveRow* x) {
		
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->numBaseline != x->numBaseline) return false;
			
		if (this->numAPC != x->numAPC) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->antennaNames != x->antennaNames) return false;
			
		if (this->refAntennaName != x->refAntennaName) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->atmPhaseCorrections != x->atmPhaseCorrections) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->typeCurve != x->typeCurve) return false;
			
		if (this->timeOrigin != x->timeOrigin) return false;
			
		if (this->curve != x->curve) return false;
			
		if (this->rms != x->rms) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
