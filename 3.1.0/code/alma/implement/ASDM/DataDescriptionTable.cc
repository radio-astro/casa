
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
 * File DataDescriptionTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <DataDescriptionTable.h>
#include <DataDescriptionRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::DataDescriptionTable;
using asdm::DataDescriptionRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>

#include <Misc.h>

using namespace std;
using namespace asdm;


namespace asdm {

	string DataDescriptionTable::tableName = "DataDescription";
	const vector<string> DataDescriptionTable::attributesNames = initAttributesNames();

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> DataDescriptionTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> DataDescriptionTable::getKeyName() {
		return key;
	}


	DataDescriptionTable::DataDescriptionTable(ASDM &c) : container(c) {
	
		key.push_back("dataDescriptionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("DataDescriptionTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive binary or XML
		archiveAsBin = false;
		
		// File binary or XML
		fileAsBin = false;		
	}
	
/**
 * A destructor for DataDescriptionTable.
 */	
	
	DataDescriptionTable::~DataDescriptionTable() {
		for (unsigned int i = 0; i < row.size(); ++i) {
			delete row[i];
		}
	}


	/**
	 * Container to which this table belongs.
	 */
	ASDM &DataDescriptionTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int DataDescriptionTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string DataDescriptionTable::getName() const {
		return tableName;
	}

	/**
	 * Build the vector of attributes names.
	 */
	vector<string> DataDescriptionTable::initAttributesNames() {
		vector<string> attributesNames;

		attributesNames.push_back("dataDescriptionId");
		attributesNames.push_back("polOrHoloId");
		attributesNames.push_back("spectralWindowId");

		return attributesNames;
	}

	/**
	 * Return the names of the attributes.
	 */
	const vector<string>& DataDescriptionTable::getAttributesNames() { return attributesNames; }

	/**
	 * Return this table's Entity.
	 */
	Entity DataDescriptionTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void DataDescriptionTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	DataDescriptionRow *DataDescriptionTable::newRow() {
		return new DataDescriptionRow (*this);
	}

	DataDescriptionRow *DataDescriptionTable::newRowEmpty() {
		return new DataDescriptionRow (*this);
	}
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param polOrHoloId. 
	
 	 * @param spectralWindowId. 
	
     */
	DataDescriptionRow* DataDescriptionTable::newRow(Tag polOrHoloId, Tag spectralWindowId){
		DataDescriptionRow *row = new DataDescriptionRow(*this);
			
		row->setPolOrHoloId(polOrHoloId);
			
		row->setSpectralWindowId(spectralWindowId);
	
		return row;		
	}	

	DataDescriptionRow* DataDescriptionTable::newRowFull(Tag polOrHoloId, Tag spectralWindowId){
		return newRow(polOrHoloId, spectralWindowId);
	}

	DataDescriptionRow* DataDescriptionTable::newRow(DataDescriptionRow* row) {
		return new DataDescriptionRow(*this, *row);
	}

	DataDescriptionRow* DataDescriptionTable::newRowCopy(DataDescriptionRow* row) {
		return newRow(row);
	}
	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.dataDescriptionId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a DataDescriptionRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	DataDescriptionRow* DataDescriptionTable::add(DataDescriptionRow* x) {
			 
		DataDescriptionRow* aRow = lookup(
				
		x->getPolOrHoloId()
				,
		x->getSpectralWindowId()
				
		);
		if (aRow) return aRow;
			

			
		// Autoincrement dataDescriptionId
		x->setDataDescriptionId(Tag(size(), TagType::DataDescription));
						
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded(true);
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
	DataDescriptionRow*  DataDescriptionTable::checkAndAdd(DataDescriptionRow* x) throw (DuplicateKey, UniquenessViolationException) {
	 
		 
		if (lookup(
			
			x->getPolOrHoloId()
		,
			x->getSpectralWindowId()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table DataDescriptionTable");
		
		
		
		if (getRowByKey(
	
			x->getDataDescriptionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "DataDescriptionTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded(true);
		return x;	
	}	







	

	//
	// ====> Methods returning rows.
	//	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of DataDescriptionRow
	 */
	vector<DataDescriptionRow *> DataDescriptionTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a DataDescriptionRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	DataDescriptionRow* DataDescriptionTable::getRowByKey(Tag dataDescriptionId)  {
	DataDescriptionRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->dataDescriptionId != dataDescriptionId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param polOrHoloId.
 	 		
 * @param spectralWindowId.
 	 		 
 */
DataDescriptionRow* DataDescriptionTable::lookup(Tag polOrHoloId, Tag spectralWindowId) {
		DataDescriptionRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(polOrHoloId, spectralWindowId)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	DataDescriptionTableIDL *DataDescriptionTable::toIDL() {
		DataDescriptionTableIDL *x = new DataDescriptionTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<DataDescriptionRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void DataDescriptionTable::fromIDL(DataDescriptionTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			DataDescriptionRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *DataDescriptionTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","DataDescription");
	}

	void DataDescriptionTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","DataDescription");
	}

	string DataDescriptionTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","DataDescription");
	}

	void DataDescriptionTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","DataDescription");
	}

	string DataDescriptionTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<DataDescriptionTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/DataDescriptionTable\" xsi:schemaLocation=\"http://Alma/XASDM/DataDescriptionTable http://almaobservatory.org/XML/XASDM/1/DataDescriptionTable.xsd\"> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<DataDescriptionRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</DataDescriptionTable> ");
		return buf;
	}
	
	void DataDescriptionTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<DataDescriptionTable")) 
			error();
		// cout << "Parsing a DataDescriptionTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "DataDescriptionTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		DataDescriptionRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a DataDescriptionRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"DataDescriptionTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"DataDescriptionTable");	
			}
			catch (...) {
				// cout << "Unexpected error in DataDescriptionTable::checkAndAdd called from DataDescriptionTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</DataDescriptionTable>")) 
			error();
	}

	void DataDescriptionTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","DataDescription");
	}
	
	string DataDescriptionTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void DataDescriptionTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	void DataDescriptionTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " + directory , "DataDescription");
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/DataDescription.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "DataDescription");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "DataDescription");
		}
		else {
			// write the XML
			string fileName = directory + "/DataDescription.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "DataDescription");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "DataDescription");
		}
	}
	
	void DataDescriptionTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/DataDescription.bin";
		else
			tablename = directory + "/DataDescription.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "DataDescription");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"DataDescription");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"DataDescription");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());		
	}	
	
	
	void DataDescriptionTable::autoIncrement(string key, DataDescriptionRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  dataDescriptionId to Tag(0).
			x->setDataDescriptionId(Tag(0,  TagType::DataDescription));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  dataDescriptionId to Tag(n).
			x->setDataDescriptionId(Tag(n, TagType::DataDescription));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
