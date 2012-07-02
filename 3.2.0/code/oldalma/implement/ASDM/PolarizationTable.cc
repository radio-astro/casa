
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
 * File PolarizationTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <PolarizationTable.h>
#include <PolarizationRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::PolarizationTable;
using asdm::PolarizationRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string PolarizationTable::tableName = "Polarization";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> PolarizationTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> PolarizationTable::getKeyName() {
		return key;
	}


	PolarizationTable::PolarizationTable(ASDM &c) : container(c) {

	
		key.push_back("polarizationId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("PolarizationTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for PolarizationTable.
 */
 
	PolarizationTable::~PolarizationTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &PolarizationTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int PolarizationTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string PolarizationTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity PolarizationTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void PolarizationTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	PolarizationRow *PolarizationTable::newRow() {
		return new PolarizationRow (*this);
	}
	
	PolarizationRow *PolarizationTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param numCorr. 
	
 	 * @param corrType. 
	
 	 * @param corrProduct. 
	
     */
	PolarizationRow* PolarizationTable::newRow(int numCorr, vector<StokesParameterMod::StokesParameter > corrType, vector<vector<PolarizationTypeMod::PolarizationType > > corrProduct){
		PolarizationRow *row = new PolarizationRow(*this);
			
		row->setNumCorr(numCorr);
			
		row->setCorrType(corrType);
			
		row->setCorrProduct(corrProduct);
	
		return row;		
	}	

	PolarizationRow* PolarizationTable::newRowFull(int numCorr, vector<StokesParameterMod::StokesParameter > corrType, vector<vector<PolarizationTypeMod::PolarizationType > > corrProduct)	{
		PolarizationRow *row = new PolarizationRow(*this);
			
		row->setNumCorr(numCorr);
			
		row->setCorrType(corrType);
			
		row->setCorrProduct(corrProduct);
	
		return row;				
	}
	


PolarizationRow* PolarizationTable::newRow(PolarizationRow* row) {
	return new PolarizationRow(*this, *row);
}

PolarizationRow* PolarizationTable::newRowCopy(PolarizationRow* row) {
	return new PolarizationRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.polarizationId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a PolarizationRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	PolarizationRow* PolarizationTable::add(PolarizationRow* x) {
			 
		PolarizationRow* aRow = lookup(
				
		x->getNumCorr()
				,
		x->getCorrType()
				,
		x->getCorrProduct()
				
		);
		if (aRow) return aRow;
			

			
		// Autoincrement polarizationId
		x->setPolarizationId(Tag(size(), TagType::Polarization));
						
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
	PolarizationRow*  PolarizationTable::checkAndAdd(PolarizationRow* x) throw (DuplicateKey, UniquenessViolationException) {
	 
		 
		if (lookup(
			
			x->getNumCorr()
		,
			x->getCorrType()
		,
			x->getCorrProduct()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table PolarizationTable");
		
		
		
		if (getRowByKey(
	
			x->getPolarizationId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "PolarizationTable");
		
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
	 * @return Alls rows as an array of PolarizationRow
	 */
	vector<PolarizationRow *> PolarizationTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a PolarizationRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	PolarizationRow* PolarizationTable::getRowByKey(Tag polarizationId)  {
	PolarizationRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->polarizationId != polarizationId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param numCorr.
 	 		
 * @param corrType.
 	 		
 * @param corrProduct.
 	 		 
 */
PolarizationRow* PolarizationTable::lookup(int numCorr, vector<StokesParameterMod::StokesParameter > corrType, vector<vector<PolarizationTypeMod::PolarizationType > > corrProduct) {
		PolarizationRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(numCorr, corrType, corrProduct)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	PolarizationTableIDL *PolarizationTable::toIDL() {
		PolarizationTableIDL *x = new PolarizationTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<PolarizationRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void PolarizationTable::fromIDL(PolarizationTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			PolarizationRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *PolarizationTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","Polarization");
	}

	void PolarizationTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","Polarization");
	}

	string PolarizationTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","Polarization");
	}

	void PolarizationTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","Polarization");
	}

	string PolarizationTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<PolarizationTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/PolarizationTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<PolarizationTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<PolarizationRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</PolarizationTable> ");
		return buf;
	}
	
	void PolarizationTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<PolarizationTable")) 
			error();
		// cout << "Parsing a PolarizationTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "PolarizationTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		PolarizationRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a PolarizationRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"PolarizationTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"PolarizationTable");	
			}
			catch (...) {
				// cout << "Unexpected error in PolarizationTable::checkAndAdd called from PolarizationTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</PolarizationTable>")) 
			error();
	}

	void PolarizationTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","Polarization");
	}
	
	string PolarizationTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void PolarizationTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void PolarizationTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/Polarization.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Polarization");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Polarization");
		}
		else {
			// write the XML
			string fileName = directory + "/Polarization.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "Polarization");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "Polarization");
		}
	}
	
	void PolarizationTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/Polarization.bin";
		else
			tablename = directory + "/Polarization.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "Polarization");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"Polarization");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"Polarization");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			
			
	
	

	
	void PolarizationTable::autoIncrement(string key, PolarizationRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  polarizationId to Tag(0).
			x->setPolarizationId(Tag(0,  TagType::Polarization));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  polarizationId to Tag(n).
			x->setPolarizationId(Tag(n, TagType::Polarization));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
