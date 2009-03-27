
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
 * File CalFocusTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalFocusTable.h>
#include <CalFocusRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalFocusTable;
using asdm::CalFocusRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string CalFocusTable::tableName = "CalFocus";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalFocusTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalFocusTable::getKeyName() {
		return key;
	}


	CalFocusTable::CalFocusTable(ASDM &c) : container(c) {

	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	
		key.push_back("antennaName");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalFocusTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalFocusTable.
 */
 
	CalFocusTable::~CalFocusTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalFocusTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int CalFocusTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string CalFocusTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalFocusTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalFocusTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalFocusRow *CalFocusTable::newRow() {
		return new CalFocusRow (*this);
	}
	
	CalFocusRow *CalFocusTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param calDataId. 
	
 	 * @param calReductionId. 
	
 	 * @param antennaName. 
	
 	 * @param frequencyRange. 
	
 	 * @param startValidTime. 
	
 	 * @param endValidTime. 
	
 	 * @param receiverBand. 
	
 	 * @param offset. 
	
 	 * @param error. 
	
 	 * @param focusMethod. 
	
 	 * @param pointingDirection. 
	
 	 * @param wasFixed. 
	
 	 * @param ambientTemperature. 
	
     */
	CalFocusRow* CalFocusTable::newRow(Tag calDataId, Tag calReductionId, string antennaName, vector<Frequency > frequencyRange, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Length > offset, vector<Length > error, FocusMethodMod::FocusMethod focusMethod, vector<Angle > pointingDirection, vector<bool > wasFixed, Temperature ambientTemperature){
		CalFocusRow *row = new CalFocusRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setReceiverBand(receiverBand);
			
		row->setOffset(offset);
			
		row->setError(error);
			
		row->setFocusMethod(focusMethod);
			
		row->setPointingDirection(pointingDirection);
			
		row->setWasFixed(wasFixed);
			
		row->setAmbientTemperature(ambientTemperature);
	
		return row;		
	}	

	CalFocusRow* CalFocusTable::newRowFull(Tag calDataId, Tag calReductionId, string antennaName, vector<Frequency > frequencyRange, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Length > offset, vector<Length > error, FocusMethodMod::FocusMethod focusMethod, vector<Angle > pointingDirection, vector<bool > wasFixed, Temperature ambientTemperature)	{
		CalFocusRow *row = new CalFocusRow(*this);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setAntennaName(antennaName);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setReceiverBand(receiverBand);
			
		row->setOffset(offset);
			
		row->setError(error);
			
		row->setFocusMethod(focusMethod);
			
		row->setPointingDirection(pointingDirection);
			
		row->setWasFixed(wasFixed);
			
		row->setAmbientTemperature(ambientTemperature);
	
		return row;				
	}
	


CalFocusRow* CalFocusTable::newRow(CalFocusRow* row) {
	return new CalFocusRow(*this, *row);
}

CalFocusRow* CalFocusTable::newRowCopy(CalFocusRow* row) {
	return new CalFocusRow(*this, *row);
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
	CalFocusRow* CalFocusTable::add(CalFocusRow* x) {
		
		if (getRowByKey(
						x->getCalDataId()
						,
						x->getCalReductionId()
						,
						x->getAntennaName()
						))
			//throw DuplicateKey(x.getCalDataId() + "|" + x.getCalReductionId() + "|" + x.getAntennaName(),"CalFocus");
			throw DuplicateKey("Duplicate key exception in ","CalFocusTable");
		
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
	CalFocusRow*  CalFocusTable::checkAndAdd(CalFocusRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getCalDataId()
	,
			x->getCalReductionId()
	,
			x->getAntennaName()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalFocusTable");
		
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
	 * @return Alls rows as an array of CalFocusRow
	 */
	vector<CalFocusRow *> CalFocusTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalFocusRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalFocusRow* CalFocusTable::getRowByKey(Tag calDataId, Tag calReductionId, string antennaName)  {
	CalFocusRow* aRow = 0;
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
 	 		
 * @param frequencyRange.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param receiverBand.
 	 		
 * @param offset.
 	 		
 * @param error.
 	 		
 * @param focusMethod.
 	 		
 * @param pointingDirection.
 	 		
 * @param wasFixed.
 	 		
 * @param ambientTemperature.
 	 		 
 */
CalFocusRow* CalFocusTable::lookup(Tag calDataId, Tag calReductionId, string antennaName, vector<Frequency > frequencyRange, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Length > offset, vector<Length > error, FocusMethodMod::FocusMethod focusMethod, vector<Angle > pointingDirection, vector<bool > wasFixed, Temperature ambientTemperature) {
		CalFocusRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(calDataId, calReductionId, antennaName, frequencyRange, startValidTime, endValidTime, receiverBand, offset, error, focusMethod, pointingDirection, wasFixed, ambientTemperature)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	CalFocusTableIDL *CalFocusTable::toIDL() {
		CalFocusTableIDL *x = new CalFocusTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalFocusRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalFocusTable::fromIDL(CalFocusTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalFocusRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *CalFocusTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalFocus");
	}

	void CalFocusTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","CalFocus");
	}

	string CalFocusTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","CalFocus");
	}

	void CalFocusTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","CalFocus");
	}

	string CalFocusTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<CalFocusTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/CalFocusTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<CalFocusTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalFocusRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalFocusTable> ");
		return buf;
	}
	
	void CalFocusTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalFocusTable")) 
			error();
		// cout << "Parsing a CalFocusTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalFocusTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalFocusRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalFocusRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalFocusTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalFocusTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalFocusTable::checkAndAdd called from CalFocusTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalFocusTable>")) 
			error();
	}

	void CalFocusTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","CalFocus");
	}
	
	string CalFocusTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void CalFocusTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void CalFocusTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalFocus.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalFocus");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalFocus");
		}
		else {
			// write the XML
			string fileName = directory + "/CalFocus.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalFocus");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalFocus");
		}
	}
	
	void CalFocusTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/CalFocus.bin";
		else
			tablename = directory + "/CalFocus.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "CalFocus");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"CalFocus");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"CalFocus");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
