
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
 * File CalPositionTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalPositionTable.h>
#include <CalPositionRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalPositionTable;
using asdm::CalPositionRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalPositionTable::tableName = "CalPosition";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalPositionTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalPositionTable::getKeyName() {
		return key;
	}


	CalPositionTable::CalPositionTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalPositionTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalPositionTable.
 */
 
	CalPositionTable::~CalPositionTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalPositionTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalPositionTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalPositionTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalPositionTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalPositionTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalPositionRow *CalPositionTable::newRow() {
		return new CalPositionRow (*this);
	}
	
	CalPositionRow *CalPositionTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param numAntenna. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param positionOffset. 
	
 	 * @param positionErr. 
	
 	 * @param delayRms. 
	
 	 * @param phaseRms. 
	
 	 * @param axesOffset. 
	
 	 * @param axesOffsetFixed. 
	
 	 * @param axesOffsetErr. 
	
 	 * @param positionMethod. 
	
 	 * @param refAntennaNames. 
	
 	 * @param stationName. 
	
 	 * @param antennaPosition. 
	
 	 * @param stationPosition. 
	
     */
	CalPositionRow* CalPositionTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, int numAntenna, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > positionOffset, vector<Length > positionErr, Interval delayRms, Angle phaseRms, Length axesOffset, bool axesOffsetFixed, Length axesOffsetErr, PositionMethodMod::PositionMethod positionMethod, vector<string > refAntennaNames, string stationName, vector<Length > antennaPosition, vector<Length > stationPosition){
		CalPositionRow *row = new CalPositionRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setNumAntenna(numAntenna);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setPositionOffset(positionOffset);
			
		row->setPositionErr(positionErr);
			
		row->setDelayRms(delayRms);
			
		row->setPhaseRms(phaseRms);
			
		row->setAxesOffset(axesOffset);
			
		row->setAxesOffsetFixed(axesOffsetFixed);
			
		row->setAxesOffsetErr(axesOffsetErr);
			
		row->setPositionMethod(positionMethod);
			
		row->setRefAntennaNames(refAntennaNames);
			
		row->setStationName(stationName);
			
		row->setAntennaPosition(antennaPosition);
			
		row->setStationPosition(stationPosition);
	
		return row;		
	}	

	CalPositionRow* CalPositionTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, int numAntenna, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > positionOffset, vector<Length > positionErr, Interval delayRms, Angle phaseRms, Length axesOffset, bool axesOffsetFixed, Length axesOffsetErr, PositionMethodMod::PositionMethod positionMethod, vector<string > refAntennaNames, string stationName, vector<Length > antennaPosition, vector<Length > stationPosition)	{
		CalPositionRow *row = new CalPositionRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setNumAntenna(numAntenna);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setPositionOffset(positionOffset);
			
		row->setPositionErr(positionErr);
			
		row->setDelayRms(delayRms);
			
		row->setPhaseRms(phaseRms);
			
		row->setAxesOffset(axesOffset);
			
		row->setAxesOffsetFixed(axesOffsetFixed);
			
		row->setAxesOffsetErr(axesOffsetErr);
			
		row->setPositionMethod(positionMethod);
			
		row->setRefAntennaNames(refAntennaNames);
			
		row->setStationName(stationName);
			
		row->setAntennaPosition(antennaPosition);
			
		row->setStationPosition(stationPosition);
	
		return row;				
	}
	


CalPositionRow* CalPositionTable::newRow(CalPositionRow* row) {
	return new CalPositionRow(*this, *row);
}

CalPositionRow* CalPositionTable::newRowCopy(CalPositionRow* row) {
	return new CalPositionRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	/**
	 * Add a row.
	 * @throws DuplicateKey Thrown if the new row has a key that is already in the table.
	 * @param x A pointer to the row to be added.
	 * @return x
	 */
	CalPositionRow* CalPositionTable::add(CalPositionRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName(),"CalPosition");
			throw DuplicateKey("Duplicate key exception in ","CalPositionTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
		return x;
	}

		





	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @param x a pointer on the row to be appended.
	 * @returns a pointer on x.
	 */
	CalPositionRow*  CalPositionTable::checkAndAdd(CalPositionRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalPositionTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
		return x;	
	}	







	

	//
	// ====> Methods returning rows.
	//	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of CalPositionRow
	 */
	vector<CalPositionRow *> CalPositionTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalPositionRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalPositionRow* CalPositionTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName)  {
	CalPositionRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->calDataId != calDataId) continue;
			
		
			
				if (aRow->calReductionId != calReductionId) continue;
			
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param antennaName.
 	 		
 * @param numAntenna.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param positionOffset.
 	 		
 * @param positionErr.
 	 		
 * @param delayRms.
 	 		
 * @param phaseRms.
 	 		
 * @param axesOffset.
 	 		
 * @param axesOffsetFixed.
 	 		
 * @param axesOffsetErr.
 	 		
 * @param positionMethod.
 	 		
 * @param refAntennaNames.
 	 		
 * @param stationName.
 	 		
 * @param antennaPosition.
 	 		
 * @param stationPosition.
 	 		 
 */
CalPositionRow* CalPositionTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, int numAntenna, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > positionOffset, vector<Length > positionErr, Interval delayRms, Angle phaseRms, Length axesOffset, bool axesOffsetFixed, Length axesOffsetErr, PositionMethodMod::PositionMethod positionMethod, vector<string > refAntennaNames, string stationName, vector<Length > antennaPosition, vector<Length > stationPosition) {
		CalPositionRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, numAntenna, startValidTime, endValidTime, positionOffset, positionErr, delayRms, phaseRms, axesOffset, axesOffsetFixed, axesOffsetErr, positionMethod, refAntennaNames, stationName, antennaPosition, stationPosition)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalPositionTableIDL *CalPositionTable::toIDL() {
		CalPositionTableIDL *x = new CalPositionTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalPositionRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalPositionTable::fromIDL(CalPositionTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalPositionRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalPositionTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPosition");
	}

	void CalPositionTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPosition");
	}

	string CalPositionTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalPosition");
	}

	void CalPositionTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalPosition");
	}

	string CalPositionTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalPositionTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalPositionTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalPositionTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalPositionRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalPositionTable> ");
		return buf;
	}
	
	void CalPositionTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalPositionTable")) 
			error();
		// cout << "Parsing a CalPositionTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalPositionTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalPositionRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalPositionRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalPositionTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalPositionTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalPositionTable::checkAndAdd called from CalPositionTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalPositionTable>")) 
			error();
	}

	void CalPositionTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalPosition");
	}
	
	string CalPositionTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalPositionTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalPositionTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalPosition.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPosition");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPosition");
		}
		else {
			// write the XML
			string fileName = directory + "/CalPosition.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalPosition");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalPosition");
		}
	}
	
	void CalPositionTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalPosition.bin";
		else
			tablename = directory + "/CalPosition.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalPosition");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalPosition");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalPosition");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
