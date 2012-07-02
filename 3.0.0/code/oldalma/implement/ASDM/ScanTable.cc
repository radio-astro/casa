
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
 * File ScanTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <ScanTable.h>
#include <ScanRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::ScanTable;
using asdm::ScanRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string ScanTable::tableName = "Scan";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> ScanTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> ScanTable::getKeyName() {
		return key;
	}


	ScanTable::ScanTable(ASDM &c) : container(c) {

	
		key.push_back("execBlockId");
	
		key.push_back("scanNumber");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("ScanTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for ScanTable.
 */
 
	ScanTable::~ScanTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &ScanTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int ScanTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string ScanTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity ScanTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void ScanTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	ScanRow *ScanTable::newRow() {
		return new ScanRow (*this);
	}
	
	ScanRow *ScanTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param execBlockId. 
	
 	 * @param scanNumber. 
	
 	 * @param startTime. 
	
 	 * @param endTime. 
	
 	 * @param numSubScan. 
	
 	 * @param numIntent. 
	
 	 * @param scanIntent. 
	
 	 * @param flagRow. 
	
     */
	ScanRow* ScanTable::newRow(Tag execBlockId, int scanNumber, ArrayTime startTime, ArrayTime endTime, int numSubScan, int numIntent, vector<ScanIntentMod::ScanIntent > scanIntent, bool flagRow){
		ScanRow *row = new ScanRow(*this);
			
		row->setExecBlockId(execBlockId);
			
		row->setScanNumber(scanNumber);
			
		row->setStartTime(startTime);
			
		row->setEndTime(endTime);
			
		row->setNumSubScan(numSubScan);
			
		row->setNumIntent(numIntent);
			
		row->setScanIntent(scanIntent);
			
		row->setFlagRow(flagRow);
	
		return row;		
	}	

	ScanRow* ScanTable::newRowFull(Tag execBlockId, int scanNumber, ArrayTime startTime, ArrayTime endTime, int numSubScan, int numIntent, vector<ScanIntentMod::ScanIntent > scanIntent, bool flagRow)	{
		ScanRow *row = new ScanRow(*this);
			
		row->setExecBlockId(execBlockId);
			
		row->setScanNumber(scanNumber);
			
		row->setStartTime(startTime);
			
		row->setEndTime(endTime);
			
		row->setNumSubScan(numSubScan);
			
		row->setNumIntent(numIntent);
			
		row->setScanIntent(scanIntent);
			
		row->setFlagRow(flagRow);
	
		return row;				
	}
	


ScanRow* ScanTable::newRow(ScanRow* row) {
	return new ScanRow(*this, *row);
}

ScanRow* ScanTable::newRowCopy(ScanRow* row) {
	return new ScanRow(*this, *row);
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
	ScanRow* ScanTable::add(ScanRow* x) {
		
		if (getRowByKey(
						x->getExecBlockId()
						,
						x->getScanNumber()
						))
			//throw DuplicateKey(x.getExecBlockId() + "|" + x.getScanNumber(),"Scan");
			throw DuplicateKey("Duplicate key exception in ","ScanTable");
		
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
	ScanRow*  ScanTable::checkAndAdd(ScanRow* x) throw (DuplicateKey) {
		
		
		if (getRowByKey(
	
			x->getExecBlockId()
	,
			x->getScanNumber()
			
		)) throw DuplicateKey("Duplicate key exception in ", "ScanTable");
		
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
	 * @return Alls rows as an array of ScanRow
	 */
	vector<ScanRow *> ScanTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a ScanRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	ScanRow* ScanTable::getRowByKey(Tag execBlockId, int scanNumber)  {
	ScanRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->execBlockId != execBlockId) continue;
			
		
			
				if (aRow->scanNumber != scanNumber) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param execBlockId.
 	 		
 * @param scanNumber.
 	 		
 * @param startTime.
 	 		
 * @param endTime.
 	 		
 * @param numSubScan.
 	 		
 * @param numIntent.
 	 		
 * @param scanIntent.
 	 		
 * @param flagRow.
 	 		 
 */
ScanRow* ScanTable::lookup(Tag execBlockId, int scanNumber, ArrayTime startTime, ArrayTime endTime, int numSubScan, int numIntent, vector<ScanIntentMod::ScanIntent > scanIntent, bool flagRow) {
		ScanRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(execBlockId, scanNumber, startTime, endTime, numSubScan, numIntent, scanIntent, flagRow)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	ScanTableIDL *ScanTable::toIDL() {
		ScanTableIDL *x = new ScanTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<ScanRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void ScanTable::fromIDL(ScanTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			ScanRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *ScanTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","Scan");
	}

	void ScanTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","Scan");
	}

	string ScanTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","Scan");
	}

	void ScanTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","Scan");
	}

	string ScanTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<ScanTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/ScanTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<ScanTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<ScanRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</ScanTable> ");
		return buf;
	}
	
	void ScanTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<ScanTable")) 
			error();
		// cout << "Parsing a ScanTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "ScanTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		ScanRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a ScanRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"ScanTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"ScanTable");	
			}
			catch (...) {
				// cout << "Unexpected error in ScanTable::checkAndAdd called from ScanTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</ScanTable>")) 
			error();
	}

	void ScanTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","Scan");
	}
	
	string ScanTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void ScanTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void ScanTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Scan.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Scan");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Scan");
		}
		else {
			// write the XML
			string fileName = directory + "/Scan.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Scan");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Scan");
		}
	}
	
	void ScanTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/Scan.bin";
		else
			tablename = directory + "/Scan.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "Scan");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"Scan");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"Scan");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
} // End namespace asdm
 
