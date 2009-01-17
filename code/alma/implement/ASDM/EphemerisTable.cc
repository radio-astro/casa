
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
 * File EphemerisTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <EphemerisTable.h>
#include <EphemerisRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::EphemerisTable;
using asdm::EphemerisRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string EphemerisTable::tableName = "Ephemeris";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> EphemerisTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> EphemerisTable::getKeyName() {
		return key;
	}


	EphemerisTable::EphemerisTable(ASDM &c) : container(c) {

	
		key.push_back("ephemerisId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("EphemerisTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for EphemerisTable.
 */
 
	EphemerisTable::~EphemerisTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &EphemerisTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int EphemerisTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string EphemerisTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity EphemerisTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void EphemerisTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	EphemerisRow *EphemerisTable::newRow() {
		return new EphemerisRow (*this);
	}
	
	EphemerisRow *EphemerisTable::newRowEmpty() {
		return newRow ();
	}



EphemerisRow* EphemerisTable::newRow(EphemerisRow* row) {
	return new EphemerisRow(*this, *row);
}

EphemerisRow* EphemerisTable::newRowCopy(EphemerisRow* row) {
	return new EphemerisRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.ephemerisId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a EphemerisRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	EphemerisRow* EphemerisTable::add(EphemerisRow* x) {
			

			
		// Autoincrement ephemerisId
		x->setEphemerisId(Tag(size(), TagType::Ephemeris));
						
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
	EphemerisRow*  EphemerisTable::checkAndAdd(EphemerisRow* x) throw (DuplicateKey, UniquenessViolationException) {
	 
		
		
		
		if (getRowByKey(
	
			x->getEphemerisId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "EphemerisTable");
		
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
	 * @return Alls rows as an array of EphemerisRow
	 */
	vector<EphemerisRow *> EphemerisTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a EphemerisRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	EphemerisRow* EphemerisTable::getRowByKey(Tag ephemerisId)  {
	EphemerisRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->ephemerisId != ephemerisId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	EphemerisTableIDL *EphemerisTable::toIDL() {
		EphemerisTableIDL *x = new EphemerisTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<EphemerisRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void EphemerisTable::fromIDL(EphemerisTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			EphemerisRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *EphemerisTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","Ephemeris");
	}

	void EphemerisTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","Ephemeris");
	}

	string EphemerisTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","Ephemeris");
	}

	void EphemerisTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","Ephemeris");
	}

	string EphemerisTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<EphemerisTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/EphemerisTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<EphemerisTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<EphemerisRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</EphemerisTable> ");
		return buf;
	}
	
	void EphemerisTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<EphemerisTable")) 
			error();
		// cout << "Parsing a EphemerisTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "EphemerisTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		EphemerisRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a EphemerisRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"EphemerisTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"EphemerisTable");	
			}
			catch (...) {
				// cout << "Unexpected error in EphemerisTable::checkAndAdd called from EphemerisTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</EphemerisTable>")) 
			error();
	}

	void EphemerisTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","Ephemeris");
	}
	
	string EphemerisTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void EphemerisTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void EphemerisTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Ephemeris.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Ephemeris");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Ephemeris");
		}
		else {
			// write the XML
			string fileName = directory + "/Ephemeris.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Ephemeris");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Ephemeris");
		}
	}
	
	void EphemerisTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/Ephemeris.bin";
		else
			tablename = directory + "/Ephemeris.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "Ephemeris");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"Ephemeris");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"Ephemeris");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
	void EphemerisTable::autoIncrement(string key, EphemerisRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  ephemerisId to Tag(0).
			x->setEphemerisId(Tag(0,  TagType::Ephemeris));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  ephemerisId to Tag(n).
			x->setEphemerisId(Tag(n, TagType::Ephemeris));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
