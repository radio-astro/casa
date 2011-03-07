
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
 * File FocusModelTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <FocusModelTable.h>
#include <FocusModelRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::FocusModelTable;
using asdm::FocusModelRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string FocusModelTable::tableName = "FocusModel";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> FocusModelTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> FocusModelTable::getKeyName() {
		return key;
	}


	FocusModelTable::FocusModelTable(ASDM &c) : container(c) {

	
		key.push_back("focusModelId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("FocusModelTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for FocusModelTable.
 */
 
	FocusModelTable::~FocusModelTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &FocusModelTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int FocusModelTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string FocusModelTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity FocusModelTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void FocusModelTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	FocusModelRow *FocusModelTable::newRow() {
		return new FocusModelRow (*this);
	}
	
	FocusModelRow *FocusModelTable::newRowEmpty() {
		return newRow ();
	}



FocusModelRow* FocusModelTable::newRow(FocusModelRow* row) {
	return new FocusModelRow(*this, *row);
}

FocusModelRow* FocusModelTable::newRowCopy(FocusModelRow* row) {
	return new FocusModelRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.focusModelId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a FocusModelRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	FocusModelRow* FocusModelTable::add(FocusModelRow* x) {
			

			
		// Autoincrement focusModelId
		x->setFocusModelId(Tag(size(), TagType::FocusModel));
						
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
	FocusModelRow*  FocusModelTable::checkAndAdd(FocusModelRow* x) throw (DuplicateKey, UniquenessViolationException) {
	 
		
		
		
		if (getRowByKey(
	
			x->getFocusModelId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "FocusModelTable");
		
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
	 * @return Alls rows as an array of FocusModelRow
	 */
	vector<FocusModelRow *> FocusModelTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a FocusModelRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	FocusModelRow* FocusModelTable::getRowByKey(Tag focusModelId)  {
	FocusModelRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->focusModelId != focusModelId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	FocusModelTableIDL *FocusModelTable::toIDL() {
		FocusModelTableIDL *x = new FocusModelTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<FocusModelRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void FocusModelTable::fromIDL(FocusModelTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			FocusModelRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *FocusModelTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","FocusModel");
	}

	void FocusModelTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","FocusModel");
	}

	string FocusModelTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","FocusModel");
	}

	void FocusModelTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","FocusModel");
	}

	string FocusModelTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<FocusModelTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/FocusModelTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<FocusModelTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<FocusModelRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</FocusModelTable> ");
		return buf;
	}
	
	void FocusModelTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<FocusModelTable")) 
			error();
		// cout << "Parsing a FocusModelTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "FocusModelTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		FocusModelRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a FocusModelRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"FocusModelTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"FocusModelTable");	
			}
			catch (...) {
				// cout << "Unexpected error in FocusModelTable::checkAndAdd called from FocusModelTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</FocusModelTable>")) 
			error();
	}

	void FocusModelTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","FocusModel");
	}
	
	string FocusModelTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void FocusModelTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void FocusModelTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/FocusModel.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FocusModel");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FocusModel");
		}
		else {
			// write the XML
			string fileName = directory + "/FocusModel.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FocusModel");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FocusModel");
		}
	}
	
	void FocusModelTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/FocusModel.bin";
		else
			tablename = directory + "/FocusModel.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "FocusModel");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"FocusModel");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"FocusModel");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
	void FocusModelTable::autoIncrement(string key, FocusModelRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  focusModelId to Tag(0).
			x->setFocusModelId(Tag(0,  TagType::FocusModel));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  focusModelId to Tag(n).
			x->setFocusModelId(Tag(n, TagType::FocusModel));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
