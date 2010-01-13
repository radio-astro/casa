
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
 * File WVMCalRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <WVMCalRow.h>
#include <WVMCalTable.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::WVMCalRow;
using asdm::WVMCalTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::AntennaTable;
using asdm::AntennaRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	WVMCalRow::~WVMCalRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	WVMCalTable &WVMCalRow::getTable() const {
		return table;
	}
	
	void WVMCalRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a WVMCalRowIDL struct.
	 */
	WVMCalRowIDL *WVMCalRow::toIDL() const {
		WVMCalRowIDL *x = new WVMCalRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->wvrMethod = wvrMethod;
 				
 			
		
	

	
  		
		
		
			
		x->polyFreqLimits.length(polyFreqLimits.size());
		for (unsigned int i = 0; i < polyFreqLimits.size(); ++i) {
			
			x->polyFreqLimits[i] = polyFreqLimits.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->numChan = numChan;
 				
 			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
		x->pathCoeff.length(pathCoeff.size());
		for (unsigned int i = 0; i < pathCoeff.size(); i++) {
			x->pathCoeff[i].length(pathCoeff.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < pathCoeff.size() ; i++)
			for (unsigned int j = 0; j < pathCoeff.at(i).size(); j++)
					
						
				x->pathCoeff[i][j] = pathCoeff.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->refTemp.length(refTemp.size());
		for (unsigned int i = 0; i < refTemp.size(); ++i) {
			
				
			x->refTemp[i] = refTemp.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct WVMCalRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void WVMCalRow::setFromIDL (WVMCalRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setWvrMethod(x.wvrMethod);
  			
 		
		
	

	
		
		
			
		polyFreqLimits .clear();
		for (unsigned int i = 0; i <x.polyFreqLimits.length(); ++i) {
			
			polyFreqLimits.push_back(Frequency (x.polyFreqLimits[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumChan(x.numChan);
  			
 		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		pathCoeff .clear();
		vector<double> v_aux_pathCoeff;
		for (unsigned int i = 0; i < x.pathCoeff.length(); ++i) {
			v_aux_pathCoeff.clear();
			for (unsigned int j = 0; j < x.pathCoeff[0].length(); ++j) {
				
				v_aux_pathCoeff.push_back(x.pathCoeff[i][j]);
	  			
  			}
  			pathCoeff.push_back(v_aux_pathCoeff);			
		}
			
  		
		
	

	
		
		
			
		refTemp .clear();
		for (unsigned int i = 0; i <x.refTemp.length(); ++i) {
			
			refTemp.push_back(x.refTemp[i]);
  			
		}
			
  		
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"WVMCal");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string WVMCalRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("wvrMethod", wvrMethod));
		
		
	

  	
 		
		
		Parser::toXML(polyFreqLimits, "polyFreqLimits", buf);
		
		
	

  	
 		
		
		Parser::toXML(numChan, "numChan", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(pathCoeff, "pathCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(refTemp, "refTemp", buf);
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void WVMCalRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","WVMCal",rowDoc));
			
		
	

	
		
		
		
		wvrMethod = EnumerationParser::getWVRMethod("wvrMethod","WVMCal",rowDoc);
		
		
		
	

	
  		
			
					
	  	setPolyFreqLimits(Parser::get1DFrequency("polyFreqLimits","WVMCal",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumChan(Parser::getInteger("numChan","WVMCal",rowDoc));
			
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","WVMCal",rowDoc));
			
		
	

	
  		
			
					
	  	setPathCoeff(Parser::get2DDouble("pathCoeff","WVMCal",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setRefTemp(Parser::get1DDouble("refTemp","WVMCal",rowDoc));
	  			
	  		
		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"WVMCal");
		}
	}
	
	void WVMCalRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	spectralWindowId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
					
			eoss.writeInt(wvrMethod);
				
		
	

	
	
		
	Frequency::toBin(polyFreqLimits, eoss);
		
	

	
	
		
						
			eoss.writeInt(numChan);
				
		
	

	
	
		
						
			eoss.writeInt(numPoly);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) pathCoeff.size());
		eoss.writeInt((int) pathCoeff.at(0).size());
		for (unsigned int i = 0; i < pathCoeff.size(); i++) 
			for (unsigned int j = 0;  j < pathCoeff.at(0).size(); j++) 
							 
				eoss.writeDouble(pathCoeff.at(i).at(j));
				
	
						
		
	

	
	
		
		
			
		eoss.writeInt((int) refTemp.size());
		for (unsigned int i = 0; i < refTemp.size(); i++)
				
			eoss.writeDouble(refTemp.at(i));
				
				
						
		
	


	
	
	}
	
void WVMCalRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void WVMCalRow::spectralWindowIdFromBin(EndianISStream& eiss) {
		
	
		
		
		spectralWindowId =  Tag::fromBin(eiss);
		
	
	
}
void WVMCalRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void WVMCalRow::wvrMethodFromBin(EndianISStream& eiss) {
		
	
	
		
			
		wvrMethod = CWVRMethod::from_int(eiss.readInt());
			
		
	
	
}
void WVMCalRow::polyFreqLimitsFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	polyFreqLimits = Frequency::from1DBin(eiss);	
	

		
	
	
}
void WVMCalRow::numChanFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numChan =  eiss.readInt();
			
		
	
	
}
void WVMCalRow::numPolyFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numPoly =  eiss.readInt();
			
		
	
	
}
void WVMCalRow::pathCoeffFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		pathCoeff.clear();
		
		unsigned int pathCoeffDim1 = eiss.readInt();
		unsigned int pathCoeffDim2 = eiss.readInt();
		vector <double> pathCoeffAux1;
		for (unsigned int i = 0; i < pathCoeffDim1; i++) {
			pathCoeffAux1.clear();
			for (unsigned int j = 0; j < pathCoeffDim2 ; j++)			
			
			pathCoeffAux1.push_back(eiss.readDouble());
			
			pathCoeff.push_back(pathCoeffAux1);
		}
	
	

		
	
	
}
void WVMCalRow::refTempFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		refTemp.clear();
		
		unsigned int refTempDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < refTempDim1; i++)
			
			refTemp.push_back(eiss.readDouble());
			
	

		
	
	
}

		
	
	WVMCalRow* WVMCalRow::fromBin(EndianISStream& eiss, WVMCalTable& table, const vector<string>& attributesSeq) {
		WVMCalRow* row = new  WVMCalRow(table);
		
		map<string, WVMCalAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "WVMCalTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval WVMCalRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WVMCalRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "WVMCal");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get wvrMethod.
 	 * @return wvrMethod as WVRMethodMod::WVRMethod
 	 */
 	WVRMethodMod::WVRMethod WVMCalRow::getWvrMethod() const {
	
  		return wvrMethod;
 	}

 	/**
 	 * Set wvrMethod with the specified WVRMethodMod::WVRMethod.
 	 * @param wvrMethod The WVRMethodMod::WVRMethod value to which wvrMethod is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setWvrMethod (WVRMethodMod::WVRMethod wvrMethod)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->wvrMethod = wvrMethod;
	
 	}
	
	

	

	
 	/**
 	 * Get polyFreqLimits.
 	 * @return polyFreqLimits as vector<Frequency >
 	 */
 	vector<Frequency > WVMCalRow::getPolyFreqLimits() const {
	
  		return polyFreqLimits;
 	}

 	/**
 	 * Set polyFreqLimits with the specified vector<Frequency >.
 	 * @param polyFreqLimits The vector<Frequency > value to which polyFreqLimits is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setPolyFreqLimits (vector<Frequency > polyFreqLimits)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polyFreqLimits = polyFreqLimits;
	
 	}
	
	

	

	
 	/**
 	 * Get numChan.
 	 * @return numChan as int
 	 */
 	int WVMCalRow::getNumChan() const {
	
  		return numChan;
 	}

 	/**
 	 * Set numChan with the specified int.
 	 * @param numChan The int value to which numChan is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setNumChan (int numChan)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numChan = numChan;
	
 	}
	
	

	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int WVMCalRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setNumPoly (int numPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPoly = numPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get pathCoeff.
 	 * @return pathCoeff as vector<vector<double > >
 	 */
 	vector<vector<double > > WVMCalRow::getPathCoeff() const {
	
  		return pathCoeff;
 	}

 	/**
 	 * Set pathCoeff with the specified vector<vector<double > >.
 	 * @param pathCoeff The vector<vector<double > > value to which pathCoeff is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setPathCoeff (vector<vector<double > > pathCoeff)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pathCoeff = pathCoeff;
	
 	}
	
	

	

	
 	/**
 	 * Get refTemp.
 	 * @return refTemp as vector<double >
 	 */
 	vector<double > WVMCalRow::getRefTemp() const {
	
  		return refTemp;
 	}

 	/**
 	 * Set refTemp with the specified vector<double >.
 	 * @param refTemp The vector<double > value to which refTemp is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setRefTemp (vector<double > refTemp)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refTemp = refTemp;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag WVMCalRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WVMCalRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "WVMCal");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag WVMCalRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WVMCalRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "WVMCal");
		
  		}
  	
 		this->spectralWindowId = spectralWindowId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* WVMCalRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* WVMCalRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	/**
	 * Create a WVMCalRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	WVMCalRow::WVMCalRow (WVMCalTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
wvrMethod = CWVRMethod::from_int(0);
	

	

	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &WVMCalRow::antennaIdFromBin; 
	 fromBinMethods["spectralWindowId"] = &WVMCalRow::spectralWindowIdFromBin; 
	 fromBinMethods["timeInterval"] = &WVMCalRow::timeIntervalFromBin; 
	 fromBinMethods["wvrMethod"] = &WVMCalRow::wvrMethodFromBin; 
	 fromBinMethods["polyFreqLimits"] = &WVMCalRow::polyFreqLimitsFromBin; 
	 fromBinMethods["numChan"] = &WVMCalRow::numChanFromBin; 
	 fromBinMethods["numPoly"] = &WVMCalRow::numPolyFromBin; 
	 fromBinMethods["pathCoeff"] = &WVMCalRow::pathCoeffFromBin; 
	 fromBinMethods["refTemp"] = &WVMCalRow::refTempFromBin; 
		
	
	
	}
	
	WVMCalRow::WVMCalRow (WVMCalTable &t, WVMCalRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			wvrMethod = row.wvrMethod;
		
			polyFreqLimits = row.polyFreqLimits;
		
			numChan = row.numChan;
		
			numPoly = row.numPoly;
		
			pathCoeff = row.pathCoeff;
		
			refTemp = row.refTemp;
		
		
		
		
		}
		
		 fromBinMethods["antennaId"] = &WVMCalRow::antennaIdFromBin; 
		 fromBinMethods["spectralWindowId"] = &WVMCalRow::spectralWindowIdFromBin; 
		 fromBinMethods["timeInterval"] = &WVMCalRow::timeIntervalFromBin; 
		 fromBinMethods["wvrMethod"] = &WVMCalRow::wvrMethodFromBin; 
		 fromBinMethods["polyFreqLimits"] = &WVMCalRow::polyFreqLimitsFromBin; 
		 fromBinMethods["numChan"] = &WVMCalRow::numChanFromBin; 
		 fromBinMethods["numPoly"] = &WVMCalRow::numPolyFromBin; 
		 fromBinMethods["pathCoeff"] = &WVMCalRow::pathCoeffFromBin; 
		 fromBinMethods["refTemp"] = &WVMCalRow::refTempFromBin; 
			
	
			
	}

	
	bool WVMCalRow::compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, WVRMethodMod::WVRMethod wvrMethod, vector<Frequency > polyFreqLimits, int numChan, int numPoly, vector<vector<double > > pathCoeff, vector<double > refTemp) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->wvrMethod == wvrMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->polyFreqLimits == polyFreqLimits);
		
		if (!result) return false;
	

	
		
		result = result && (this->numChan == numChan);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->pathCoeff == pathCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->refTemp == refTemp);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool WVMCalRow::compareRequiredValue(WVRMethodMod::WVRMethod wvrMethod, vector<Frequency > polyFreqLimits, int numChan, int numPoly, vector<vector<double > > pathCoeff, vector<double > refTemp) {
		bool result;
		result = true;
		
	
		if (!(this->wvrMethod == wvrMethod)) return false;
	

	
		if (!(this->polyFreqLimits == polyFreqLimits)) return false;
	

	
		if (!(this->numChan == numChan)) return false;
	

	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->pathCoeff == pathCoeff)) return false;
	

	
		if (!(this->refTemp == refTemp)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the WVMCalRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool WVMCalRow::equalByRequiredValue(WVMCalRow* x) {
		
			
		if (this->wvrMethod != x->wvrMethod) return false;
			
		if (this->polyFreqLimits != x->polyFreqLimits) return false;
			
		if (this->numChan != x->numChan) return false;
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->pathCoeff != x->pathCoeff) return false;
			
		if (this->refTemp != x->refTemp) return false;
			
		
		return true;
	}	
	
/*
	 map<string, WVMCalAttributeFromBin> WVMCalRow::initFromBinMethods() {
		map<string, WVMCalAttributeFromBin> result;
		
		result["antennaId"] = &WVMCalRow::antennaIdFromBin;
		result["spectralWindowId"] = &WVMCalRow::spectralWindowIdFromBin;
		result["timeInterval"] = &WVMCalRow::timeIntervalFromBin;
		result["wvrMethod"] = &WVMCalRow::wvrMethodFromBin;
		result["polyFreqLimits"] = &WVMCalRow::polyFreqLimitsFromBin;
		result["numChan"] = &WVMCalRow::numChanFromBin;
		result["numPoly"] = &WVMCalRow::numPolyFromBin;
		result["pathCoeff"] = &WVMCalRow::pathCoeffFromBin;
		result["refTemp"] = &WVMCalRow::refTempFromBin;
		
		
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
