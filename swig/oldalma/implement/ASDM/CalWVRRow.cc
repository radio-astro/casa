
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
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
				
		x->numChan = numChan;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->wvrMethod = wvrMethod;
 				
 			
		
	

	
  		
		
		
			
		x->freqLimits.length(freqLimits.size());
		for (unsigned int i = 0; i < freqLimits.size(); ++i) {
			
			x->freqLimits[i] = freqLimits.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
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
		 				
			 									
			
		
	

	
  		
		
		
			
		x->chanFreq.length(chanFreq.size());
		for (unsigned int i = 0; i < chanFreq.size(); ++i) {
			
			x->chanFreq[i] = chanFreq.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->chanWidth.length(chanWidth.size());
		for (unsigned int i = 0; i < chanWidth.size(); ++i) {
			
			x->chanWidth[i] = chanWidth.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->numInputAntenna = numInputAntenna;
 				
 			
		
	

	
  		
		
		
			
		x->inputAntennaNames.length(inputAntennaNames.size());
		for (unsigned int i = 0; i < inputAntennaNames.size(); ++i) {
			
				
			x->inputAntennaNames[i] = CORBA::string_dup(inputAntennaNames.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->polyFreqLimits.length(polyFreqLimits.size());
		for (unsigned int i = 0; i < polyFreqLimits.size(); ++i) {
			
			x->polyFreqLimits[i] = polyFreqLimits.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
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
	void CalWVRRow::setFromIDL (CalWVRRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		setNumChan(x.numChan);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setWvrMethod(x.wvrMethod);
  			
 		
		
	

	
		
		
			
		freqLimits .clear();
		for (unsigned int i = 0; i <x.freqLimits.length(); ++i) {
			
			freqLimits.push_back(Frequency (x.freqLimits[i]));
			
		}
			
  		
		
	

	
		
		
			
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
			
  		
		
	

	
		
		
			
		chanFreq .clear();
		for (unsigned int i = 0; i <x.chanFreq.length(); ++i) {
			
			chanFreq.push_back(Frequency (x.chanFreq[i]));
			
		}
			
  		
		
	

	
		
		
			
		chanWidth .clear();
		for (unsigned int i = 0; i <x.chanWidth.length(); ++i) {
			
			chanWidth.push_back(Frequency (x.chanWidth[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumInputAntenna(x.numInputAntenna);
  			
 		
		
	

	
		
		
			
		inputAntennaNames .clear();
		for (unsigned int i = 0; i <x.inputAntennaNames.length(); ++i) {
			
			inputAntennaNames.push_back(string (x.inputAntennaNames[i]));
			
		}
			
  		
		
	

	
		
		
			
		polyFreqLimits .clear();
		for (unsigned int i = 0; i <x.polyFreqLimits.length(); ++i) {
			
			polyFreqLimits.push_back(Frequency (x.polyFreqLimits[i]));
			
		}
			
  		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalWVR");
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
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(numChan, "numChan", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("wvrMethod", wvrMethod));
		
		
	

  	
 		
		
		Parser::toXML(freqLimits, "freqLimits", buf);
		
		
	

  	
 		
		
		Parser::toXML(pathCoeff, "pathCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(chanFreq, "chanFreq", buf);
		
		
	

  	
 		
		
		Parser::toXML(chanWidth, "chanWidth", buf);
		
		
	

  	
 		
		
		Parser::toXML(numInputAntenna, "numInputAntenna", buf);
		
		
	

  	
 		
		
		Parser::toXML(inputAntennaNames, "inputAntennaNames", buf);
		
		
	

  	
 		
		
		Parser::toXML(polyFreqLimits, "polyFreqLimits", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalWVRRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalWVR",rowDoc));
			
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","CalWVR",rowDoc));
			
		
	

	
  		
			
	  	setNumChan(Parser::getInteger("numChan","CalWVR",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalWVR",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalWVR",rowDoc));
			
		
	

	
		
		
		
		wvrMethod = EnumerationParser::getWVRMethod("wvrMethod","CalWVR",rowDoc);
		
		
		
	

	
  		
			
					
	  	setFreqLimits(Parser::get1DFrequency("freqLimits","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPathCoeff(Parser::get3DFloat("pathCoeff","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setChanFreq(Parser::get1DFrequency("chanFreq","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setChanWidth(Parser::get1DFrequency("chanWidth","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumInputAntenna(Parser::getInteger("numInputAntenna","CalWVR",rowDoc));
			
		
	

	
  		
			
					
	  	setInputAntennaNames(Parser::get1DString("inputAntennaNames","CalWVR",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPolyFreqLimits(Parser::get1DFrequency("polyFreqLimits","CalWVR",rowDoc));
	  			
	  		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalWVR");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
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
 	 * Get freqLimits.
 	 * @return freqLimits as vector<Frequency >
 	 */
 	vector<Frequency > CalWVRRow::getFreqLimits() const {
	
  		return freqLimits;
 	}

 	/**
 	 * Set freqLimits with the specified vector<Frequency >.
 	 * @param freqLimits The vector<Frequency > value to which freqLimits is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setFreqLimits (vector<Frequency > freqLimits)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->freqLimits = freqLimits;
	
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
 	 * Get numInputAntenna.
 	 * @return numInputAntenna as int
 	 */
 	int CalWVRRow::getNumInputAntenna() const {
	
  		return numInputAntenna;
 	}

 	/**
 	 * Set numInputAntenna with the specified int.
 	 * @param numInputAntenna The int value to which numInputAntenna is to be set.
 	 
 	
 		
 	 */
 	void CalWVRRow::setNumInputAntenna (int numInputAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numInputAntenna = numInputAntenna;
	
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
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
		
		
		
			numPoly = row.numPoly;
		
			numChan = row.numChan;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			wvrMethod = row.wvrMethod;
		
			freqLimits = row.freqLimits;
		
			pathCoeff = row.pathCoeff;
		
			chanFreq = row.chanFreq;
		
			chanWidth = row.chanWidth;
		
			numInputAntenna = row.numInputAntenna;
		
			inputAntennaNames = row.inputAntennaNames;
		
			polyFreqLimits = row.polyFreqLimits;
		
		
		
		
		}	
	}

	
	bool CalWVRRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, int numPoly, int numChan, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, vector<Frequency > freqLimits, vector<vector<vector<float > > > pathCoeff, vector<Frequency > chanFreq, vector<Frequency > chanWidth, int numInputAntenna, vector<string > inputAntennaNames, vector<Frequency > polyFreqLimits) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->numChan == numChan);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->wvrMethod == wvrMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->freqLimits == freqLimits);
		
		if (!result) return false;
	

	
		
		result = result && (this->pathCoeff == pathCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->chanFreq == chanFreq);
		
		if (!result) return false;
	

	
		
		result = result && (this->chanWidth == chanWidth);
		
		if (!result) return false;
	

	
		
		result = result && (this->numInputAntenna == numInputAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->inputAntennaNames == inputAntennaNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->polyFreqLimits == polyFreqLimits);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalWVRRow::compareRequiredValue(int numPoly, int numChan, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, vector<Frequency > freqLimits, vector<vector<vector<float > > > pathCoeff, vector<Frequency > chanFreq, vector<Frequency > chanWidth, int numInputAntenna, vector<string > inputAntennaNames, vector<Frequency > polyFreqLimits) {
		bool result;
		result = true;
		
	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->numChan == numChan)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->wvrMethod == wvrMethod)) return false;
	

	
		if (!(this->freqLimits == freqLimits)) return false;
	

	
		if (!(this->pathCoeff == pathCoeff)) return false;
	

	
		if (!(this->chanFreq == chanFreq)) return false;
	

	
		if (!(this->chanWidth == chanWidth)) return false;
	

	
		if (!(this->numInputAntenna == numInputAntenna)) return false;
	

	
		if (!(this->inputAntennaNames == inputAntennaNames)) return false;
	

	
		if (!(this->polyFreqLimits == polyFreqLimits)) return false;
	

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
		
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->numChan != x->numChan) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->wvrMethod != x->wvrMethod) return false;
			
		if (this->freqLimits != x->freqLimits) return false;
			
		if (this->pathCoeff != x->pathCoeff) return false;
			
		if (this->chanFreq != x->chanFreq) return false;
			
		if (this->chanWidth != x->chanWidth) return false;
			
		if (this->numInputAntenna != x->numInputAntenna) return false;
			
		if (this->inputAntennaNames != x->inputAntennaNames) return false;
			
		if (this->polyFreqLimits != x->polyFreqLimits) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
