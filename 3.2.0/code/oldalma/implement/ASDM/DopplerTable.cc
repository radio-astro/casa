
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
 * File DopplerTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <DopplerTable.h>
#include <DopplerRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::DopplerTable;
using asdm::DopplerRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string DopplerTable::tableName = "Doppler";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> DopplerTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> DopplerTable::getKeyName() {
		return key;
	}


	DopplerTable::DopplerTable(ASDM &c) : container(c) {

	
		key.push_back("dopplerId");
	
		key.push_back("sourceId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("DopplerTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for DopplerTable.
 */
 
	DopplerTable::~DopplerTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &DopplerTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int DopplerTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string DopplerTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity DopplerTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void DopplerTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	DopplerRow *DopplerTable::newRow() {
		return new DopplerRow (*this);
	}
	
	DopplerRow *DopplerTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param sourceId. 
	
 	 * @param transitionIndex. 
	
 	 * @param velDef. 
	
     */
	DopplerRow* DopplerTable::newRow(int sourceId, int transitionIndex, Speed velDef){
		DopplerRow *row = new DopplerRow(*this);
			
		row->setSourceId(sourceId);
			
		row->setTransitionIndex(transitionIndex);
			
		row->setVelDef(velDef);
	
		return row;		
	}	

	DopplerRow* DopplerTable::newRowFull(int sourceId, int transitionIndex, Speed velDef)	{
		DopplerRow *row = new DopplerRow(*this);
			
		row->setSourceId(sourceId);
			
		row->setTransitionIndex(transitionIndex);
			
		row->setVelDef(velDef);
	
		return row;				
	}
	


DopplerRow* DopplerTable::newRow(DopplerRow* row) {
	return new DopplerRow(*this, *row);
}

DopplerRow* DopplerTable::newRowCopy(DopplerRow* row) {
	return new DopplerRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.dopplerId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a DopplerRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	DopplerRow* DopplerTable::add(DopplerRow* x) {
		DopplerRow* aRow = lookup(
				
			x->getSourceId()
				,
			x->getTransitionIndex()
				,
			x->getVelDef()
				
		);
		if (aRow) return aRow;

		// Autoincrementation algorithm. We use the hashtable.
		ostringstream noAutoIncIdsEntry;
		noAutoIncIdsEntry
			
				
		<< x->sourceId << "_"			
					
			
		;
		
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(noAutoIncIdsEntry.str())) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable

			
			// Initialize  dopplerId to 0.
			x->dopplerId = 0;		
			//x->setDopplerId(0);
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(noAutoIncIdsEntry.str(), 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  dopplerId to n.		
			//x->setDopplerId(n);
			x->dopplerId = n;
			
			// Record it in the map.		
			noAutoIncIds.erase(iter);
			noAutoIncIds.insert(make_pair(noAutoIncIdsEntry.str(), n));				
		}	
		
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
	DopplerRow*  DopplerTable::checkAndAdd(DopplerRow* x) throw (DuplicateKey, UniquenessViolationException) {
	 
		 
		if (lookup(
			
			x->getSourceId()
		,
			x->getTransitionIndex()
		,
			x->getVelDef()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table DopplerTable");
		
		
		
		if (getRowByKey(
	
			x->getDopplerId()
	,
			x->getSourceId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "DopplerTable");
		
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
	 * @return Alls rows as an array of DopplerRow
	 */
	vector<DopplerRow *> DopplerTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a DopplerRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	DopplerRow* DopplerTable::getRowByKey(int dopplerId, int sourceId)  {
	DopplerRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->dopplerId != dopplerId) continue;
			
		
			
				if (aRow->sourceId != sourceId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param sourceId.
 	 		
 * @param transitionIndex.
 	 		
 * @param velDef.
 	 		 
 */
DopplerRow* DopplerTable::lookup(int sourceId, int transitionIndex, Speed velDef) {
		DopplerRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(sourceId, transitionIndex, velDef)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	 	
/*
 * Returns a vector of pointers on rows whose key element dopplerId 
 * is equal to the parameter dopplerId.
 * @return a vector of vector <DopplerRow *>. A returned vector of size 0 means that no row has been found.
 * @param dopplerId int contains the value of
 * the autoincrementable attribute that is looked up in the table.
 */
 vector <DopplerRow *>  DopplerTable::getRowByDopplerId(int dopplerId) {
	vector<DopplerRow *> list;
	for (unsigned int i = 0; i < row.size(); ++i) {
		DopplerRow &x = *row[i];
					
		if (x.dopplerId == dopplerId)
			
		list.push_back(row[i]);
	}
	//if (list.size() == 0) throw new NoSuchRow("","Doppler");
	return list;	
 }
	





#ifndef WITHOUT_ACS
	// Conversion Methods

	DopplerTableIDL *DopplerTable::toIDL() {
		DopplerTableIDL *x = new DopplerTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<DopplerRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void DopplerTable::fromIDL(DopplerTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			DopplerRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *DopplerTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","Doppler");
	}

	void DopplerTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","Doppler");
	}

	string DopplerTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","Doppler");
	}

	void DopplerTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","Doppler");
	}

	string DopplerTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<DopplerTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/DopplerTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<DopplerTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<DopplerRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</DopplerTable> ");
		return buf;
	}
	
	void DopplerTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<DopplerTable")) 
			error();
		// cout << "Parsing a DopplerTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "DopplerTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		DopplerRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a DopplerRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"DopplerTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"DopplerTable");	
			}
			catch (...) {
				// cout << "Unexpected error in DopplerTable::checkAndAdd called from DopplerTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</DopplerTable>")) 
			error();
	}

	void DopplerTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","Doppler");
	}
	
	string DopplerTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void DopplerTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void DopplerTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Doppler.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Doppler");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Doppler");
		}
		else {
			// write the XML
			string fileName = directory + "/Doppler.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Doppler");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Doppler");
		}
	}
	
	void DopplerTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/Doppler.bin";
		else
			tablename = directory + "/Doppler.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "Doppler");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"Doppler");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"Doppler");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
	void DopplerTable::autoIncrement(string key, DopplerRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  dopplerId to 0.		
			x->setDopplerId(0);
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  dopplerId to n.		
			x->setDopplerId(n);
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
