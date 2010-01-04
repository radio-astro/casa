
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
 * File CalWVRRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalWVRRow.h>
#include <CalWVRTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalWVRRow;
using asdm::CalWVRTable;

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

	CalWVRRow::~CalWVRRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalWVRTable &CalWVRRow::getTable() const {
		return table;
	}
	
	void CalWVRRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalWVRRowIDL struct.
	 */
	CalWVRRowIDL *CalWVRRow::toIDL() const {
		CalWVRRowIDL *x = new CalWVRRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->wvrMethod = wvrMethod;
 				
 			
		
	

	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numInputAntennas = numInputAntennas;
 				
 			
		
	

	
  		
		
		
			
		x->inputAntennaNames.length(inputAntennaNames.size());
		for (unsigned int i = 0; i < inputAntennaNames.size(); ++i) {
			
				
			x->inputAntennaNames[i] = CORBA::string_dup(inputAntennaNames.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->numChan = numChan;
 				
 			
		
	

	
  		
		
		
			
		x->chanFreq.length(chanFreq.size());
		for (unsigned int i = 0; i < chanFreq.size(); ++i) {
			
			x->chanFreq[i] = chanFreq.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->chanWidth.length(chanWidth.size());
		for (unsigned int i = 0; i < chanWidth.size(); ++i) {
			
			x->chanWidth[i] = chanWidth.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->refTemp.length(refTemp.size());
		for (unsigned int i = 0; i < refTemp.size(); i++) {
			x->refTemp[i].length(refTemp.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < refTemp.size() ; i++)
			for (unsigned int j = 0; j < refTemp.at(i).size(); j++)
					
				x->refTemp[i][j]= refTemp.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
		x->pathCoeff.length(pathCoeff.size());
		for (unsigned int i = 0; i < pathCoeff.size(); i++) {
			x->pathCoeff[i].length(pathCoeff.at(i).size());
			for (unsigned int j = 0; j < pathCoeff.at(i).size(); j++) {
				x->pathCoeff[i][j].length(pathCoeff.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < pathCoeff.size() ; i++)
			for (unsigned int j = 0; j < pathCoeff.at(i).size(); j++)
				for (unsigned int k = 0; k < pathCoeff.at(i).at(j).size(); k++)
					
						
					x->pathCoeff[i][j][k] = pathCoeff.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
  		
		
		
			
		x->polyFreqLimits.length(polyFreqLimits.size());
		for (unsigned int i = 0; i < polyFreqLimits.size(); ++i) {
			
			x->polyFreqLimits[i] = polyFreqLimits.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->wetPath.length(wetPath.size());
		for (unsigned int i = 0; i < wetPath.size(); ++i) {
			
				
			x->wetPath[i] = wetPath.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->dryPath.length(dryPath.size());
		for (unsigned int i = 0; i < dryPath.size(); ++i) {
			
				
			x->dryPath[i] = dryPath.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->water = water.toIDLLength();
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalWVRRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalWVRRow::setFromIDL (CalWVRRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setWvrMethod(x.wvrMethod);
  			
 		
		
	

	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setNumInputAntennas(x.numInputAntennas);
  			
 		
		
	

	
		
		
			
		inputAntennaNames .clear();
		for (unsigned int i = 0; i <x.inputAntennaNames.length(); ++i) {
			
			inputAntennaNames.push_back(string (x.inputAntennaNames[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumChan(x.numChan);
  			
 		
		
	

	
		
		
			
		chanFreq .clear();
		for (unsigned int i = 0; i <x.chanFreq.length(); ++i) {
			
			chanFreq.push_back(Frequency (x.chanFreq[i]));
			
		}
			
  		
		
	

	
		
		
			
		chanWidth .clear();
		for (unsigned int i = 0; i <x.chanWidth.length(); ++i) {
			
			chanWidth.push_back(Frequency (x.chanWidth[i]));
			
		}
			
  		
		
	

	
		
		
			
		refTemp .clear();
		vector<Temperature> v_aux_refTemp;
		for (unsigned int i = 0; i < x.refTemp.length(); ++i) {
			v_aux_refTemp.clear();
			for (unsigned int j = 0; j < x.refTemp[0].length(); ++j) {
				
				v_aux_refTemp.push_back(Temperature (x.refTemp[i][j]));
				
  			}
  			refTemp.push_back(v_aux_refTemp);			
		}
			
  		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		pathCoeff .clear();
		vector< vector<float> > vv_aux_pathCoeff;
		vector<float> v_aux_pathCoeff;
		
		for (unsigned int i = 0; i < x.pathCoeff.length(); ++i) {
			vv_aux_pathCoeff.clear();
			for (unsigned int j = 0; j < x.pathCoeff[0].length(); ++j) {
				v_aux_pathCoeff.clear();
				for (unsigned int k = 0; k < x.pathCoeff[0][0].length(); ++k) {
					
					v_aux_pathCoeff.push_back(x.pathCoeff[i][j][k]);
		  			
		  		}
		  		vv_aux_pathCoeff.push_back(v_aux_pathCoeff);
  			}
  			pathCoeff.push_back(vv_aux_pathCoeff);
		}
			
  		
		
	

	
		
		
			
		polyFreqLimits .clear();
		for (unsigned int i = 0; i <x.polyFreqLimits.length(); ++i) {
			
			polyFreqLimits.push_back(Frequency (x.polyFreqLimits[i]));
			
		}
			
  		
		
	

	
		
		
			
		wetPath .clear();
		for (unsigned int i = 0; i <x.wetPath.length(); ++i) {
			
			wetPath.push_back(x.wetPath[i]);
  			
		}
			
  		
		
	

	
		
		
			
		dryPath .clear();
		for (unsigned int i = 0; i <x.dryPath.length(); ++i) {
			
			dryPath.push_back(x.dryPath[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setWater(Length (x.water));
			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalWVR");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalWVRRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("wvrMethod", wvrMethod));
		
		
	

  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(numInputAntennas, "numInputAntennas", buf);
		
		
	

  	
 		
		
		Parser::toXML(inputAntennaNames, "inputAntennaNames", buf);
		
		
	

  	
 		
		
		Parser::toXML(numChan, "numChan", buf);
		
		
	

  	
 		
		
		Parser::toXML(chanFreq, "chanFreq", buf);
		
		
	

  	
 		
		
		Parser::toXML(chanWidth, "chanWidth", buf);
		
		
	

  	
 		
		
		Parser::toXML(refTemp, "refTemp", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(pathCoeff, "pathCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(polyFreqLimits, "polyFreqLimits", buf);
		
		
	

  	
 		
		
		Parser::toXML(wetPath, "wetPath", buf);
		
		
	

  	
 		
		
		Parser::toXML(dryPath, "dryPath", buf);
		
		
	

  	
 		
		
		Parser::toXML(water, "water", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalWVRRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalWVR",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalWVR",rowDoc));
			
		
	

	
		
		
		
		wvrMethod = EnumerationParser::getWVRMethod("wvrMethod","CalWVR",rowDoc);
		
		
		
	

	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalWVR",rowDoc));
			
		
	

	
  		
			
	  	setNumInputAntennas(Parser::getInteger("numInputAntennas","CalWVR",rowDoc));
			
		
	

	
  		
			
					
	  	setInputAntennaNames(Parser::get1DString("inputAntennaNames","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumChan(Parser::getInteger("numChan","CalWVR",rowDoc));
			
		
	

	
  		
			
					
	  	setChanFreq(Parser::get1DFrequency("chanFreq","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setChanWidth(Parser::get1DFrequency("chanWidth","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setRefTemp(Parser::get2DTemperature("refTemp","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","CalWVR",rowDoc));
			
		
	

	
  		
			
					
	  	setPathCoeff(Parser::get3DFloat("pathCoeff","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPolyFreqLimits(Parser::get1DFrequency("polyFreqLimits","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setWetPath(Parser::get1DFloat("wetPath","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setDryPath(Parser::get1DFloat("dryPath","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setWater(Parser::getLength("water","CalWVR",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalWVR");
		}
	}
	
	void CalWVRRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeString(antennaName);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
					
			eoss.writeInt(wvrMethod);
				
		
	

	
	
		
						
			eoss.writeInt(numInputAntennas);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) inputAntennaNames.size());
		for (unsigned int i = 0; i < inputAntennaNames.size(); i++)
				
			eoss.writeString(inputAntennaNames.at(i));
				
				
						
		
	

	
	
		
						
			eoss.writeInt(numChan);
				
		
	

	
	
		
	Frequency::toBin(chanFreq, eoss);
		
	

	
	
		
	Frequency::toBin(chanWidth, eoss);
		
	

	
	
		
	Temperature::toBin(refTemp, eoss);
		
	

	
	
		
						
			eoss.writeInt(numPoly);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) pathCoeff.size());
		eoss.writeInt((int) pathCoeff.at(0).size());		
		eoss.writeInt((int) pathCoeff.at(0).at(0).size());
		for (unsigned int i = 0; i < pathCoeff.size(); i++) 
			for (unsigned int j = 0;  j < pathCoeff.at(0).size(); j++)
				for (unsigned int k = 0; k <  pathCoeff.at(0).at(0).size(); k++)	
							 
					eoss.writeFloat(pathCoeff.at(i).at(j).at(k));
						
						
		
	

	
	
		
	Frequency::toBin(polyFreqLimits, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) wetPath.size());
		for (unsigned int i = 0; i < wetPath.size(); i++)
				
			eoss.writeFloat(wetPath.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) dryPath.size());
		for (unsigned int i = 0; i < dryPath.size(); i++)
				
			eoss.writeFloat(dryPath.at(i));
				
				
						
		
	

	
	
		
	water.toBin(eoss);
		
	


	
	
	}
	
	CalWVRRow* CalWVRRow::fromBin(EndianISStream& eiss, CalWVRTable& table) {
		CalWVRRow* row = new  CalWVRRow(table);
		
		
		
	
	
		
			
		row->antennaName =  eiss.readString();
			
		
	

	
		
		
		row->calDataId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->calReductionId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->startValidTime =  ArrayTime::fromBin(eiss);
		
	

	
		
		
		row->endValidTime =  ArrayTime::fromBin(eiss);
		
	

	
	
		
			
		row->wvrMethod = CWVRMethod::from_int(eiss.readInt());
			
		
	

	
	
		
			
		row->numInputAntennas =  eiss.readInt();
			
		
	

	
	
		
			
	
		row->inputAntennaNames.clear();
		
		unsigned int inputAntennaNamesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < inputAntennaNamesDim1; i++)
			
			row->inputAntennaNames.push_back(eiss.readString());
			
	

		
	

	
	
		
			
		row->numChan =  eiss.readInt();
			
		
	

	
		
		
			
	
	row->chanFreq = Frequency::from1DBin(eiss);	
	

		
	

	
		
		
			
	
	row->chanWidth = Frequency::from1DBin(eiss);	
	

		
	

	
		
		
			
	
	row->refTemp = Temperature::from2DBin(eiss);		
	

		
	

	
	
		
			
		row->numPoly =  eiss.readInt();
			
		
	

	
	
		
			
	
		row->pathCoeff.clear();
			
		unsigned int pathCoeffDim1 = eiss.readInt();
		unsigned int pathCoeffDim2 = eiss.readInt();
		unsigned int pathCoeffDim3 = eiss.readInt();
		vector <vector<float> > pathCoeffAux2;
		vector <float> pathCoeffAux1;
		for (unsigned int i = 0; i < pathCoeffDim1; i++) {
			pathCoeffAux2.clear();
			for (unsigned int j = 0; j < pathCoeffDim2 ; j++) {
				pathCoeffAux1.clear();
				for (unsigned int k = 0; k < pathCoeffDim3; k++) {
			
					pathCoeffAux1.push_back(eiss.readFloat());
			
				}
				pathCoeffAux2.push_back(pathCoeffAux1);
			}
			row->pathCoeff.push_back(pathCoeffAux2);
		}	
	

		
	

	
		
		
			
	
	row->polyFreqLimits = Frequency::from1DBin(eiss);	
	

		
	

	
	
		
			
	
		row->wetPath.clear();
		
		unsigned int wetPathDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < wetPathDim1; i++)
			
			row->wetPath.push_back(eiss.readFloat());
			
	

		
	

	
	
		
			
	
		row->dryPath.clear();
		
		unsigned int dryPathDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < dryPathDim1; i++)
			
			row->dryPath.push_back(eiss.readFloat());
			
	

		
	

	
		
		
		row->water =  Length::fromBin(eiss);
		
	

		
		
		
		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalWVRRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalWVRRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get wvrMethod.
 	 * @return wvrMethod as WVRMethodMod::WVRMethod
 	 */
 	WVRMethodMod::WVRMethod CalWVRRow::getWvrMethod() const {
	
  		return wvrMethod;
 	}

 	/**
 	 * Set wvrMethod with the specified WVRMethodMod::WVRMethod.
 	 * @param wvrMethod The WVRMethodMod::WVRMethod value to which wvrMethod is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setWvrMethod (WVRMethodMod::WVRMethod wvrMethod)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->wvrMethod = wvrMethod;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalWVRRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalWVRRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalWVR");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get numInputAntennas.
 	 * @return numInputAntennas as int
 	 */
 	int CalWVRRow::getNumInputAntennas() const {
	
  		return numInputAntennas;
 	}

 	/**
 	 * Set numInputAntennas with the specified int.
 	 * @param numInputAntennas The int value to which numInputAntennas is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setNumInputAntennas (int numInputAntennas)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numInputAntennas = numInputAntennas;
	
 	}
	
	

	

	
 	/**
 	 * Get inputAntennaNames.
 	 * @return inputAntennaNames as vector<string >
 	 */
 	vector<string > CalWVRRow::getInputAntennaNames() const {
	
  		return inputAntennaNames;
 	}

 	/**
 	 * Set inputAntennaNames with the specified vector<string >.
 	 * @param inputAntennaNames The vector<string > value to which inputAntennaNames is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setInputAntennaNames (vector<string > inputAntennaNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->inputAntennaNames = inputAntennaNames;
	
 	}
	
	

	

	
 	/**
 	 * Get numChan.
 	 * @return numChan as int
 	 */
 	int CalWVRRow::getNumChan() const {
	
  		return numChan;
 	}

 	/**
 	 * Set numChan with the specified int.
 	 * @param numChan The int value to which numChan is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setNumChan (int numChan)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numChan = numChan;
	
 	}
	
	

	

	
 	/**
 	 * Get chanFreq.
 	 * @return chanFreq as vector<Frequency >
 	 */
 	vector<Frequency > CalWVRRow::getChanFreq() const {
	
  		return chanFreq;
 	}

 	/**
 	 * Set chanFreq with the specified vector<Frequency >.
 	 * @param chanFreq The vector<Frequency > value to which chanFreq is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setChanFreq (vector<Frequency > chanFreq)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->chanFreq = chanFreq;
	
 	}
	
	

	

	
 	/**
 	 * Get chanWidth.
 	 * @return chanWidth as vector<Frequency >
 	 */
 	vector<Frequency > CalWVRRow::getChanWidth() const {
	
  		return chanWidth;
 	}

 	/**
 	 * Set chanWidth with the specified vector<Frequency >.
 	 * @param chanWidth The vector<Frequency > value to which chanWidth is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setChanWidth (vector<Frequency > chanWidth)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->chanWidth = chanWidth;
	
 	}
	
	

	

	
 	/**
 	 * Get refTemp.
 	 * @return refTemp as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > CalWVRRow::getRefTemp() const {
	
  		return refTemp;
 	}

 	/**
 	 * Set refTemp with the specified vector<vector<Temperature > >.
 	 * @param refTemp The vector<vector<Temperature > > value to which refTemp is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setRefTemp (vector<vector<Temperature > > refTemp)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refTemp = refTemp;
	
 	}
	
	

	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int CalWVRRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setNumPoly (int numPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPoly = numPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get pathCoeff.
 	 * @return pathCoeff as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > CalWVRRow::getPathCoeff() const {
	
  		return pathCoeff;
 	}

 	/**
 	 * Set pathCoeff with the specified vector<vector<vector<float > > >.
 	 * @param pathCoeff The vector<vector<vector<float > > > value to which pathCoeff is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setPathCoeff (vector<vector<vector<float > > > pathCoeff)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pathCoeff = pathCoeff;
	
 	}
	
	

	

	
 	/**
 	 * Get polyFreqLimits.
 	 * @return polyFreqLimits as vector<Frequency >
 	 */
 	vector<Frequency > CalWVRRow::getPolyFreqLimits() const {
	
  		return polyFreqLimits;
 	}

 	/**
 	 * Set polyFreqLimits with the specified vector<Frequency >.
 	 * @param polyFreqLimits The vector<Frequency > value to which polyFreqLimits is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setPolyFreqLimits (vector<Frequency > polyFreqLimits)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polyFreqLimits = polyFreqLimits;
	
 	}
	
	

	

	
 	/**
 	 * Get wetPath.
 	 * @return wetPath as vector<float >
 	 */
 	vector<float > CalWVRRow::getWetPath() const {
	
  		return wetPath;
 	}

 	/**
 	 * Set wetPath with the specified vector<float >.
 	 * @param wetPath The vector<float > value to which wetPath is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setWetPath (vector<float > wetPath)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->wetPath = wetPath;
	
 	}
	
	

	

	
 	/**
 	 * Get dryPath.
 	 * @return dryPath as vector<float >
 	 */
 	vector<float > CalWVRRow::getDryPath() const {
	
  		return dryPath;
 	}

 	/**
 	 * Set dryPath with the specified vector<float >.
 	 * @param dryPath The vector<float > value to which dryPath is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setDryPath (vector<float > dryPath)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->dryPath = dryPath;
	
 	}
	
	

	

	
 	/**
 	 * Get water.
 	 * @return water as Length
 	 */
 	Length CalWVRRow::getWater() const {
	
  		return water;
 	}

 	/**
 	 * Set water with the specified Length.
 	 * @param water The Length value to which water is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setWater (Length water)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->water = water;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalWVRRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalWVRRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalWVR");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalWVRRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalWVRRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalWVR");
		
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
	 CalDataRow* CalWVRRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalWVRRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalWVRRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalWVRRow::CalWVRRow (CalWVRTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
wvrMethod = CWVRMethod::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	CalWVRRow::CalWVRRow (CalWVRTable &t, CalWVRRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			antennaName = row.antennaName;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			wvrMethod = row.wvrMethod;
		
			numInputAntennas = row.numInputAntennas;
		
			inputAntennaNames = row.inputAntennaNames;
		
			numChan = row.numChan;
		
			chanFreq = row.chanFreq;
		
			chanWidth = row.chanWidth;
		
			refTemp = row.refTemp;
		
			numPoly = row.numPoly;
		
			pathCoeff = row.pathCoeff;
		
			polyFreqLimits = row.polyFreqLimits;
		
			wetPath = row.wetPath;
		
			dryPath = row.dryPath;
		
			water = row.water;
		
		
		
		
		}	
	}

	
	bool CalWVRRow::compareNoAutoInc(string antennaName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->wvrMethod == wvrMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->numInputAntennas == numInputAntennas);
		
		if (!result) return false;
	

	
		
		result = result && (this->inputAntennaNames == inputAntennaNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->numChan == numChan);
		
		if (!result) return false;
	

	
		
		result = result && (this->chanFreq == chanFreq);
		
		if (!result) return false;
	

	
		
		result = result && (this->chanWidth == chanWidth);
		
		if (!result) return false;
	

	
		
		result = result && (this->refTemp == refTemp);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->pathCoeff == pathCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->polyFreqLimits == polyFreqLimits);
		
		if (!result) return false;
	

	
		
		result = result && (this->wetPath == wetPath);
		
		if (!result) return false;
	

	
		
		result = result && (this->dryPath == dryPath);
		
		if (!result) return false;
	

	
		
		result = result && (this->water == water);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalWVRRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->wvrMethod == wvrMethod)) return false;
	

	
		if (!(this->numInputAntennas == numInputAntennas)) return false;
	

	
		if (!(this->inputAntennaNames == inputAntennaNames)) return false;
	

	
		if (!(this->numChan == numChan)) return false;
	

	
		if (!(this->chanFreq == chanFreq)) return false;
	

	
		if (!(this->chanWidth == chanWidth)) return false;
	

	
		if (!(this->refTemp == refTemp)) return false;
	

	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->pathCoeff == pathCoeff)) return false;
	

	
		if (!(this->polyFreqLimits == polyFreqLimits)) return false;
	

	
		if (!(this->wetPath == wetPath)) return false;
	

	
		if (!(this->dryPath == dryPath)) return false;
	

	
		if (!(this->water == water)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalWVRRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalWVRRow::equalByRequiredValue(CalWVRRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->wvrMethod != x->wvrMethod) return false;
			
		if (this->numInputAntennas != x->numInputAntennas) return false;
			
		if (this->inputAntennaNames != x->inputAntennaNames) return false;
			
		if (this->numChan != x->numChan) return false;
			
		if (this->chanFreq != x->chanFreq) return false;
			
		if (this->chanWidth != x->chanWidth) return false;
			
		if (this->refTemp != x->refTemp) return false;
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->pathCoeff != x->pathCoeff) return false;
			
		if (this->polyFreqLimits != x->polyFreqLimits) return false;
			
		if (this->wetPath != x->wetPath) return false;
			
		if (this->dryPath != x->dryPath) return false;
			
		if (this->water != x->water) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
